// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

template<typename TGpuSceneObject, U32 kId>
GpuSceneArray<TGpuSceneObject, kId>::GpuSceneArray(U32 maxArraySize)
{
	maxArraySize = getAlignedRoundUp(sizeof(SubMask), maxArraySize);
	const U32 alignment = GrManager::getSingleton().getDeviceCapabilities().m_storageBufferBindOffsetAlignment;
	GpuSceneBuffer::getSingleton().allocate(sizeof(TGpuSceneObject) * maxArraySize, alignment, m_gpuSceneAllocation);

	m_inUseIndicesMask.resize(maxArraySize / sizeof(SubMask), false);
	ANKI_ASSERT(m_inuUseIndicesCount == 0);
	m_maxInUseIndex = 0;
}

template<typename TGpuSceneObject, U32 kId>
GpuSceneArray<TGpuSceneObject, kId>::~GpuSceneArray()
{
	flushInternal(false);
	validate();
	ANKI_ASSERT(m_inuUseIndicesCount == 0 && "Forgot to free");
}

template<typename TGpuSceneObject, U32 kId>
GpuSceneArrayAllocation<TGpuSceneObject, kId> GpuSceneArray<TGpuSceneObject, kId>::allocate()
{
	LockGuard lock(m_mtx);

	U32 idx = kMaxU32;
	if(m_freedAllocations.getSize())
	{
		// There are freed indices this frame, use one

		idx = m_freedAllocations[0];
		m_freedAllocations.erase(m_freedAllocations.getBegin());
	}
	else
	{
		// No freed indices this frame, get a new one

		for(U maskGroup = 0; maskGroup < m_inUseIndicesMask.getSize(); ++maskGroup)
		{
			SubMask submask = m_inUseIndicesMask[maskGroup];
			submask = ~submask;
			const U32 bit = submask.getLeastSignificantBit();
			if(bit != kMaxU32)
			{
				// Found an index
				idx = maskGroup * 64 + bit;
				break;
			}
		}
	}

	ANKI_ASSERT(idx < m_inUseIndicesMask.getSize() * 64);

	ANKI_ASSERT(m_inUseIndicesMask[idx / 64].get(idx % 64) == false);
	m_inUseIndicesMask[idx / 64].set(idx % 64);

	m_maxInUseIndex = max(m_maxInUseIndex, idx);
	++m_inuUseIndicesCount;

	Allocation out;
	out.m_index = idx;
	return out;
}

template<typename TGpuSceneObject, U32 kId>
void GpuSceneArray<TGpuSceneObject, kId>::free(Allocation& alloc)
{
	if(!alloc.isValid()) [[unlikely]]
	{
		return;
	}

	const U32 idx = alloc.m_index;
	alloc.invalidate();

	LockGuard lock(m_mtx);

	m_freedAllocations.emplaceBack(idx);
	ANKI_ASSERT(m_inUseIndicesMask[idx / 64].get(idx % 64) == true);
	m_inUseIndicesMask[idx / 64].unset(idx % 64);

	ANKI_ASSERT(m_inuUseIndicesCount > 0);
	--m_inuUseIndicesCount;

	// Sort because allocations later on should preferably grab the lowest index possible
	std::sort(m_freedAllocations.getBegin(), m_freedAllocations.getEnd());
}

template<typename TGpuSceneObject, U32 kId>
void GpuSceneArray<TGpuSceneObject, kId>::flushInternal(Bool nullifyElements)
{
	LockGuard lock(m_mtx);

	if(m_freedAllocations.getSize())
	{
		// Nullify the deleted elements
		if(nullifyElements)
		{
			TGpuSceneObject nullObj = {};
			for(U32 idx : m_freedAllocations)
			{
				const PtrSize offset = idx * sizeof(TGpuSceneObject) + m_gpuSceneAllocation.getOffset();
				GpuSceneMicroPatcher::getSingleton().newCopy(SceneGraph::getSingleton().getFrameMemoryPool(), offset, nullObj);
			}
		}

		// Update the the last index
		U32 maskGroup = m_maxInUseIndex / 64;
		m_maxInUseIndex = 0;
		while(maskGroup--)
		{
			const U32 bit = m_inUseIndicesMask[maskGroup].getMostSignificantBit();
			if(bit != kMaxU32)
			{
				m_maxInUseIndex = maskGroup * 64 + bit;
				break;
			}
		}

		m_freedAllocations.destroy();
	}

	validate();
}

template<typename TGpuSceneObject, U32 kId>
void GpuSceneArray<TGpuSceneObject, kId>::validate() const
{
#if ANKI_ASSERTIONS_ENABLED
	U32 count = 0;
	U32 maxIdx = 0;
	U32 maskGroupCount = 0;
	for(const SubMask& mask : m_inUseIndicesMask)
	{
		count += mask.getEnabledBitCount();
		maxIdx = max(maxIdx, (mask.getMostSignificantBit() != kMaxU32) ? (mask.getMostSignificantBit() + maskGroupCount * 64) : 0);
		++maskGroupCount;
	}

	ANKI_ASSERT(count == m_inuUseIndicesCount);
	ANKI_ASSERT(maxIdx == m_maxInUseIndex);
#endif
}

} // end namespace anki
