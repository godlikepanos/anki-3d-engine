// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/GpuMemory/GpuReadbackMemoryPool.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// A persistent GPU readback token. It's essentially a group of allocations.
class MultiframeReadbackToken
{
	friend class ReadbackManager;

private:
	Array<GpuReadbackMemoryAllocation, kMaxFramesInFlight> m_allocations;
	Array<U64, kMaxFramesInFlight> m_frameIds = {};
	U32 m_slot = 0;
};

/// A small class that is used to streamling the use of GPU readbacks.
class ReadbackManager
{
public:
	Error init()
	{
		// Just for the interface
		return Error::kNone;
	}

	/// Read the most up to date data from the GPU. 1st thing to call in a frame.
	void readMostRecentData(const MultiframeReadbackToken& token, void* data, PtrSize dataSize, PtrSize& dataOut) const;

	/// Read the most up to date data from the GPU.
	template<typename T, typename TMemPool>
	void readMostRecentData(const MultiframeReadbackToken& token, DynamicArray<T, TMemPool>& data) const
	{
		const U32 slot = findBestSlot(token);
		if(slot != kMaxU32 && token.m_allocations[slot])
		{
			const GpuReadbackMemoryAllocation& allocation = token.m_allocations[slot];

			data.resize(U32(allocation.getSize()) / sizeof(T));
			memcpy(&data[0], static_cast<const U8*>(allocation.getMappedMemory()), allocation.getSize());
		}
		else
		{
			data.resize(0);
		}
	}

	/// Allocate new data for the following frame. 2nd thing to call in a frame.
	template<typename T>
	BufferView allocateStructuredBuffer(MultiframeReadbackToken& token, U32 count) const
	{
		ANKI_ASSERT(count > 0);

		for([[maybe_unused]] U64 frame : token.m_frameIds)
		{
			ANKI_ASSERT(frame != m_frameId && "Can't allocate multiple times in a frame");
		}

		GpuReadbackMemoryAllocation& allocation = token.m_allocations[token.m_slot];

		if(allocation && allocation.getSize() != sizeof(T) * count)
		{
			GpuReadbackMemoryPool::getSingleton().deferredFree(allocation);
		}

		if(!allocation)
		{
			allocation = GpuReadbackMemoryPool::getSingleton().allocateStructuredBuffer<T>(count);
		}
		token.m_frameIds[token.m_slot] = m_frameId;

		token.m_slot = (token.m_slot + 1) % kMaxFramesInFlight;

		return BufferView(allocation).setRange(sizeof(T) * count);
	}

	/// Last thing to call in a frame.
	void endFrame(Fence* fence);

private:
	class Frame
	{
	public:
		FencePtr m_fence;
	};

	Array<Frame, kMaxFramesInFlight> m_frames;
	U64 m_frameId = kMaxFramesInFlight;

	U32 findBestSlot(const MultiframeReadbackToken& token) const;
};
/// @}

} // end namespace anki
