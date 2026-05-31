// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/CopyEngine.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

// This is a union of commands the CopyEngine can accept.
class CopyEngine::Command
{
public:
	ANKI_NON_COPYABLE(Command)

	enum class CommandType : U8
	{
		kCopyBufferToTexture,
		kCopyBufferToBuffer,
		kSetPipelineBarrier,
		kBuildAs,

		kCount
	};

	class CopyBufferToTextureCommand
	{
	public:
		U32 m_ringBufferOffset = kMaxU32;
		U32 m_ringBufferRange = 0;

		TexturePtr m_tex;
		TextureSubresourceDesc m_texSubresource = TextureSubresourceDesc::all();
	};

	class CopyBufferToBufferCommand
	{
	public:
		U32 m_ringBufferOffset = kMaxU32;
		U32 m_ringBufferRange = 0;

		BufferPtr m_dst;
		PtrSize m_dstOffset = kMaxPtrSize;
		PtrSize m_dstRange = kMaxPtrSize;
	};

	class SetPipelineBarrierCommand
	{
	public:
		DynamicArray<TextureBarrierInfo> m_textures;
		DynamicArray<BufferBarrierInfo> m_buffers;
		DynamicArray<AccelerationStructureBarrierInfo> m_accelerationStructures;
	};

	class BuildAsCommand
	{
	public:
		AccelerationStructurePtr m_as;

		BufferPtr m_buff;
		PtrSize m_buffOffset = kMaxPtrSize;
		PtrSize m_buffRange = kMaxPtrSize;
	};

	CommandType m_type = CommandType::kCount;

	union
	{
		U32 m_dummy = 0;
		CopyBufferToTextureCommand m_copyBufferToTexture;
		CopyBufferToBufferCommand m_copyBufferToBuffer;
		SetPipelineBarrierCommand m_setPipelineBarrier;
		BuildAsCommand m_buildAs;
	};

	Command()
	{
		construct();
	}

	Command(Command&& b)
		: Command()
	{
		*this = std::move(b);
	}

	~Command()
	{
		destroy();
	}

	Command& operator=(Command&& b)
	{
		destroy();

		m_type = b.m_type;
		switch(b.m_type)
		{
		case CommandType::kCopyBufferToTexture:
			m_copyBufferToTexture = std::move(b.m_copyBufferToTexture);
			break;
		case CommandType::kCopyBufferToBuffer:
			m_copyBufferToBuffer = std::move(b.m_copyBufferToBuffer);
			break;
		case CommandType::kSetPipelineBarrier:
			m_setPipelineBarrier = std::move(b.m_setPipelineBarrier);
			break;
		case CommandType::kBuildAs:
			m_buildAs = std::move(b.m_buildAs);
			break;
		case CommandType::kCount:
			break;
		default:
			ANKI_ASSERT(0);
		}

		b.destroy();

		return *this;
	}

	void construct()
	{
		zeroMemory(*this); // It's fine to zero everything, we will construct it in command methods
		m_type = CommandType::kCount;
	}

	void destroy()
	{
		switch(m_type)
		{
		case CommandType::kCopyBufferToTexture:
			callDestructor(m_copyBufferToTexture);
			break;
		case CommandType::kCopyBufferToBuffer:
			callDestructor(m_copyBufferToBuffer);
			break;
		case CommandType::kSetPipelineBarrier:
			callDestructor(m_setPipelineBarrier);
			break;
		case CommandType::kBuildAs:
			callDestructor(m_buildAs);
			break;
		case CommandType::kCount:
			break;
		default:
			ANKI_ASSERT(0);
		}

		construct();
	}
};

class CopyEngine::Batch
{
public:
	ANKI_NON_COPYABLE(Batch)

	DynamicArray<Command> m_commands;
	U32 m_startOffset = kMaxU32; // This is always less than CopyEngine's ring buffer size
	U32 m_allocatedSize = kMaxU32;
	FencePtr m_fence;

	Batch() = default;

	Batch(Batch&& b)
	{
		*this = std::move(b);
	}

	Batch& operator=(Batch&& b)
	{
		m_commands = std::move(b.m_commands);
		m_startOffset = b.m_startOffset;
		b.m_startOffset = kMaxU32;
		m_allocatedSize = b.m_allocatedSize;
		b.m_allocatedSize = kMaxU32;
		m_fence = std::move(b.m_fence);
		return *this;
	}

	Bool isClosed() const
	{
		return !!m_fence;
	}
};

