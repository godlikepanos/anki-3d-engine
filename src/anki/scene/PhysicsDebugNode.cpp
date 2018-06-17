// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/PhysicsDebugNode.h>
#include <anki/resource/ResourceManager.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

/// Render component implementation.
class PhysicsDebugNode::MyRenderComponent : public RenderComponent
{
public:
	ShaderProgramResourcePtr m_prog;

	MyRenderComponent(SceneNode* node)
		: RenderComponent(node)
	{
		m_castsShadow = false;
		m_isForwardShading = false;
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const override
	{
		// TODO
	}
};

Error PhysicsDebugNode::init()
{
	MyRenderComponent* rcomp = newComponent<MyRenderComponent>(this);
	ANKI_CHECK(getResourceManager().loadResource("shaders/SceneDebug.glslp", rcomp->m_prog));

	ObbSpatialComponent* scomp = newComponent<ObbSpatialComponent>(this);
	Vec3 center = (getSceneGraph().getSceneMax() + getSceneGraph().getSceneMin()) / 2.0f;
	scomp->m_obb.setCenter(center.xyz0());
	scomp->m_obb.setExtend((getSceneGraph().getSceneMax() - center).xyz0());
	scomp->m_obb.setRotation(Mat3x4::getIdentity());

	return Error::NONE;
}

} // end namespace anki