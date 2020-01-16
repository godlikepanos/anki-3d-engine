// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/gr/utils/StackGpuAllocator.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Memory handle.
class TransferGpuAllocatorHandle
{
	friend class TransferGpuAllocator;

public:
	TransferGpuAllocatorHandle()
	{
	}

	TransferGpuAllocatorHandle(const TransferGpuAllocatorHandle&) = delete;

	TransferGpuAllocatorHandle(TransferGpuAllocatorHandle&& b)
	{
		*this = std::move(b);
	}

	~TransferGpuAllocatorHandle()
	{
		ANKI_ASSERT(!valid() && "Forgot to release");
	}

	TransferGpuAllocatorHandle& operator=(TransferGpuAllocatorHandle&& b)
	{
		m_handle = b.m_handle;
		m_range = b.m_range;
		m_frame = b.m_frame;
		b.invalidate();
		return *this;
	}

	BufferPtr getBuffer() const;

	void* getMappedMemory() const;

	PtrSize getOffset() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle.m_offset;
	}

	PtrSize getRange() const
	{
		ANKI_ASSERT(m_handle);
		ANKI_ASSERT(m_range != 0);
		return m_range;
	}

private:
	StackGpuAllocatorHandle m_handle;
	PtrSize m_range = 0;
	U8 m_frame = MAX_U8;

	Bool valid() const
	{
		return m_range != 0 && m_frame < MAX_U8;
	}

	void invalidate()
	{
		m_handle.m_memory = nullptr;
		m_handle.m_offset = 0;
		m_range = 0;
		m_frame = MAX_U8;
	}
};

/// GPU memory allocator for GPU buffers used in transfer operations.
class TransferGpuAllocator
{
	friend class TransferGpuAllocatorHandle;

public:
	static const U32 FRAME_COUNT = 3;
	static const PtrSize CHUNK_INITIAL_SIZE = 64_MB;
	static constexpr Second MAX_FENCE_WAIT_TIME = 500.0_ms;

	TransferGpuAllocator();

	~TransferGpuAllocator();

	ANKI_USE_RESULT Error init(PtrSize maxSize, GrManager* gr, ResourceAllocator<U8> alloc);

	/// Allocate some transfer memory. If there is not enough memory it will block until some is releaced. It's
	/// threadsafe.
	ANKI_USE_RESULT Error allocate(PtrSize size, TransferGpuAllocatorHandle& handle);

	/// Release the memory. It will not be recycled before the fence is signaled. It's threadsafe.
	void release(TransferGpuAllocatorHandle& handle, FencePtr fence);

private:
	class Interface;
	class Memory;

	ResourceAllocator<U8> m_alloc;
	GrManager* m_gr = nullptr;
	PtrSize m_maxAllocSize = 0;

	UniquePtr<Interface> m_interface;

	class Frame
	{
	public:
		StackGpuAllocator m_stackAlloc;
		List<FencePtr> m_fences;
		U32 m_pendingReleases = 0;
	};

	Mutex m_mtx; ///< Protect all members bellow.
	ConditionVariable m_condVar;
	Array<Frame, FRAME_COUNT> m_frames;
	U8 m_frameCount = 0;
	PtrSize m_crntFrameAllocatedSize = 0;
};
/// @}

} // end namespace anki