CopyEngine::CopyEngine()
{
}

CopyEngine::~CopyEngine()
{
	if(m_ringBufferMappedMem)
	{
		m_ringBuffer->unmap();
	}
}

void CopyEngine::init()
{
	BufferInitInfo info("CopyEngine");
	info.m_mapAccess = BufferMapAccessBit::kWrite;
	info.m_size = g_cvarGpuMemCopyEngineBuffersize;
	info.m_usage = BufferUsageBit::kCopySource;
	m_ringBuffer = GrManager::getSingleton().newBuffer(info);

	m_ringBufferMappedMem = static_cast<U8*>(m_ringBuffer->map(0, kMaxPtrSize));

	m_ringBufferSize = g_cvarGpuMemCopyEngineBuffersize;
}

void CopyEngine::flushInternal(FencePtr& fence)
{
	fence.reset(nullptr);

	if(m_batches.isEmpty()) [[unlikely]]
	{
		return;
	}

	Batch& crntBatch = m_batches.getBack();
	ANKI_ASSERT(!crntBatch.m_fence);

	if(crntBatch.m_allocatedSize == 0 || crntBatch.m_commands.getSize() == 0)
	{
		// Empty batch, don't bother
		return;
	}

	// Populate and submit the command buffer
	CommandBufferInitInfo init("CopyEngine commands");
	init.m_flags = CommandBufferFlag::kComputeWork;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(init);

	cmdb->pushDebugMarker("CopyEngine", Vec3(1.0f, 1.0f, 0.0f));

	for(const Command& cmd : crntBatch.m_commands)
	{
		switch(cmd.m_type)
		{
		case Command::CommandType::kCopyBufferToTexture:
			cmdb->copyBufferToTexture(
				BufferView(m_ringBuffer.get(), cmd.m_copyBufferToTexture.m_ringBufferOffset, cmd.m_copyBufferToTexture.m_ringBufferRange),
				TextureView(cmd.m_copyBufferToTexture.m_tex.get(), cmd.m_copyBufferToTexture.m_texSubresource));
			break;
		case Command::CommandType::kCopyBufferToBuffer:
			cmdb->copyBufferToBuffer(
				BufferView(m_ringBuffer.get(), cmd.m_copyBufferToBuffer.m_ringBufferOffset, cmd.m_copyBufferToBuffer.m_ringBufferRange),
				BufferView(cmd.m_copyBufferToBuffer.m_dst.get(), cmd.m_copyBufferToBuffer.m_dstOffset, cmd.m_copyBufferToBuffer.m_dstRange));
			break;
		case Command::CommandType::kSetPipelineBarrier:
			cmdb->setPipelineBarrier(cmd.m_setPipelineBarrier.m_textures, cmd.m_setPipelineBarrier.m_buffers,
									 cmd.m_setPipelineBarrier.m_accelerationStructures);
			break;
		case Command::CommandType::kBuildAs:
			cmdb->buildAccelerationStructure(cmd.m_buildAs.m_as.get(),
											 BufferView(cmd.m_buildAs.m_buff.get(), cmd.m_buildAs.m_buffOffset, cmd.m_buildAs.m_buffRange));
			break;
		default:
			ANKI_ASSERT(0);
		}
	}

	cmdb->popDebugMarker();
	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);

	// Update the current batch
	crntBatch.m_fence = fence;
	crntBatch.m_commands.resize(0); // Free memory
}

void CopyEngine::cleanupCompletedBatches()
{
	while(m_batches.getSize())
	{
		auto it = m_batches.getBegin();
		if(it->m_fence && it->m_fence->signaled())
		{
			m_batches.erase(it);
		}
		else
		{
			break;
		}
	}
}

