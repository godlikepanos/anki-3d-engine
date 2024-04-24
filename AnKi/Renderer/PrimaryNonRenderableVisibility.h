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

/// Contains some interesting visible scene components that will be used by various renderer systems.
/// @memberof PrimaryNonRenderableVisibility
class InterestingVisibleComponents
{
public:
	WeakArray<LightComponent*> m_shadowLights;
	WeakArray<ReflectionProbeComponent*> m_reflectionProbes;
	WeakArray<GlobalIlluminationProbeComponent*> m_globalIlluminationProbes;
};

/// Multiple passes for GPU visibility of non-renderable entities.
class PrimaryNonRenderableVisibility : public RendererObject
{
public:
	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph(RenderingContext& ctx);

	const InterestingVisibleComponents& getInterestingVisibleComponents() const
	{
		return m_runCtx.m_interestingComponents;
	}

	BufferHandle getVisibleIndicesBufferHandle(GpuSceneNonRenderableObjectType type) const
	{
		return m_runCtx.m_visibleIndicesHandles[type];
	}

	const BufferView& getVisibleIndicesBuffer(GpuSceneNonRenderableObjectType type) const
	{
		return m_runCtx.m_visibleIndicesBuffers[type];
	}

private:
	Array<MultiframeReadbackToken, U32(GpuSceneNonRenderableObjectTypeWithFeedback::kCount)> m_readbacks;

	class
	{
	public:
		Array<BufferHandle, U32(GpuSceneNonRenderableObjectType::kCount)> m_visibleIndicesHandles;
		Array<BufferView, U32(GpuSceneNonRenderableObjectType::kCount)> m_visibleIndicesBuffers;

		/// Feedback from the GPU
		InterestingVisibleComponents m_interestingComponents;
	} m_runCtx;
};
/// @}

} // end namespace anki
