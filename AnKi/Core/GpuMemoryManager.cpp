// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemoryManager.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

VertexGpuMemoryManager::~VertexGpuMemoryManager()
{
	// Do nothing
}

Error VertexGpuMemoryManager::init(GenericMemoryPoolAllocator<U8> alloc, GrManager* gr, const ConfigSet& cfg)
{
	m_gr = gr;

	// Create the GPU buffer.
	BufferInitInfo bufferInit("Global vertex & index");
	bufferInit.m_size = cfg.getNumberU64("core_globalVertexMemorySize");
	if(!isPowerOfTwo(bufferInit.m_size))
	{
		ANKI_CORE_LOGE("core_globalVertexMemorySize should be a power of two (because of the buddy allocator");
		return Error::USER_DATA;
	}
	bufferInit.m_usage = BufferUsageBit::VERTEX | BufferUsageBit::INDEX;
	m_vertBuffer = gr->newBuffer(bufferInit);

	// Init the rest
	m_buddyAllocator.init(alloc, __builtin_ctzll(bufferInit.m_size));

	return Error::NONE;
}

ANKI_USE_RESULT Error VertexGpuMemoryManager::allocate(PtrSize size, PtrSize& offset)
{
	U32 offset32;
	ANKI_CHECK(m_buddyAllocator.allocate(size, offset32));

	offset = offset32;

	return Error::NONE;
}

void VertexGpuMemoryManager::free(PtrSize size, PtrSize offset)
{
	m_buddyAllocator.free(U32(offset), size);
}

StagingGpuMemoryManager::~StagingGpuMemoryManager()
{
	m_gr->finish();

	for(auto& it : m_perFrameBuffers)
	{
		it.m_buff->unmap();
		it.m_buff.reset(nullptr);
	}
}

Error StagingGpuMemoryManager::init(GrManager* gr, const ConfigSet& cfg)
{
	m_gr = gr;

	m_perFrameBuffers[StagingGpuMemoryType::UNIFORM].m_size = cfg.getNumberU32("core_uniformPerFrameMemorySize");
	m_perFrameBuffers[StagingGpuMemoryType::STORAGE].m_size = cfg.getNumberU32("core_storagePerFrameMemorySize");
	m_perFrameBuffers[StagingGpuMemoryType::VERTEX].m_size = cfg.getNumberU32("core_vertexPerFrameMemorySize");
	m_perFrameBuffers[StagingGpuMemoryType::TEXTURE].m_size = cfg.getNumberU32("core_textureBufferPerFrameMemorySize");

	initBuffer(StagingGpuMemoryType::UNIFORM, gr->getDeviceCapabilities().m_uniformBufferBindOffsetAlignment,
			   gr->getDeviceCapabilities().m_uniformBufferMaxRange, BufferUsageBit::ALL_UNIFORM, *gr);

	initBuffer(StagingGpuMemoryType::STORAGE,
			   max(gr->getDeviceCapabilities().m_storageBufferBindOffsetAlignment,
				   gr->getDeviceCapabilities().m_sbtRecordAlignment),
			   gr->getDeviceCapabilities().m_storageBufferMaxRange, BufferUsageBit::ALL_STORAGE | BufferUsageBit::SBT,
			   *gr);

	initBuffer(StagingGpuMemoryType::VERTEX, 16, MAX_U32, BufferUsageBit::VERTEX | BufferUsageBit::INDEX, *gr);

	initBuffer(StagingGpuMemoryType::TEXTURE, gr->getDeviceCapabilities().m_textureBufferBindOffsetAlignment,
			   gr->getDeviceCapabilities().m_textureBufferMaxRange, BufferUsageBit::ALL_TEXTURE, *gr);

	return Error::NONE;
}

void StagingGpuMemoryManager::initBuffer(StagingGpuMemoryType type, U32 alignment, PtrSize maxAllocSize,
										 BufferUsageBit usage, GrManager& gr)
{
	PerFrameBuffer& perframe = m_perFrameBuffers[type];

	perframe.m_buff = gr.newBuffer(BufferInitInfo(perframe.m_size, usage, BufferMapAccessBit::WRITE, "Staging"));
	perframe.m_alloc.init(perframe.m_size, alignment, maxAllocSize);
	perframe.m_mappedMem = static_cast<U8*>(perframe.m_buff->map(0, perframe.m_size, BufferMapAccessBit::WRITE));
}

void* StagingGpuMemoryManager::allocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
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

void* StagingGpuMemoryManager::tryAllocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token)
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

void StagingGpuMemoryManager::endFrame()
{
	for(StagingGpuMemoryType usage = StagingGpuMemoryType::UNIFORM; usage < StagingGpuMemoryType::COUNT; ++usage)
	{
		PerFrameBuffer& buff = m_perFrameBuffers[usage];

		if(buff.m_mappedMem)
		{
			// Increase the counters
			switch(usage)
			{
			case StagingGpuMemoryType::UNIFORM:
				ANKI_TRACE_INC_COUNTER(STAGING_UNIFORMS_SIZE, buff.m_alloc.getUnallocatedMemorySize());
				break;
			case StagingGpuMemoryType::STORAGE:
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
