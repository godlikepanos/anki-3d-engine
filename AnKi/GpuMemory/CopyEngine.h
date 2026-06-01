// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/Common.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

ANKI_CVAR(NumericCVar<U32>, GpuMem, CopyEngineBuffersize, 64_MB, 16_MB, 2_GB, "Memory size for the copy engine")

// A mutex lock guard for some CopyEngine operations
class CopyEngineLockGuard
{
	friend class CopyEngine;

public:
	ANKI_NON_COPYABLE(CopyEngineLockGuard)

	CopyEngineLockGuard() = delete;

	CopyEngineLockGuard(CopyEngineLockGuard&& b)
	{
		m_mtx = b.m_mtx;
		b.m_mtx = nullptr;
	}

	~CopyEngineLockGuard()
	{
		unlock();
	}

	CopyEngineLockGuard& operator=(CopyEngineLockGuard&& b)
	{
		m_mtx = b.m_mtx;
		b.m_mtx = nullptr;
		return *this;
	}

	void unlock()
	{
		if(m_mtx)
		{
			m_mtx->unlock();
			m_mtx = nullptr;
#if ANKI_TRACING_ENABLED
			Tracer::getSingleton().endEvent("CopyEngineLock", m_traceHandle);
#endif
		}
	}

private:
	Mutex* m_mtx = nullptr;
#if ANKI_TRACING_ENABLED
	TracerEventHandle m_traceHandle;
#endif

	CopyEngineLockGuard(Mutex* mtx)
		: m_mtx(mtx)
	{
		if(m_mtx)
		{
			m_mtx->lock();
#if ANKI_TRACING_ENABLED
			m_traceHandle = Tracer::getSingleton().beginEvent("CopyEngineLock");
#endif
		}
	}
};

// This allocates and also executes commands used for streaming and initializing resources.
class CopyEngine : public MakeSingleton<CopyEngine>
{
public:
	ANKI_NON_COPYABLE(CopyEngine)

	CopyEngine();

	~CopyEngine();

	// Begin commands //

	// It's a copy command. It allocates srcBufferSize bytes of staging buffer and stores the mapped memory in srcBufferMappedMem. The
	// srcBufferMappedMem is valid until the CopyEngineLockGuard goes out of scope or until CopyEngineLockGuard::unlock
	// It's thread-safe
	CopyEngineLockGuard copyBufferToTexture(U32 srcBufferSize, WeakArray<U8>& srcBufferMappedMem, const TextureView& dst);

	// It's a copy command. It allocates srcBufferSize bytes of staging buffer and stores the mapped memory in srcBufferMappedMem. The
	// srcBufferMappedMem is valid until the CopyEngineLockGuard goes out of scope or until CopyEngineLockGuard::unlock
	// It's thread-safe
	CopyEngineLockGuard copyBufferToBuffer(U32 srcBufferSize, WeakArray<U8>& srcBufferMappedMem, const BufferView& dst);

	// It's a barrier command
	// It's thread-safe
	void setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
							ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures);

	// A general compute command.
	// It's thread-safe
	void buildAccelerationStructure(AccelerationStructure* as, const BufferView& scratchBuffer);

	// End commands //

	// Flush the pending commands and get a fence back. If nothing happened the fence will be empty.
	// It's thread-safe
	void flush(FencePtr& fence);

private:
	static constexpr U32 kSplitBatchPercentage = 50; // If a batch grows bigger than this, flush it

	// Choose an alignment that satisfies 16 bytes and 3 bytes. RGB8 formats require 3 bytes alignment for the source of the buffer to image copies.
	static constexpr U32 kGpuBufferAlignment = 16 * 3;

	class Command;
	class Batch;

	Mutex m_mtx;

	BufferPtr m_ringBuffer;
	U8* m_ringBufferMappedMem = nullptr;

	DynamicArray<Batch> m_batches;

	U32 m_ringBufferSize = kMaxU32; // Cache it

	void flushInternal(FencePtr& fence);

	Command& newCommand(U32 ringBufferAllocSize, WeakArray<U8>& ringBufferMappedMem, U32& ringBufferOffset);

	U32 allocate(U32 size);

	void cleanupCompletedBatches();

	void validate() const;
};

} // end namespace anki
