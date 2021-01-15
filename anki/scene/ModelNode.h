// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Aabb.h>
#include <anki/util/WeakArray.h>

namespace anki
{

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

	ANKI_USE_RESULT Error init();

private:
	class FeedbackComponent;
	class RenderProxy;

	Aabb m_aabbLocal;
	DynamicArray<RenderProxy> m_renderProxies; ///< The size matches the number of render components.

	void feedbackUpdate();

	void draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData, U32 modelPatchIdx) const;

	void setupRayTracingInstanceQueueElement(U32 lod, U32 modelPatchIdx, RayTracingInstanceQueueElement& el) const;

	void initRenderComponents();
};
/// @}

} // end namespace anki