U32 CopyEngine::allocate(U32 origSize)
{
	ANKI_TRACE_SCOPED_EVENT(CopyEngineAllocate);

	ANKI_ASSERT(origSize > 0);
	const U32 size = origSize + kGpuBufferAlignment;
	ANKI_ASSERT(size < m_ringBufferSize);
	ANKI_ASSERT(size < m_ringBufferSize * kSplitBatchPercentage / 100 && "Can't have that huge allocations");

	auto createBatch = [this]() -> Batch* {
		U32 startOffset;
		if(m_batches.getSize())
		{
			startOffset = m_batches.getBack().m_startOffset + m_batches.getBack().m_allocatedSize;
			startOffset %= m_ringBufferSize;
		}
		else
		{
			startOffset = 0;
		}

		Batch& b = *m_batches.emplaceBack();
		b.m_allocatedSize = 0;
		b.m_startOffset = startOffset;

		return &b;
	};

	Batch* batch = nullptr;
	if(m_batches.getSize() == 0 || m_batches.getBack().isClosed())
	{
		batch = createBatch();
	}
	else
	{
		batch = &m_batches.getBack();
	}

	if(batch->m_allocatedSize + size > m_ringBufferSize * kSplitBatchPercentage / 100)
	{
		// Batch has grown too big, flush and create new

		ANKI_ASSERT(batch->m_commands.getSize() > 0 && "Oversized batch without commands shouldn't happen");

		FencePtr fence;
		flushInternal(fence);

		batch = createBatch();
	}

	// Now allocate
	U32 newOffset = kMaxU32;
	U32 newEnd = kMaxU32;

	U32 iterationCount = 100;
	while(--iterationCount)
	{
		newOffset = batch->m_startOffset + batch->m_allocatedSize;

		newEnd = newOffset + size;
		U32 newAllocatedSize = 0;
		if(newOffset < m_ringBufferSize && newEnd > m_ringBufferSize)
		{
			// Wraps around, start at the beginning of the ring buffer

			newOffset = 0;
			newEnd = size;

			newAllocatedSize = m_ringBufferSize - batch->m_startOffset + size;
		}
		else
		{
			newAllocatedSize = newEnd - batch->m_startOffset;

			newOffset %= m_ringBufferSize;
			newEnd = newOffset + size;
		}

		ANKI_ASSERT(newOffset < newEnd);
		ANKI_ASSERT(newEnd <= m_ringBufferSize);

		const Bool batchCanGrow = (m_batches.getSize() == 1) || newEnd <= m_batches.getFront().m_startOffset;
		if(!batchCanGrow)
		{
			const Bool signaled = m_batches.getFront().m_fence->clientWait(kMaxSecond);
			if(!signaled)
			{
				ANKI_GPUMEM_LOGF("GPU timeout detected");
			}

			cleanupCompletedBatches();
			batch = &m_batches.getBack();
		}
		else
		{
			batch->m_allocatedSize = newAllocatedSize;
			break;
		}
	}

	ANKI_ASSERT(iterationCount != 0);

	const U32 outOffset = getAlignedRoundUp(kGpuBufferAlignment, newOffset);

	ANKI_ASSERT(outOffset >= newOffset);
	ANKI_ASSERT(outOffset < m_ringBufferSize);
	ANKI_ASSERT(outOffset + origSize <= newEnd);
	ANKI_ASSERT(outOffset + origSize <= m_ringBufferSize);

	validate();

	return outOffset;
}

CopyEngine::Command& CopyEngine::newCommand(U32 ringBufferAllocSize, WeakArray<U8>& ringBufferMappedMem, U32& ringBufferOffset)
{
	if(ringBufferAllocSize > 0)
	{
		ringBufferOffset = allocate(ringBufferAllocSize);
		ringBufferMappedMem = WeakArray<U8>(m_ringBufferMappedMem + ringBufferOffset, ringBufferAllocSize);
	}
	else
	{
		// Allocate dummy memory to force creation of a batch
		allocate(1);
	}

	ANKI_ASSERT(m_batches.getSize() && !m_batches.getBack().isClosed());
	Batch& batch = m_batches.getBack();
	Command& newCmd = *batch.m_commands.emplaceBack();

	return newCmd;
}

CopyEngineLockGuard CopyEngine::copyBufferToTexture(U32 srcBufferSize, WeakArray<U8>& srcBufferMappedMem, const TextureView& dst)
{
	CopyEngineLockGuard guard(&m_mtx);

	U32 ringBufferOffset = kMaxU32;
	Command& cmd = newCommand(srcBufferSize, srcBufferMappedMem, ringBufferOffset);

	cmd.m_type = Command::CommandType::kCopyBufferToTexture;
	cmd.m_copyBufferToTexture.m_ringBufferOffset = ringBufferOffset;
	cmd.m_copyBufferToTexture.m_ringBufferRange = srcBufferSize;
	cmd.m_copyBufferToTexture.m_tex.reset(&dst.getTexture());
	cmd.m_copyBufferToTexture.m_texSubresource = dst.getSubresource();

	return guard;
}

