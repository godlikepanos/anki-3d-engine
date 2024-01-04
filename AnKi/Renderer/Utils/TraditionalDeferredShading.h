// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Parameters to be passed to TraditionalDeferredLightShading::drawLights.
class TraditionalDeferredLightShadingDrawInfo
{
public:
	Mat4 m_viewProjectionMatrix;
	Mat4 m_invViewProjectionMatrix;
	Vec4 m_cameraPosWSpace;
	UVec4 m_viewport;
	F32 m_effectiveShadowDistance = -1.0f; // TODO rm
	Mat4 m_dirLightMatrix; // TODO rm

	BufferOffsetRange m_visibleLightsBuffer;

	Bool m_computeSpecular = false;

	// Render targets
	Array<RenderTargetHandle, kGBufferColorRenderTargetCount - 1> m_gbufferRenderTargets;
	Array<TextureSubresourceInfo, kGBufferColorRenderTargetCount - 1> m_gbufferRenderTargetSubresourceInfos;

	RenderTargetHandle m_gbufferDepthRenderTarget;
	TextureSubresourceInfo m_gbufferDepthRenderTargetSubresourceInfo = {DepthStencilAspectBit::kDepth};

	RenderTargetHandle m_directionalLightShadowmapRenderTarget;
	TextureSubresourceInfo m_directionalLightShadowmapRenderTargetSubresourceInfo = {DepthStencilAspectBit::kDepth};

	RenderPassWorkContext* m_renderpassContext = nullptr;
};

/// Helper for drawing using traditional deferred shading.
class TraditionalDeferredLightShading : public RendererObject
{
public:
	Error init();

	/// Run the light shading. It will iterate over the lights and draw them. It doesn't bind anything related to GBuffer or the output buffer.
	void drawLights(TraditionalDeferredLightShadingDrawInfo& info);

private:
	ShaderProgramResourcePtr m_lightProg;
	Array<ShaderProgramPtr, 2> m_lightGrProg;

	ShaderProgramResourcePtr m_skyboxProg;
	Array<ShaderProgramPtr, 2> m_skyboxGrProgs;

	SamplerPtr m_shadowSampler;
};
/// @}
} // end namespace anki
