// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>
#include <AnKi/Resource/ShaderProgramResource.h>

namespace anki {

/// @addtogroup core
/// @{

/// @memberof GpuSceneBuffer
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

	Bool isValid() const
	{
		return m_token.m_offset != kMaxPtrSize;
	}

	/// Get offset in the Unified Geometry Buffer buffer.
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

/// Memory pool for the GPU scene.
class GpuSceneBuffer : public MakeSingleton<GpuSceneBuffer>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuSceneBuffer(const GpuSceneBuffer&) = delete; // Non-copyable

	GpuSceneBuffer& operator=(const GpuSceneBuffer&) = delete; // Non-copyable

	void init();

	void allocate(PtrSize size, U32 alignment, GpuSceneBufferAllocation& alloc)
	{
		m_pool.allocate(size, alignment, alloc.m_token);
	}

	void deferredFree(GpuSceneBufferAllocation& alloc)
	{
		m_pool.deferredFree(alloc.m_token);
	}

	void endFrame()
	{
		m_pool.endFrame();
	}

	Buffer& getBuffer() const
	{
		return m_pool.getGpuBuffer();
	}

	void getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const
	{
		m_pool.getStats(externalFragmentation, userAllocatedSize, totalSize);
	}

private:
	SegregatedListsGpuMemoryPool m_pool;

	GpuSceneBuffer() = default;

	~GpuSceneBuffer() = default;
};

inline GpuSceneBufferAllocation::~GpuSceneBufferAllocation()
{
	GpuSceneBuffer::getSingleton().deferredFree(*this);
}

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

	template<typename T>
	void newCopy(StackMemoryPool& frameCpuPool, PtrSize gpuSceneDestOffset, const T& value)
	{
		newCopy(frameCpuPool, gpuSceneDestOffset, sizeof(value), &value);
	}

	/// @see newCopy
	void newCopy(StackMemoryPool& frameCpuPool, const GpuSceneBufferAllocation& dest, PtrSize dataSize, const void* data)
	{
		ANKI_ASSERT(dataSize <= dest.getAllocatedSize());
		newCopy(frameCpuPool, dest.getOffset(), dataSize, data);
	}

	/// @see newCopy
	template<typename T>
	void newCopy(StackMemoryPool& frameCpuPool, const GpuSceneBufferAllocation& dest, const T& value)
	{
		ANKI_ASSERT(sizeof(value) <= dest.getAllocatedSize());
		newCopy(frameCpuPool, dest.getOffset(), sizeof(value), &value);
	}

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

	DynamicArray<PatchHeader, MemoryPoolPtrWrapper<StackMemoryPool>> m_crntFramePatchHeaders;
	DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> m_crntFramePatchData;
	Mutex m_mtx;

	ShaderProgramResourcePtr m_copyProgram;
	ShaderProgramPtr m_grProgram;

	GpuSceneMicroPatcher();

	~GpuSceneMicroPatcher();
};
/// @}

} // end namespace anki
