// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

	Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class FeedbackComponent;
	class RenderProxy;

	Aabb m_aabbLocal;
	DynamicArray<RenderProxy> m_renderProxies; ///< The size matches the number of render components.

	Bool m_deferredRenderComponentUpdate = false;

	void feedbackUpdate();

	void draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData, U32 modelPatchIdx) const;

	void setupRayTracingInstanceQueueElement(U32 lod, U32 modelPatchIdx, RayTracingInstanceQueueElement& el) const;

	void initRenderComponents();
};
/// @}

} // end namespace anki
