// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
public:
	PtrSize m_offset = 0;
	PtrSize m_range = 0;

	RebarAllocation() = default;

	~RebarAllocation() = default;

	Bool operator==(const RebarAllocation& b) const
	{
		return m_offset == b.m_offset && m_range == b.m_range;
	}

	void markUnused()
	{
		m_offset = m_range = kMaxU32;
	}

	Bool isUnused() const
	{
		return m_offset == kMaxU32 && m_range == kMaxU32;
	}
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

	PtrSize endFrame();

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the N-(kMaxFramesInFlight-1) frame.
	void* allocateFrame(PtrSize size, RebarAllocation& token);

	template<typename T>
	T* allocateFrame(U32 count, RebarAllocation& token)
	{
		return static_cast<T*>(allocateFrame(count * sizeof(T), token));
	}

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the N-(kMaxFramesInFlight-1) frame.
	void* tryAllocateFrame(PtrSize size, RebarAllocation& token);

	ANKI_PURE const BufferPtr& getBuffer() const
	{
		return m_buffer;
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
/// @}

} // end namespace anki
