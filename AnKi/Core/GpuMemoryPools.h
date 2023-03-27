// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Utils/StackGpuMemoryPool.h>
#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>
#include <AnKi/Resource/ShaderProgramResource.h>

namespace anki {

/// @addtogroup core
/// @{

/// Manages vertex and index memory for the whole application.
class UnifiedGeometryMemoryPool : public MakeSingleton<UnifiedGeometryMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	UnifiedGeometryMemoryPool(const UnifiedGeometryMemoryPool&) = delete; // Non-copyable

	UnifiedGeometryMemoryPool& operator=(const UnifiedGeometryMemoryPool&) = delete; // Non-copyable

	void init(GrManager* gr);

	void allocate(PtrSize size, U32 alignment, SegregatedListsGpuMemoryPoolToken& token)
	{
		m_pool.allocate(size, alignment, token);
	}

	void deferredFree(SegregatedListsGpuMemoryPoolToken& token)
	{
		m_pool.deferredFree(token);
	}

	void endFrame()
	{
		m_pool.endFrame();
	}

	const BufferPtr& getBuffer() const
	{
		return m_pool.getGpuBuffer();
	}

	void getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const
	{
		m_pool.getStats(externalFragmentation, userAllocatedSize, totalSize);
	}

private:
	SegregatedListsGpuMemoryPool m_pool;

	UnifiedGeometryMemoryPool() = default;

	~UnifiedGeometryMemoryPool() = default;
};

/// Memory pool for the GPU scene.
class GpuSceneMemoryPool : public MakeSingleton<GpuSceneMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuSceneMemoryPool(const GpuSceneMemoryPool&) = delete; // Non-copyable

	GpuSceneMemoryPool& operator=(const GpuSceneMemoryPool&) = delete; // Non-copyable

	void init(GrManager* gr);

	void allocate(PtrSize size, U32 alignment, SegregatedListsGpuMemoryPoolToken& token)
	{
		m_pool.allocate(size, alignment, token);
	}

	void deferredFree(SegregatedListsGpuMemoryPoolToken& token)
	{
		m_pool.deferredFree(token);
	}

	void endFrame()
	{
		m_pool.endFrame();
	}

	const BufferPtr& getBuffer() const
	{
		return m_pool.getGpuBuffer();
	}

	void getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const
	{
		m_pool.getStats(externalFragmentation, userAllocatedSize, totalSize);
	}

	GrManager& getGrManager()
	{
		return m_pool.getGrManager();
	}

private:
	SegregatedListsGpuMemoryPool m_pool;

	GpuSceneMemoryPool() = default;

	~GpuSceneMemoryPool() = default;
};

/// Token that gets returned when requesting for memory to write to a resource.
class RebarGpuMemoryToken
{
public:
	PtrSize m_offset = 0;
	PtrSize m_range = 0;

	RebarGpuMemoryToken() = default;

	~RebarGpuMemoryToken() = default;

	Bool operator==(const RebarGpuMemoryToken& b) const
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
class RebarStagingGpuMemoryPool : public MakeSingleton<RebarStagingGpuMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	RebarStagingGpuMemoryPool(const RebarStagingGpuMemoryPool&) = delete; // Non-copyable

	~RebarStagingGpuMemoryPool();

	RebarStagingGpuMemoryPool& operator=(const RebarStagingGpuMemoryPool&) = delete; // Non-copyable

	void init(GrManager* gr);

	PtrSize endFrame();

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the
	/// N-(kMaxFramesInFlight-1) frame.
	void* allocateFrame(PtrSize size, RebarGpuMemoryToken& token);

	/// Allocate staging memory for various operations. The memory will be reclaimed at the begining of the
	/// N-(kMaxFramesInFlight-1) frame.
	void* tryAllocateFrame(PtrSize size, RebarGpuMemoryToken& token);

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

	RebarStagingGpuMemoryPool() = default;
};

/// Creates the copy jobs that will patch the GPU Scene.
class GpuSceneMicroPatcher : public MakeSingleton<GpuSceneMicroPatcher>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuSceneMicroPatcher(const GpuSceneMicroPatcher&) = delete;

	GpuSceneMicroPatcher& operator=(const GpuSceneMicroPatcher&) = delete;

	Error init();

	/// Copy data for the GPU scene to a staging buffer.
	/// @note It's thread-safe.
	void newCopy(StackMemoryPool& frameCpuPool, PtrSize gpuSceneDestOffset, PtrSize dataSize, const void* data);

	/// Check if there is a need to call patchGpuScene or if no copies are needed.
	/// @note Not thread-safe. Nothing else should be happening before calling it.
	Bool patchingIsNeeded() const
	{
		return m_crntFramePatchHeaders.getSize() > 0;
	}

	/// Copy the data to the GPU scene buffer.
	/// @note Not thread-safe. Nothing else should be happening before calling it.
	void patchGpuScene(CommandBuffer& cmdb);

private:
	static constexpr U32 kDwordsPerPatch = 64;

	class PatchHeader;

	DynamicArray<PatchHeader> m_crntFramePatchHeaders;
	DynamicArray<U32> m_crntFramePatchData;
	Mutex m_mtx;

	ShaderProgramResourcePtr m_copyProgram;
	ShaderProgramPtr m_grProgram;

	GpuSceneMicroPatcher() = default;

	~GpuSceneMicroPatcher();
};
/// @}

} // end namespace anki
