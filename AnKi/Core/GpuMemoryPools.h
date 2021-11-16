// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Utils/FrameGpuAllocator.h>
#include <AnKi/Util/BuddyAllocatorBuilder.h>

namespace anki {

// Forward
class ConfigSet;

/// @addtogroup core
/// @{

/// Manages vertex and index memory for the whole application.
class VertexGpuMemoryPool
{
public:
	VertexGpuMemoryPool() = default;

	VertexGpuMemoryPool(const VertexGpuMemoryPool&) = delete; // Non-copyable

	~VertexGpuMemoryPool();

	VertexGpuMemoryPool& operator=(const VertexGpuMemoryPool&) = delete; // Non-copyable

	ANKI_USE_RESULT Error init(GenericMemoryPoolAllocator<U8> alloc, GrManager* gr, const ConfigSet& cfg);

	ANKI_USE_RESULT Error allocate(PtrSize size, PtrSize& offset);

	void free(PtrSize size, PtrSize offset);

	BufferPtr getVertexBuffer() const
	{
		return m_vertBuffer;
	}

private:
	GrManager* m_gr = nullptr;
	BufferPtr m_vertBuffer;
	BuddyAllocatorBuilder<32, Mutex> m_buddyAllocator;
};

enum class StagingGpuMemoryType : U8
{
	UNIFORM,
	STORAGE,
	VERTEX,
	TEXTURE,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(StagingGpuMemoryType)

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
class StagingGpuMemoryPool
{
public:
	StagingGpuMemoryPool() = default;

	StagingGpuMemoryPool(const StagingGpuMemoryPool&) = delete; // Non-copyable

	~StagingGpuMemoryPool();

	StagingGpuMemoryPool& operator=(const StagingGpuMemoryPool&) = delete; // Non-copyable

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

	void initBuffer(StagingGpuMemoryType type, U32 alignment, PtrSize maxAllocSize, BufferUsageBit usage,
					GrManager& gr);
};
/// @}

} // end namespace anki
