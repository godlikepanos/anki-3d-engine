// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/PhysicsDebugNode.h>
#include <anki/resource/ResourceManager.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/DebugDrawer.h>

namespace anki
{

/// Render component implementation.
class PhysicsDebugNode::MyRenderComponent : public RenderComponent
{
public:
	SceneNode* m_node;
	DebugDrawer m_dbgDrawer;
	PhysicsDebugDrawer m_physDbgDrawer;

	MyRenderComponent(SceneNode* node)
		: m_node(node)
		, m_physDbgDrawer(&m_dbgDrawer)
	{
		ANKI_ASSERT(node);
		setFlags(RenderComponentFlag::NONE);
		initRaster([](RenderQueueDrawContext& ctx,
					  ConstWeakArray<void*> userData) { static_cast<MyRenderComponent*>(userData[0])->draw(ctx); },
				   this, 0);
	}

	ANKI_USE_RESULT Error init()
	{
		return m_dbgDrawer.init(&m_node->getSceneGraph().getResourceManager());
	}

	/// Draw the world.
	void draw(RenderQueueDrawContext& ctx)
	{
		if(ctx.m_debugDraw)
		{
			m_dbgDrawer.prepareFrame(&ctx);
			m_dbgDrawer.setViewProjectionMatrix(ctx.m_viewProjectionMatrix);
			m_dbgDrawer.setModelMatrix(Mat4::getIdentity());
			m_physDbgDrawer.drawWorld(m_node->getSceneGraph().getPhysicsWorld());
			m_dbgDrawer.finishFrame();
		}
	}
};

PhysicsDebugNode::~PhysicsDebugNode()
{
}

Error PhysicsDebugNode::init()
{
	MyRenderComponent* rcomp = newComponent<MyRenderComponent>(this);
	ANKI_CHECK(rcomp->init());

	ObbSpatialComponent* scomp = newComponent<ObbSpatialComponent>(this);
	const Vec3 center = (getSceneGraph().getSceneMax() + getSceneGraph().getSceneMin()) / 2.0f;
	scomp->m_obb.setCenter(center.xyz0());
	scomp->m_obb.setExtend((getSceneGraph().getSceneMax() - center).xyz0());
	scomp->m_obb.setRotation(Mat3x4::getIdentity());
	scomp->setSpatialOrigin(Vec4(0.0f));
	scomp->setUpdateOctreeBounds(false); // Don't mess with the bounds

	return Error::NONE;
}

} // end namespace anki
