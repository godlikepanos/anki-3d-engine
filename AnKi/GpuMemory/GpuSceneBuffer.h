// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Assert.h>

namespace anki {

ANKI_CVAR(NumericCVar<PtrSize>, Core, GpuSceneInitialSize, 64_MB, 16_MB, 2_GB, "Global memory for the GPU scene")

class GpuSceneBufferAllocation
{
	friend class GpuSceneBuffer;

public:
	GpuSceneBufferAllocation() = default;

	GpuSceneBufferAllocation(const GpuSceneBufferAllocation&) = delete;

	GpuSceneBufferAllocation(GpuSceneBufferAllocation&& b)
	{
		*this = std::move(b);
	}

	~GpuSceneBufferAllocation();

	GpuSceneBufferAllocation& operator=(const GpuSceneBufferAllocation&) = delete;

	GpuSceneBufferAllocation& operator=(GpuSceneBufferAllocation&& b)
	{
		ANKI_ASSERT(!isValid() && "Forgot to delete");
		m_token = b.m_token;
		b.m_token = {};
		return *this;
	}

	explicit operator Bool() const
	{
		return isValid();
	}

	Bool isValid() const
	{
		return m_token.m_offset != kMaxPtrSize;
	}

	// Get offset in the Unified Geometry Buffer buffer.
	U32 getOffset() const
	{
		ANKI_ASSERT(isValid());
		return U32(m_token.m_offset);
	}

	U32 getAllocatedSize() const
	{
		ANKI_ASSERT(isValid());
		return U32(m_token.m_size);
	}

private:
	SegregatedListsGpuMemoryPoolToken m_token;
};

// Memory pool for the GPU scene.
class GpuSceneBuffer : public MakeSingleton<GpuSceneBuffer>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuSceneBuffer(const GpuSceneBuffer&) = delete; // Non-copyable

	GpuSceneBuffer& operator=(const GpuSceneBuffer&) = delete; // Non-copyable

	void init();

	GpuSceneBufferAllocation allocate(PtrSize size, U32 alignment)
	{
		GpuSceneBufferAllocation alloc;
		m_pool.allocate(size, alignment, alloc.m_token);
		return alloc;
	}

	template<typename T>
	GpuSceneBufferAllocation allocateStructuredBuffer(U32 count)
	{
		const U32 alignment = (GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
								  ? sizeof(T)
								  : GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
		GpuSceneBufferAllocation alloc;
		m_pool.allocate(count * sizeof(T), alignment, alloc.m_token);
		return alloc;
	}

	void deferredFree(GpuSceneBufferAllocation& alloc)
	{
		m_pool.deferredFree(alloc.m_token);
	}

	void endFrame()
	{
		m_pool.endFrame();
#if ANKI_STATS_ENABLED
		updateStats();
#endif
	}

	Buffer& getBuffer() const
	{
		return m_pool.getGpuBuffer();
	}

	BufferView getBufferView() const
	{
		return BufferView(&m_pool.getGpuBuffer());
	}

private:
	SegregatedListsGpuMemoryPool m_pool;

	GpuSceneBuffer() = default;

	~GpuSceneBuffer() = default;

	void updateStats() const;
};

inline GpuSceneBufferAllocation::~GpuSceneBufferAllocation()
{
	GpuSceneBuffer::getSingleton().deferredFree(*this);
}

// Creates the copy jobs that will patch the GPU Scene.
class GpuSceneMicroPatcher : public MakeSingleton<GpuSceneMicroPatcher>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuSceneMicroPatcher(const GpuSceneMicroPatcher&) = delete;

	GpuSceneMicroPatcher& operator=(const GpuSceneMicroPatcher&) = delete;

	Error init();

	// 1st thing to call before any calls to newCopy
	// Note: Not thread-safe
	void beginPatching();

	// 2nd thing to call
	// Copy data for the GPU scene to a staging buffer.
	// Note: It's thread-safe against other newCopy()
	void newCopy(PtrSize gpuSceneDestOffset, PtrSize dataSize, const void* data);

	// See newCopy
	template<typename T>
	void newCopy(PtrSize gpuSceneDestOffset, const T& value)
	{
		newCopy(gpuSceneDestOffset, sizeof(value), &value);
	}

	// See newCopy
	void newCopy(const GpuSceneBufferAllocation& dest, PtrSize dataSize, const void* data)
	{
		ANKI_ASSERT(dataSize <= dest.getAllocatedSize());
		newCopy(dest.getOffset(), dataSize, data);
	}

	// See newCopy
	template<typename T>
	void newCopy(const GpuSceneBufferAllocation& dest, const T& value)
	{
		ANKI_ASSERT(sizeof(value) <= dest.getAllocatedSize());
		newCopy(dest.getOffset(), sizeof(value), &value);
	}

	// 3rd thing to call after all newCopy() calls have be done
	// Note: Not thread-safe
	void endPatching()
	{
		ANKI_ASSERT(m_bPatchingMode.fetchSub(1) == 1);
	}

	// 4th optional thing to call. Check if there is a need to call patchGpuScene or if no copies are needed
	// Note: Not thread-safe
	Bool patchingIsNeeded() const
	{
		ANKI_ASSERT(m_bPatchingMode.load() == 0);
		return m_crntFramePatchHeaders.getSize() > 0;
	}

	// 5th thing to call to copy all scratch data to the GPU scene buffer
	// Note: Not thread-safe
	void patchGpuScene(CommandBuffer& cmdb);

private:
	static constexpr U32 kDwordsPerPatch = 64; // If you change this change the bellow as well
	static constexpr U32 kDwordsPerPatchBitCount = 6;

	class PatchHeader;

	DynamicArray<PatchHeader, MemoryPoolPtrWrapper<StackMemoryPool>> m_crntFramePatchHeaders;
	DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> m_crntFramePatchData;
	Mutex m_mtx;

	ShaderProgramResourcePtr m_copyProgram;
	ShaderProgramPtr m_grProgram;

	StackMemoryPool m_stackMemPool;

#if ANKI_ASSERTIONS_ENABLED
	Atomic<U32> m_bPatchingMode = {0};
#endif

	GpuSceneMicroPatcher();

	~GpuSceneMicroPatcher();
};

} // end namespace anki
