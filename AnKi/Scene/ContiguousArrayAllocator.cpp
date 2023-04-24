// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ContiguousArrayAllocator.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

void GpuSceneContiguousArrays::ContiguousArrayAllocator::destroy()
{
	for(U32 i = 0; i < kMaxFramesInFlight; ++i)
	{
		collectGarbage(i);
	}
}

U32 GpuSceneContiguousArrays::ContiguousArrayAllocator::allocateObject()
{
	LockGuard lock(m_mtx);

	if(!m_allocation.isValid())
	{
		// Initialize
		const U32 alignment = GrManager::getSingleton().getDeviceCapabilities().m_storageBufferBindOffsetAlignment;
		GpuSceneBuffer::getSingleton().allocate(m_objectSize * m_initialArraySize, alignment, m_allocation);
		m_nextSlotIndex = 0;

		m_freeSlotStack.resize(m_initialArraySize);
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

	const U32 idx = m_freeSlotStack[m_nextSlotIndex];
	++m_nextSlotIndex;

	ANKI_ASSERT(idx < m_freeSlotStack.getSize());
	return idx;
}

void GpuSceneContiguousArrays::ContiguousArrayAllocator::deferredFree(U32 crntFrameIdx, U32 index)
{
	LockGuard lock(m_mtx);

	ANKI_ASSERT(index < m_freeSlotStack.getSize());
	m_garbage[crntFrameIdx].emplaceBack(index);
}

void GpuSceneContiguousArrays::ContiguousArrayAllocator::collectGarbage(U32 newFrameIdx)
{
	LockGuard lock(m_mtx);

	if(m_garbage[newFrameIdx].getSize() == 0) [[likely]]
	{
		return;
	}

	// Release deferred frees
	for(U32 idx : m_garbage[newFrameIdx])
	{
		ANKI_ASSERT(m_nextSlotIndex > 0);
		--m_nextSlotIndex;
		m_freeSlotStack[m_nextSlotIndex] = idx;
	}

	m_garbage[newFrameIdx].destroy();

	// Sort so we can keep memory close to the beginning of the array for better cache behaviour
	std::sort(m_freeSlotStack.getBegin() + m_nextSlotIndex, m_freeSlotStack.getEnd());

	// Adjust the stack size
	const U32 allocatedSlots = m_nextSlotIndex;
	if(U32(F32(allocatedSlots) * m_growRate) < m_freeSlotStack.getSize() && m_freeSlotStack.getSize() > m_initialArraySize)
	{
		// Shrink
		ANKI_ASSERT(!"TODO");
	}
	else if(allocatedSlots == 0)
	{
		ANKI_ASSERT(m_nextSlotIndex == 0);
		GpuSceneBuffer::getSingleton().deferredFree(m_allocation);
		m_freeSlotStack.destroy();
	}
}

GpuSceneContiguousArrays::GpuSceneContiguousArrays()
{
	const ConfigSet& cfg = ConfigSet::getSingleton();
	constexpr F32 kGrowRate = 2.0;

	const Array<U32, U32(GpuSceneContiguousArrayType::kCount)> minElementCount = {cfg.getSceneMinGpuSceneTransforms(),
																				  cfg.getSceneMinGpuSceneMeshes(),
																				  cfg.getSceneMinGpuSceneParticleEmitters(),
																				  cfg.getSceneMinGpuSceneLights(),
																				  cfg.getSceneMinGpuSceneLights(),
																				  cfg.getSceneMinGpuSceneReflectionProbes(),
																				  cfg.getSceneMinGpuSceneGlobalIlluminationProbes(),
																				  cfg.getSceneMinGpuSceneDecals(),
																				  cfg.getSceneMinGpuSceneFogDensityVolumes(),
																				  cfg.getSceneMinGpuSceneRenderables(),
																				  cfg.getSceneMinGpuSceneRenderables(),
																				  cfg.getSceneMinGpuSceneRenderables(),
																				  cfg.getSceneMinGpuSceneRenderables()};

	for(GpuSceneContiguousArrayType type : EnumIterable<GpuSceneContiguousArrayType>())
	{
		const U32 initialArraySize = minElementCount[type] / m_componentCount[type];
		const U16 elementSize = m_componentSize[type] * m_componentCount[type];

		m_allocs[type].init(initialArraySize, elementSize, kGrowRate);
	}
}

GpuSceneContiguousArrays::~GpuSceneContiguousArrays()
{
	for(GpuSceneContiguousArrayType type : EnumIterable<GpuSceneContiguousArrayType>())
	{
		m_allocs[type].destroy();
	}
}

GpuSceneContiguousArrayIndex GpuSceneContiguousArrays::allocate(GpuSceneContiguousArrayType type)
{
	GpuSceneContiguousArrayIndex out;
	out.m_index = m_allocs[type].allocateObject();
	out.m_type = type;

	return out;
}

void GpuSceneContiguousArrays::deferredFree(GpuSceneContiguousArrayIndex& idx)
{
	if(idx.isValid())
	{
		m_allocs[idx.m_type].deferredFree(m_frame, idx.m_index);
		idx.invalidate();
	}
}

void GpuSceneContiguousArrays::endFrame()
{
	m_frame = (m_frame + 1) % kMaxFramesInFlight;

	for(GpuSceneContiguousArrayType type : EnumIterable<GpuSceneContiguousArrayType>())
	{
		m_allocs[type].collectGarbage(m_frame);
	}
}

} // end namespace anki
