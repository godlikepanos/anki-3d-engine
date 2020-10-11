// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

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
	DirectionalLightQueueElement* m_directionalLight ANKI_DEBUG_CODE(= numberToPtr<DirectionalLightQueueElement*>(1));
	ConstWeakArray<PointLightQueueElement> m_pointLights;
	ConstWeakArray<SpotLightQueueElement> m_spotLights;
	CommandBufferPtr m_commandBuffer;
	Bool m_computeSpecular = false;
};

/// Helper for drawing using traditional deferred shading.
class TraditionalDeferredLightShading : public RendererObject
{
public:
	TraditionalDeferredLightShading(Renderer* r);

	~TraditionalDeferredLightShading();

	ANKI_USE_RESULT Error init();

	/// Run the light shading. It will iterate over the lights and draw them. It doesn't bind anything related to
	/// GBuffer or the output buffer.
	void drawLights(TraditionalDeferredLightShadingDrawInfo& info);

private:
	ShaderProgramResourcePtr m_lightProg;
	Array<ShaderProgramPtr, 2> m_plightGrProg;
	Array<ShaderProgramPtr, 2> m_slightGrProg;
	Array<ShaderProgramPtr, 2> m_dirLightGrProg;

	/// @name Meshes of light volumes.
	/// @{
	MeshResourcePtr m_plightMesh;
	MeshResourcePtr m_slightMesh;
	/// @}

	static void bindVertexIndexBuffers(MeshResourcePtr& mesh, CommandBufferPtr& cmdb, U32& indexCount);
};
/// @}
} // end namespace anki
