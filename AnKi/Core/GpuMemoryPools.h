// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
class UnifiedGeometryMemoryPool
{
public:
	UnifiedGeometryMemoryPool() = default;

	UnifiedGeometryMemoryPool(const UnifiedGeometryMemoryPool&) = delete; // Non-copyable

	~UnifiedGeometryMemoryPool();

	UnifiedGeometryMemoryPool& operator=(const UnifiedGeometryMemoryPool&) = delete; // Non-copyable

	Error init(HeapMemoryPool* pool, GrManager* gr, const ConfigSet& cfg);

	Error allocate(PtrSize size, U32 alignment, PtrSize& offset);

	void free(PtrSize size, U32 alignment, PtrSize offset);

	BufferPtr getVertexBuffer() const
	{
		return m_vertBuffer;
	}

	void getMemoryStats(BuddyAllocatorBuilderStats& stats) const
	{
		m_buddyAllocator.getStats(stats);
	}

private:
	GrManager* m_gr = nullptr;
	BufferPtr m_vertBuffer;
	BuddyAllocatorBuilder<32, Mutex> m_buddyAllocator;
};

enum class StagingGpuMemoryType : U8
{
	kUniform,
	kStorage,
	kVertex,
	kTexture,

	kCount,
	kFirst = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(StagingGpuMemoryType)

/// Token that gets returned when requesting for memory to write to a resource.
class StagingGpuMemoryToken
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset = 0;
	PtrSize m_range = 0;
	StagingGpuMemoryType m_type = StagingGpuMemoryType::kCount;

	StagingGpuMemoryToken() = default;

	~StagingGpuMemoryToken() = default;

	Bool operator==(const StagingGpuMemoryToken& b) const
	{
		return m_buffer == b.m_buffer && m_offset == b.m_offset && m_range == b.m_range && m_type == b.m_type;
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
class StagingGpuMemoryPool
{
public:
	StagingGpuMemoryPool() = default;

	StagingGpuMemoryPool(const StagingGpuMemoryPool&) = delete; // Non-copyable

	~StagingGpuMemoryPool();

	StagingGpuMemoryPool& operator=(const StagingGpuMemoryPool&) = delete; // Non-copyable

	Error init(GrManager* gr, const ConfigSet& cfg);

	void endFrame();

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the
	/// N-(kMaxFramesInFlight-1) frame.
	void* allocateFrame(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token);

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the
	/// N-(kMaxFramesInFlight-1) frame.
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
	Array<PerFrameBuffer, U(StagingGpuMemoryType::kCount)> m_perFrameBuffers;

	void initBuffer(StagingGpuMemoryType type, U32 alignment, PtrSize maxAllocSize, BufferUsageBit usage,
					GrManager& gr);
};
/// @}

} // end namespace anki
