// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

void AllGpuSceneContiguousArrays::ContiguousArrayAllocator::destroy(GpuSceneMemoryPool* gpuScene,
																	HeapMemoryPool* cpuPool)
{
	for(U32 i = 0; i < kMaxFramesInFlight; ++i)
	{
		collectGarbage(i, gpuScene, cpuPool);
	}
}

AllGpuSceneContiguousArrays::ContiguousArrayAllocator::Index
AllGpuSceneContiguousArrays::ContiguousArrayAllocator::allocateObject(GpuSceneMemoryPool* gpuScene,
																	  HeapMemoryPool* cpuPool)
{
	ANKI_ASSERT(gpuScene && cpuPool);

	LockGuard lock(m_mtx);

	if(m_poolToken.m_offset == kMaxPtrSize)
	{
		// Initialize
		const U32 alignment = sizeof(U32);
		gpuScene->allocate(m_objectSize * m_initialArraySize, alignment, m_poolToken);
		m_nextSlotIndex = 0;

		m_freeSlotStack.create(*cpuPool, m_initialArraySize);
		for(U32 i = 0; i < m_initialArraySize; ++i)
		{
			m_freeSlotStack[i] = i;
		}
	}
	else if(m_nextSlotIndex == m_freeSlotStack.getSize())
	{
		// Grow
		ANKI_ASSERT(!"TODO");
	}

	const Index idx = m_freeSlotStack[m_nextSlotIndex];
	++m_nextSlotIndex;

	ANKI_ASSERT(idx < m_freeSlotStack.getSize());
	return idx;
}

void AllGpuSceneContiguousArrays::ContiguousArrayAllocator::deferredFree(U32 crntFrameIdx, HeapMemoryPool* cpuPool,
																		 Index index)
{
	ANKI_ASSERT(cpuPool);

	LockGuard lock(m_mtx);

	ANKI_ASSERT(index < m_freeSlotStack.getSize());
	m_garbage[crntFrameIdx].emplaceBack(*cpuPool, index);
}

void AllGpuSceneContiguousArrays::ContiguousArrayAllocator::collectGarbage(U32 newFrameIdx,
																		   GpuSceneMemoryPool* gpuScene,
																		   HeapMemoryPool* cpuPool)
{
	ANKI_ASSERT(gpuScene && cpuPool);

	LockGuard lock(m_mtx);

	if(m_garbage[newFrameIdx].getSize() == 0) [[likely]]
	{
		return;
	}

	// Release deferred frees
	for(Index idx : m_garbage[newFrameIdx])
	{
		ANKI_ASSERT(m_nextSlotIndex > 0);
		--m_nextSlotIndex;
		m_freeSlotStack[m_nextSlotIndex] = idx;
	}

	m_garbage[newFrameIdx].destroy(*cpuPool);

	// Sort so we can keep memory close to the beginning of the array for better cache behaviour
	std::sort(m_freeSlotStack.getBegin() + m_nextSlotIndex, m_freeSlotStack.getEnd());

	// Adjust the stack size
	const U32 allocatedSlots = m_nextSlotIndex;
	if(U32(F32(allocatedSlots) * m_growRate) < m_freeSlotStack.getSize()
	   && m_freeSlotStack.getSize() > m_initialArraySize)
	{
		// Shrink
		ANKI_ASSERT(!"TODO");
	}
	else if(allocatedSlots == 0)
	{
		ANKI_ASSERT(m_nextSlotIndex == 0);
		gpuScene->deferredFree(m_poolToken);
		m_freeSlotStack.destroy(*cpuPool);
	}
}

void AllGpuSceneContiguousArrays::init(SceneGraph* scene)
{
	m_scene = scene;

	const ConfigSet& cfg = *scene->m_subsystems.m_config;
	constexpr F32 kGrowRate = 2.0;

	U32 arraySize =
		cfg.getSceneMinGpuSceneTransforms() / m_componentCount[GpuSceneContiguousArrayType::kTransformPairs];
	m_allocs[GpuSceneContiguousArrayType::kTransformPairs].init(
		arraySize,
		m_componentSize[GpuSceneContiguousArrayType::kTransformPairs]
			* m_componentCount[GpuSceneContiguousArrayType::kTransformPairs],
		kGrowRate);

	arraySize = cfg.getSceneMinGpuSceneMeshes() / m_componentCount[GpuSceneContiguousArrayType::kMeshLods];
	m_allocs[GpuSceneContiguousArrayType::kMeshLods].init(
		arraySize,
		m_componentSize[GpuSceneContiguousArrayType::kMeshLods]
			* m_componentCount[GpuSceneContiguousArrayType::kMeshLods],
		kGrowRate);

	arraySize =
		cfg.getSceneMinGpuSceneParticleEmitters() / m_componentCount[GpuSceneContiguousArrayType::kParticleEmitters];
	m_allocs[GpuSceneContiguousArrayType::kParticleEmitters].init(
		arraySize,
		m_componentSize[GpuSceneContiguousArrayType::kParticleEmitters]
			* m_componentCount[GpuSceneContiguousArrayType::kParticleEmitters],
		kGrowRate);
}

void AllGpuSceneContiguousArrays::destroy()
{
	for(GpuSceneContiguousArrayType type : EnumIterable<GpuSceneContiguousArrayType>())
	{
		m_allocs[type].destroy(m_scene->m_subsystems.m_gpuSceneMemoryPool, &m_scene->m_pool);
	}
}

PtrSize AllGpuSceneContiguousArrays::allocate(GpuSceneContiguousArrayType type)
{
	const U32 idx = m_allocs[type].allocateObject(m_scene->m_subsystems.m_gpuSceneMemoryPool, &m_scene->m_pool);
	return PtrSize(idx) * m_componentCount[type] * m_componentSize[type] + m_allocs[type].m_poolToken.m_offset;
}

void AllGpuSceneContiguousArrays::deferredFree(GpuSceneContiguousArrayType type, PtrSize offset)
{
	ANKI_ASSERT(offset >= m_allocs[type].m_poolToken.m_offset);
	offset -= m_allocs[type].m_poolToken.m_offset;
	ANKI_ASSERT((offset % (m_componentCount[type] * m_componentSize[type])) == 0);
	const U32 idx = U32(offset / (m_componentCount[type] * m_componentSize[type]));
	m_allocs[type].deferredFree(m_frame, &m_scene->m_pool, idx);
}

void AllGpuSceneContiguousArrays::endFrame()
{
	m_frame = (m_frame + 1) % kMaxFramesInFlight;

	for(GpuSceneContiguousArrayType type : EnumIterable<GpuSceneContiguousArrayType>())
	{
		m_allocs[type].collectGarbage(m_frame, m_scene->m_subsystems.m_gpuSceneMemoryPool, &m_scene->m_pool);
	}
}

} // end namespace anki
