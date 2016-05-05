// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Manages pre-allocated, always mapped GPU memory for per frame usage.
class GpuFrameRingAllocator : public NonCopyable
{
	friend class DynamicMemorySerializeCommand;

public:
	GpuFrameRingAllocator()
	{
	}

	~GpuFrameRingAllocator()
	{
	}

	/// Initialize with pre-allocated always mapped memory.
	/// @param[in] cpuMappedMem Pre-allocated always mapped GPU memory.
	/// @param size The size of the cpuMappedMem.
	/// @param alignment The working alignment.
	/// @param maxAllocationSize The size in @a allocate cannot exceed
	///        maxAllocationSize.
	void init(void* cpuMappedMem,
		PtrSize size,
		U32 alignment,
		PtrSize maxAllocationSize = MAX_PTR_SIZE);

	/// Allocate memory for a dynamic buffer.
	ANKI_USE_RESULT void* allocate(PtrSize size, DynamicBufferToken& token);

	/// Call this at the end of the frame.
	/// @return The bytes that were not used. Used for statistics.
	PtrSize endFrame();

private:
	U8* m_cpuAddress = nullptr; ///< Host address of the buffer.
	PtrSize m_size = 0; ///< The full size of the buffer.
	U32 m_alignment = 0; ///< Always work in that alignment.
	PtrSize m_maxAllocationSize = 0; ///< For debugging.

	Atomic<PtrSize> m_offset = {0};
	U64 m_frame = 0;
};
/// @}

} // end namespace anki