// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

UnifiedGeometryMemoryPool::~UnifiedGeometryMemoryPool()
{
	// Do nothing
}

Error UnifiedGeometryMemoryPool::init(HeapMemoryPool* pool, GrManager* gr, const ConfigSet& cfg)
{
	ANKI_ASSERT(pool && gr);
	m_gr = gr;

	// Create the GPU buffer.
	BufferInitInfo bufferInit("Global vertex & index");
	bufferInit.m_size = cfg.getCoreGlobalVertexMemorySize();
	if(!isPowerOfTwo(bufferInit.m_size))
	{
		ANKI_CORE_LOGE("core_globalVertexMemorySize should be a power of two (because of the buddy allocator");
		return Error::kUserData;
	}

	bufferInit.m_usage = BufferUsageBit::kVertex | BufferUsageBit::kIndex | BufferUsageBit::kTransferDestination;
	if(gr->getDeviceCapabilities().m_rayTracingEnabled)
	{
		bufferInit.m_usage |= BufferUsageBit::kAccelerationStructureBuild;
	}

	m_vertBuffer = gr->newBuffer(bufferInit);

	// Init the rest
	m_buddyAllocator.init(pool, __builtin_ctzll(bufferInit.m_size));

	return Error::kNone;
}

Error UnifiedGeometryMemoryPool::allocate(PtrSize size, U32 alignment, PtrSize& offset)
{
	U32 offset32;
	const Bool success = m_buddyAllocator.allocate(size, alignment, offset32);
	if(ANKI_UNLIKELY(!success))
	{
		BuddyAllocatorBuilderStats stats;
		m_buddyAllocator.getStats(stats);
		ANKI_CORE_LOGE("Failed to allocate vertex memory of size %zu. The allocator has %zu (user requested %zu) out "
					   "%zu allocated",
					   size, stats.m_realAllocatedSize, stats.m_userAllocatedSize, m_vertBuffer->getSize());
		return Error::kOutOfMemory;
	}

	offset = offset32;

	return Error::kNone;
}

void UnifiedGeometryMemoryPool::free(PtrSize size, U32 alignment, PtrSize offset)
{
	m_buddyAllocator.free(U32(offset), size, alignment);
}

StagingGpuMemoryPool::~StagingGpuMemoryPool()
{
	m_gr->finish();

	for(auto& it : m_perFrameBuffers)
	{
		it.m_buff->unmap();
		it.m_buff.reset(nullptr);
	}
}

Error StagingGpuMemoryPool::init(GrManager* gr, const ConfigSet& cfg)
{
	m_gr = gr;

	m_perFrameBuffers[StagingGpuMemoryType::kUniform].m_size = cfg.getCoreUniformPerFrameMemorySize();
	m_perFrameBuffers[StagingGpuMemoryType::kStorage].m_size = cfg.getCoreStoragePerFrameMemorySize();
	m_perFrameBuffers[StagingGpuMemoryType::kVertex].m_size = cfg.getCoreVertexPerFrameMemorySize();
	m_perFrameBuffers[StagingGpuMemoryType::kTexture].m_size = cfg.getCoreTextureBufferPerFrameMemorySize();

	initBuffer(StagingGpuMemoryType::kUniform, gr->getDeviceCapabilities().m_uniformBufferBindOffsetAlignment,
			   gr->getDeviceCapabilities().m_uniformBufferMaxRange, BufferUsageBit::kAllUniform, *gr);

	initBuffer(StagingGpuMemoryType::kStorage,
			   max(gr->getDeviceCapabilities().m_storageBufferBindOffsetAlignment,
				   gr->getDeviceCapabilities().m_sbtRecordAlignment),
			   gr->getDeviceCapabilities().m_storageBufferMaxRange, BufferUsageBit::kAllStorage | BufferUsageBit::kSBT,
			   *gr);

	initBuffer(StagingGpuMemoryType::kVertex, 16, kMaxU32, BufferUsageBit::kVertex | BufferUsageBit::kIndex, *gr);

	initBuffer(StagingGpuMemoryType::kTexture, gr->getDeviceCapabilities().m_textureBufferBindOffsetAlignment,
			   gr->getDeviceCapabilities().m_textureBufferMaxRange, BufferUsageBit::kAllTexture, *gr);

	return Error::kNone;
}

void StagingGpuMemoryPool::initBuffer(StagingGpuMemoryType type, U32 alignment, PtrSize maxAllocSize,
									  BufferUsageBit usage, GrManager& gr)
{
	PerFrameBuffer& perframe = m_perFrameBuffers[type];

	perframe.m_buff = gr.newBuffer(BufferInitInfo(perframe.m_size, usage, BufferMapAccessBit::kWrite, "Staging"));
	perframe.m_alloc.init(perframe.m_size, alignment, maxAllocSize);
	perframe.m_mappedMem = static_cast<U8*>(perframe.m_buff->map(0, perframe.m_size, BufferMapAccessBit::kWrite));
}

void* StagingGpuMemoryPool::allocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
{
	PerFrameBuffer& buff = m_perFrameBuffers[usage];
	const Error err = buff.m_alloc.allocate(size, token.m_offset);
	if(err)
	{
		ANKI_CORE_LOGF("Out of staging GPU memory. Usage: %u", U32(usage));
	}

	token.m_buffer = buff.m_buff;
	token.m_range = size;
	token.m_type = usage;

	return buff.m_mappedMem + token.m_offset;
}

void* StagingGpuMemoryPool::tryAllocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
{
	PerFrameBuffer& buff = m_perFrameBuffers[usage];
	const Error err = buff.m_alloc.allocate(size, token.m_offset);
	if(!err)
	{
		token.m_buffer = buff.m_buff;
		token.m_range = size;
		token.m_type = usage;
		return buff.m_mappedMem + token.m_offset;
	}
	else
	{
		token = {};
		return nullptr;
	}
}

void StagingGpuMemoryPool::endFrame()
{
	for(StagingGpuMemoryType usage : EnumIterable<StagingGpuMemoryType>())
	{
		PerFrameBuffer& buff = m_perFrameBuffers[usage];

		if(buff.m_mappedMem)
		{
			// Increase the counters
			switch(usage)
			{
			case StagingGpuMemoryType::kUniform:
				ANKI_TRACE_INC_COUNTER(STAGING_UNIFORMS_SIZE, buff.m_alloc.getUnallocatedMemorySize());
				break;
			case StagingGpuMemoryType::kStorage:
				ANKI_TRACE_INC_COUNTER(STAGING_STORAGE_SIZE, buff.m_alloc.getUnallocatedMemorySize());
				break;
			default:
				break;
			}

			buff.m_alloc.endFrame();
		}
	}
}

} // end namespace anki
