// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Core/GpuMemory/GpuReadbackMemoryPool.h>

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
		if(slot != kMaxU32 && token.m_allocations[slot].isValid())
		{
			const GpuReadbackMemoryAllocation& allocation = token.m_allocations[slot];

			data.resize(allocation.getAllocatedSize() / sizeof(T));
			memcpy(&data[0], static_cast<const U8*>(allocation.getMappedMemory()), allocation.getAllocatedSize());
		}
		else
		{
			data.resize(0);
		}
	}

	/// Allocate new data for the following frame. 2nd thing to call in a frame.
	void allocateData(MultiframeReadbackToken& token, PtrSize size, BufferView& buffer) const;

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
