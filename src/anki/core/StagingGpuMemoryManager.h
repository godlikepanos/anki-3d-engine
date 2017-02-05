// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Buffer.h>
#include <anki/gr/common/FrameGpuAllocator.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup core
/// @{

enum class StagingGpuMemoryType
{
	UNIFORM,
	STORAGE,
	VERTEX,
	TRANSFER,
	TEXTURE,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(StagingGpuMemoryType, inline)

/// Token that gets returned when requesting for memory to write to a resource.
class StagingGpuMemoryToken
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset = 0;
	PtrSize m_range = 0;
	StagingGpuMemoryType m_type = StagingGpuMemoryType::COUNT;

	StagingGpuMemoryToken() = default;

	~StagingGpuMemoryToken() = default;

	operator Bool() const
	{
		return m_range != 0;
	}

	Bool operator==(const StagingGpuMemoryToken& b) const
	{
		return m_buffer == b.m_buffer && m_offset == b.m_offset && m_range == b.m_range && m_type == b.m_type;
	}

	void markUnused()
	{
		m_offset = m_range = MAX_U32;
	}

	Bool isUnused() const
	{
		return m_offset == MAX_U32 && m_range == MAX_U32;
	}
};

/// Manages staging GPU memory.
class StagingGpuMemoryManager : public NonCopyable
{
public:
	StagingGpuMemoryManager() = default;

	~StagingGpuMemoryManager();

	ANKI_USE_RESULT Error init(GrManager* gr, const ConfigSet& cfg);

	void endFrame();

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the
	/// N-(MAX_FRAMES_IN_FLIGHT-1) frame.
	void* allocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token);

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the
	/// N-(MAX_FRAMES_IN_FLIGHT-1) frame.
	void* tryAllocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token);

private:
	class PerFrameBuffer
	{
	public:
		PtrSize m_size = 0;
		BufferPtr m_buff;
		U8* m_mappedMem = nullptr; ///< Cache it
		FrameGpuAllocator m_alloc;
	};

	GrManager* m_gr = nullptr;
	Array<PerFrameBuffer, U(StagingGpuMemoryType::COUNT)> m_perFrameBuffers;

	void initBuffer(
		StagingGpuMemoryType type, U32 alignment, PtrSize maxAllocSize, BufferUsageBit usage, GrManager& gr);
};
/// @}

} // end namespace anki
