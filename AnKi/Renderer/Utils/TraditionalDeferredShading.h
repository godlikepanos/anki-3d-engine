// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/RenderQueue.h>

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
	Vec2 m_gbufferTexCoordsScale;
	Vec2 m_gbufferTexCoordsBias;
	Vec2 m_lightbufferTexCoordsScale;
	Vec2 m_lightbufferTexCoordsBias;
	F32 m_cameraNear;
	F32 m_cameraFar;
	const DirectionalLightQueueElement* m_directionalLight = nullptr;
	ConstWeakArray<PointLightQueueElement> m_pointLights;
	ConstWeakArray<SpotLightQueueElement> m_spotLights;
	const SkyboxQueueElement* m_skybox = nullptr;
	CommandBuffer* m_commandBuffer = nullptr;
	Bool m_computeSpecular = false;

	// Render targets
	Array<RenderTargetHandle, kGBufferColorRenderTargetCount - 1> m_gbufferRenderTargets;
	RenderTargetHandle m_gbufferDepthRenderTarget;
	RenderTargetHandle m_directionalLightShadowmapRenderTarget;

	RenderPassWorkContext* m_renderpassContext = nullptr;
};

/// Helper for drawing using traditional deferred shading.
class TraditionalDeferredLightShading : public RendererObject
{
public:
	Error init();

	/// Run the light shading. It will iterate over the lights and draw them. It doesn't bind anything related to
	/// GBuffer or the output buffer.
	void drawLights(TraditionalDeferredLightShadingDrawInfo& info);

private:
	enum class ProxyType
	{
		kProxySphere,
		kProxyCone
	};

	ShaderProgramResourcePtr m_lightProg;
	Array<ShaderProgramPtr, 2> m_plightGrProg;
	Array<ShaderProgramPtr, 2> m_slightGrProg;
	Array<ShaderProgramPtr, 2> m_dirLightGrProg;

	ShaderProgramResourcePtr m_skyboxProg;
	Array<ShaderProgramPtr, 2> m_skyboxGrProgs;

	/// @name Meshes of light volumes.
	/// @{
	BufferPtr m_proxyVolumesBuffer;
	/// @}

	SamplerPtr m_shadowSampler;

	void bindVertexIndexBuffers(ProxyType proxyType, CommandBuffer& cmdb, U32& indexCount) const;

	void createProxyMeshes();
};
/// @}
} // end namespace anki