CopyEngineLockGuard CopyEngine::copyBufferToBuffer(U32 srcBufferSize, WeakArray<U8>& srcBufferMappedMem, const BufferView& dst)
{
	CopyEngineLockGuard guard(&m_mtx);

	U32 ringBufferOffset = kMaxU32;
	Command& cmd = newCommand(srcBufferSize, srcBufferMappedMem, ringBufferOffset);

	cmd.m_type = Command::CommandType::kCopyBufferToBuffer;
	cmd.m_copyBufferToBuffer.m_ringBufferOffset = ringBufferOffset;
	cmd.m_copyBufferToBuffer.m_ringBufferRange = srcBufferSize;
	cmd.m_copyBufferToBuffer.m_dst.reset(&dst.getBuffer());
	cmd.m_copyBufferToBuffer.m_dstOffset = dst.getOffset();
	cmd.m_copyBufferToBuffer.m_dstRange = dst.getRange();

	return guard;
}

void CopyEngine::setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
									ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures)
{
	LockGuard lock(m_mtx);

	WeakArray<U8> unused1;
	U32 unused2 = kMaxU32;
	Command& cmd = newCommand(0, unused1, unused2);

	cmd.m_type = Command::CommandType::kSetPipelineBarrier;
	DynamicArray<TextureBarrierInfo> arr(textures);
	cmd.m_setPipelineBarrier.m_textures = std::move(arr);
	DynamicArray<BufferBarrierInfo> arr2(buffers);
	cmd.m_setPipelineBarrier.m_buffers = std::move(arr2);
	DynamicArray<AccelerationStructureBarrierInfo> arr3(accelerationStructures);
	cmd.m_setPipelineBarrier.m_accelerationStructures = std::move(arr3);
}

void CopyEngine::buildAccelerationStructure(AccelerationStructure* as, const BufferView& scratchBuffer)
{
	LockGuard lock(m_mtx);

	WeakArray<U8> unused1;
	U32 unused2 = kMaxU32;
	Command& cmd = newCommand(0, unused1, unused2);

	cmd.m_type = Command::CommandType::kBuildAs;
	cmd.m_buildAs.m_as.reset(as);
	cmd.m_buildAs.m_buff.reset(&scratchBuffer.getBuffer());
	cmd.m_buildAs.m_buffOffset = scratchBuffer.getOffset();
	cmd.m_buildAs.m_buffRange = scratchBuffer.getRange();
}

void CopyEngine::flush(FencePtr& fence)
{
	ANKI_TRACE_SCOPED_EVENT(CopyEngineFlush);

	LockGuard lock(m_mtx);

	cleanupCompletedBatches();
	flushInternal(fence);
	validate();
}

void CopyEngine::validate() const
{
#if ANKI_ASSERTIONS_ENABLED
	for(U32 i = 0; i < m_batches.getSize(); ++i)
	{
		const Batch& batch = m_batches[i];
		ANKI_ASSERT(batch.m_startOffset < m_ringBufferSize);
		ANKI_ASSERT(batch.m_allocatedSize <= m_ringBufferSize);
		const Bool last = (i == m_batches.getSize() - 1);
		if(!last)
		{
			ANKI_ASSERT(batch.m_allocatedSize > 0);
			ANKI_ASSERT(!!batch.m_fence);
		}

		if(i > 0)
		{
			const Batch& prevBatch = m_batches[i - 1];
			ANKI_ASSERT((prevBatch.m_startOffset + prevBatch.m_allocatedSize) % m_ringBufferSize == batch.m_startOffset);
		}

		// Check no overlap between batches
		for(U32 j = 0; j < m_batches.getSize(); ++j)
		{
			if(i == j)
			{
				continue;
			}

			const Batch& otherBatch = m_batches[j];

			const Batch& leftBatch = (batch.m_startOffset < otherBatch.m_startOffset) ? batch : otherBatch;
			const Batch& rightBatch = (batch.m_startOffset > otherBatch.m_startOffset) ? batch : otherBatch;
			ANKI_ASSERT(leftBatch.m_startOffset + leftBatch.m_allocatedSize <= rightBatch.m_startOffset);

			if(rightBatch.m_startOffset + rightBatch.m_allocatedSize > m_ringBufferSize)
			{
				// Right batch wraps around
				const U32 remainder = (rightBatch.m_startOffset + rightBatch.m_allocatedSize) % m_ringBufferSize;
				ANKI_ASSERT(remainder <= leftBatch.m_startOffset);
			}
		}
	}
#endif
}

} // end namespace anki
