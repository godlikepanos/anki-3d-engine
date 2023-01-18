// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Forward
class RenderQueueDrawContext;
class RayTracingInstanceQueueElement;

/// @addtogroup scene
/// @{

/// The model scene node.
class ModelNode : public SceneNode
{
public:
	ModelNode(SceneGraph* scene, CString name);

	~ModelNode();

private:
	class FeedbackComponent;
	class RenderProxy;

	Aabb m_aabbLocal;

	void feedbackUpdate();
};
/// @}

} // end namespace anki
