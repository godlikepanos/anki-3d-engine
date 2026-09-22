// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/Common.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/Function.h>

namespace anki {

ANKI_CVAR2(NumericCVar<U32>, GpuMem, CopyEngine, BufferSize, U32(64_MB), U32(16_MB), U32(2_GB), "Memory size for the copy engine")
ANKI_CVAR2(NumericCVar<U32>, GpuMem, CopyEngine, AccelerationStructureScratchBufferSize, U32(64_MB), U32(16_MB), U32(2_GB),
		   "Memory size for the ring buffer used for BLAS builds")

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

	// It's a copy command. Fills the buffer with zeros without consuming any staging memory.
	// It's thread-safe
	void zeroBuffer(const BufferView& dst);

	// It's a barrier command
	// It's thread-safe
	void setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
							ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures);

	// A general compute command.
	// It's thread-safe
	void buildAccelerationStructure(AccelerationStructure* as);

	// Append a callback to a list of callbacks. Those callbacks will be called when the previously set commands get flushed. If a flush happened
	// right before, the callback will be triggered immediately
	// WARNING: Don't access the CopyEngine in the callback. It will deadlock.
	// WARNING: The callback may trigger right away or in some other random thread.
	// It's thread-safe
	void addPostFlushCallback(const Function<void(Fence* fence)>& callback);

	// End commands //

	// Flush the pending commands and get a fence back. It always returns the last known fence except if there were no commands ever.
	// the same as the last one.
	// It's thread-safe
	void flush(FencePtr& fence);

	// Wait for all work to complete. It will flush pending work
	void flushAndWaitForAllWork()
	{
		FencePtr fence;
		flush(fence);
		if(fence)
		{
			fence->clientWaitForever();
		}
	}

private:
	static constexpr U32 kSplitBatchPercentage = 50; // If a batch grows bigger than this, flush it

	// Choose an alignment that satisfies 16 bytes and 3 bytes. RGB8 formats require 3 bytes alignment for the source of the buffer to image copies.
	static constexpr U32 kGpuBufferAlignment = 16 * 3;

	class Command;
	class Batch;

	Mutex m_mtx;

	BufferPtr m_ringBuffer;
	U8* m_ringBufferMappedMem = nullptr;

	BufferPtr m_asScratchBuffer;

	FencePtr m_lastFence;

	DynamicArray<Batch> m_batches;

	DynamicArray<Function<void(Fence*)>> m_postFlushCallbacks;

	U32 m_asScratchBufferOffset = 0;

	U32 m_ringBufferSize = kMaxU32; // Cache it
	U32 m_asScratchBufferSize = kMaxU32;

	void flushInternal();

	Command& newCommand(U32 ringBufferAllocSize, WeakArray<U8>& ringBufferMappedMem, U32& ringBufferOffset);

	U32 allocate(U32 size);

	void cleanupCompletedBatches();

	void validate() const;
};

} // end namespace anki
