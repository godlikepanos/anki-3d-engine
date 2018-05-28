// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Helper for drawing using traditional deferred shading.
class TraditionalDeferredLightShading : public RendererObject
{
public:
	TraditionalDeferredLightShading(Renderer* r);

	~TraditionalDeferredLightShading();

	ANKI_USE_RESULT Error init();

	/// Run the light shading. It will iterate over the lights and draw them. It doesn't bind anything related to
	/// GBuffer or the output buffer.
	void drawLights(const Mat4& vpMat,
		const Mat4& invViewProjMat,
		const Vec4& cameraPosWSpace,
		const UVec4& viewport,
		const Vec2& gbufferTexCoordsMin,
		const Vec2& gbufferTexCoordsMax,
		ConstWeakArray<PointLightQueueElement> plights,
		ConstWeakArray<SpotLightQueueElement> slights,
		CommandBufferPtr& cmdb);

private:
	ShaderProgramResourcePtr m_lightProg;
	ShaderProgramPtr m_plightGrProg;
	ShaderProgramPtr m_slightGrProg;

	/// @name Meshes of light volumes.
	/// @{
	MeshResourcePtr m_plightMesh;
	MeshResourcePtr m_slightMesh;
	/// @}

	static void bindVertexIndexBuffers(MeshResourcePtr& mesh, CommandBufferPtr& cmdb, U32& indexCount);
};
/// @}
} // end namespace anki
