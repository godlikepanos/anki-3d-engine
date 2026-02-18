// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/GpuMemory/TextureMemoryPool.h>

namespace anki {

// Iterates the scene particle components and initiates GPU simulations
class GpuParticles : public RendererObject
{
public:
	static constexpr U32 kMaxEmittersToSimulate = 16;

	Error init();

	void populateRenderGraph();

private:
	RendererDynamicArray<TextureMemoryPoolAllocation> m_scratchBuffers;

	class ReadbackData
	{
	public:
		MultiframeReadbackToken m_readback;
		U64 m_lastFrameSeen = 0;
	};

	RendererHashMap<U32, ReadbackData> m_readbacks; // One readback per emitter
};

} // end namespace anki
