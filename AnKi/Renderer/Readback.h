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

/// TODO
class MultiframeReadbackToken
{
	friend class ReadbackManager;

private:
	Array<GpuReadbackMemoryAllocation, kMaxFramesInFlight> m_allocations;
	Array<U64, kMaxFramesInFlight> m_frameIds = {};
	U32 m_slot = 0;
};

/// TODO
class ReadbackManager
{
	template<typename>
	friend class MakeSingleton;

public:
	/// @note Not thread-safe
	void allocateData(MultiframeReadbackToken& token, PtrSize size, Buffer*& buffer, PtrSize& bufferOffset);

	/// XXX
	/// @note Not thread-safe
	void getMostRecentReadDataAndRelease(MultiframeReadbackToken& token, void* data, PtrSize dataSize, PtrSize& dataOut);

	/// @note Not thread-safe
	template<typename TMemPool>
	void getMostRecentReadDataAndRelease(MultiframeReadbackToken& token, DynamicArray<U8, TMemPool>& data);

	/// @note Not thread-safe
	void endFrame(Fence* fence);

private:
	class Frame
	{
	public:
		FencePtr m_fence;
	};

	Array<Frame, kMaxFramesInFlight> m_frames;
	U64 m_frameId = kMaxFramesInFlight;
};
/// @}

} // end namespace anki
