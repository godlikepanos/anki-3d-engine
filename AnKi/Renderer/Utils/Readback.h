// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/GpuMemory/GpuReadbackMemoryPool.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

// A persistent GPU readback token. It's essentially a group of allocations.
class MultiframeReadbackToken
{
	friend class ReadbackManager;

private:
	class Allocation
	{
	public:
		GpuReadbackMemoryAllocation m_alloc;
		U64 m_frameIndex = kMaxU64;
	};

	RendererDynamicArray<Allocation> m_allocations;
};

// A small class that is used to streamling the use of GPU readbacks.
class ReadbackManager
{
public:
	Error init()
	{
		// Just for the interface
		return Error::kNone;
	}

	// Read the most up to date data from the GPU. 1st thing to call in a frame.
	void readMostRecentData(const MultiframeReadbackToken& token, void* data, PtrSize dataSize, PtrSize& dataOut) const;

	// Read the most up to date data from the GPU.
	template<typename T, typename TMemPool>
	void readMostRecentData(const MultiframeReadbackToken& token, DynamicArray<T, TMemPool>& data) const
	{
		const GpuReadbackMemoryAllocation* alloc = findBestAllocation(token);
		if(alloc)
		{
			data.resize(U32(alloc->getSize()) / sizeof(T));
			memcpy(&data[0], alloc->getMappedMemory(), alloc->getSize());
		}
		else
		{
			data.resize(0);
		}
	}

	// Allocate new data for the following frame. 2nd thing to call in a frame.
	template<typename T>
	BufferView allocateStructuredBuffer(MultiframeReadbackToken& token, U32 count)
	{
		ANKI_ASSERT(count > 0);

		for(const MultiframeReadbackToken::Allocation& a : token.m_allocations)
		{
			ANKI_ASSERT(a.m_frameIndex != m_crntFrame && "Can't allocate multiple times in a frame");
		}

		auto it = token.m_allocations.getBegin();
		for(; it != token.m_allocations.getEnd(); ++it)
		{
			if(it->m_frameIndex <= m_lastFinishedFrame)
			{
				break;
			}
		}

		const PtrSize allocationSize = sizeof(T) * count;

		MultiframeReadbackToken::Allocation* alloc;
		if(it == token.m_allocations.getEnd())
		{
			alloc = token.m_allocations.emplaceBack();
		}
		else
		{
			alloc = &(*it);

			if(alloc->m_alloc.getSize() != allocationSize)
			{
				GpuReadbackMemoryPool::getSingleton().deferredFree(alloc->m_alloc);
			}
		}

		if(!alloc->m_alloc)
		{
			alloc->m_alloc = GpuReadbackMemoryPool::getSingleton().allocateStructuredBuffer<T>(count);
		}

		alloc->m_frameIndex = m_crntFrame;

		return BufferView(alloc->m_alloc);
	}

	// Last thing to call in a frame.
	void endFrame(Fence* fence);

private:
	class Frame
	{
	public:
		FencePtr m_fence;
		U64 m_frameIndex = kMaxU64;
		U32 m_blockArrayIndex = kMaxU32;
	};

	RendererBlockArray<Frame> m_frames;
	U64 m_crntFrame = 1;
	U64 m_lastFinishedFrame = 0;

	const GpuReadbackMemoryAllocation* findBestAllocation(const MultiframeReadbackToken& token) const;
};

} // end namespace anki
