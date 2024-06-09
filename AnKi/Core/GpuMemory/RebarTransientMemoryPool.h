// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

/// @addtogroup core
/// @{

/// Token that gets returned when requesting for memory to write to a resource.
class RebarAllocation
{
	friend class RebarTransientMemoryPool;

public:
	RebarAllocation() = default;

	~RebarAllocation() = default;

	Bool operator==(const RebarAllocation& b) const
	{
		return m_offset == b.m_offset && m_range == b.m_range;
	}

	Bool isValid() const
	{
		return m_range != 0;
	}

	PtrSize getOffset() const
	{
		ANKI_ASSERT(isValid());
		return m_offset;
	}

	PtrSize getRange() const
	{
		ANKI_ASSERT(isValid());
		return m_range;
	}

	Buffer& getBuffer() const;

	operator BufferView() const;

private:
	PtrSize m_offset = kMaxPtrSize;
	PtrSize m_range = 0;
};

/// Manages staging GPU memory.
class RebarTransientMemoryPool : public MakeSingleton<RebarTransientMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	RebarTransientMemoryPool(const RebarTransientMemoryPool&) = delete; // Non-copyable

	RebarTransientMemoryPool& operator=(const RebarTransientMemoryPool&) = delete; // Non-copyable

	void init();

	void endFrame();

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the N-(kMaxFramesInFlight-1) frame.
	RebarAllocation allocateFrame(PtrSize size, void*& mappedMem);

	template<typename T>
	RebarAllocation allocateFrame(U32 count, T*& mappedMem)
	{
		void* mem;
		const RebarAllocation out = allocateFrame(count * sizeof(T), mem);
		mappedMem = static_cast<T*>(mem);
		return out;
	}

	template<typename T>
	RebarAllocation allocateFrame(U32 count, WeakArray<T>& arr)
	{
		void* mem;
		const RebarAllocation out = allocateFrame(count * sizeof(T), mem);
		arr = {static_cast<T*>(mem), count};
		return out;
	}

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the N-(kMaxFramesInFlight-1) frame.
	RebarAllocation tryAllocateFrame(PtrSize size, void*& mappedMem);

	ANKI_PURE Buffer& getBuffer() const
	{
		return *m_buffer;
	}

	U8* getBufferMappedAddress()
	{
		return m_mappedMem;
	}

private:
	BufferPtr m_buffer;
	U8* m_mappedMem = nullptr; ///< Cache it.
	PtrSize m_bufferSize = 0; ///< Cache it.
	Atomic<PtrSize> m_offset = {0};
	PtrSize m_previousFrameEndOffset = 0;
	U32 m_alignment = 0;

	RebarTransientMemoryPool() = default;

	~RebarTransientMemoryPool();
};

inline Buffer& RebarAllocation::getBuffer() const
{
	return RebarTransientMemoryPool::getSingleton().getBuffer();
}

inline RebarAllocation::operator BufferView() const
{
	ANKI_ASSERT(isValid());
	return {&RebarTransientMemoryPool::getSingleton().getBuffer(), m_offset, m_range};
}
/// @}

} // end namespace anki
