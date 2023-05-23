// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Core/GpuMemory/GpuReadbackMemoryPool.h>

namespace anki {

/// @addtogroup renderer
/// @{

class MultiframeReadback
{
public:
	void* getMostRecentReadData(PtrSize* dataSize = nullptr) const
	{
	}

	void freeMostRecentData()
	{
	}

	void allocateData(PtrSize size, Buffer*& buffer, PtrSize bufferOffset)
	{
		if(m_fences[m_crntSlot].isCreated())
		{
			ANKI_R_LOGW("Allocation not freed. Will have to free it now");
			GpuReadbackMemoryPool::getSingleton().deferredFree(m_allocations[m_crntSlot]);
			m_fences[m_crntSlot].reset(nullptr);
		}

		m_crntSlot = (m_crntSlot + 1) % kMaxFramesInFlight;
	}

private:
	Array<GpuReadbackMemoryAllocation, kMaxFramesInFlight> m_allocations;
	Array<U64, kMaxFramesInFlight> m_frames;
};

class ReadbackManager : public MakeSingleton<ReadbackManager>
{
	template<typename>
	friend class MakeSingleton;

public:
private:
};

/// @}

} // end namespace anki
