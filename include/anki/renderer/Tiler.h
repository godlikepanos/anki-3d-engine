// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Collision.h>
#include <anki/renderer/RenderingPass.h>
#include <anki/core/Timestamp.h>

namespace anki {

class SceneNode;

/// @addtogroup renderer
/// @{

/// Tiler used for visibility tests
class Tiler: public RenderingPass
{
	friend class UpdatePlanesPerspectiveCameraTask;

anki_internal:
	Tiler(Renderer* r);
	~Tiler();

	ANKI_USE_RESULT Error init();

	/// Test against all tiles.
	Bool test(const CollisionShape& cs, const Aabb& aabb) const;

	/// Issue the GPU job
	void run(CommandBufferPtr& cmd);

	/// Prepare for visibility tests
	void prepareForVisibilityTests(const SceneNode& node);

private:
	// GPU objects
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_outBuffers;
	Array<ResourceGroupPtr, MAX_FRAMES_IN_FLIGHT> m_rcGroups;
	ShaderResourcePtr m_shader;
	PipelinePtr m_ppline;

	// Other
	DArray<Vec2> m_currentMinMax;
	Mat4 m_viewProjMat;
	F32 m_near;
	Plane m_nearPlaneWspace;

	ANKI_USE_RESULT Error initInternal();
};
/// @}

} // end namespace anki

