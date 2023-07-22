// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr.h>
#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <AnKi/Renderer/Utils/Readback.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Multiple passes for GPU visibility of non-renderable entities.
class PrimaryNonRenderableVisibility : public RendererObject
{
public:
	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph(RenderingContext& ctx);

private:
	Array<MultiframeReadbackToken, U32(GpuSceneNonRenderableObjectTypeWithFeedback::kCount)> m_readbacks;

	class
	{
	public:
		Array<BufferHandle, U32(GpuSceneNonRenderableObjectType::kCount)> m_visOutBufferHandle;

		/// Feedback from the GPU. It's an array of object UUID and array index.
		Array<WeakArray<UVec2>, U32(GpuSceneNonRenderableObjectTypeWithFeedback::kCount)> m_uuidArrayIndexPairs;
	} m_runCtx;
};
/// @}

} // end namespace anki
