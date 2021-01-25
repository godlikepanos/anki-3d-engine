// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/PhysicsDebugNode.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

PhysicsDebugNode::PhysicsDebugNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
	, m_physDbgDrawer(&scene->getDebugDrawer())
{
	RenderComponent* rcomp = newComponent<RenderComponent>();
	rcomp->setFlags(RenderComponentFlag::NONE);
	rcomp->initRaster([](RenderQueueDrawContext& ctx,
						 ConstWeakArray<void*> userData) { static_cast<PhysicsDebugNode*>(userData[0])->draw(ctx); },
					  this, 0);

	SpatialComponent* scomp = newComponent<SpatialComponent>();
	scomp->setUpdateOctreeBounds(false); // Don't mess with the bounds
	scomp->setAabbWorldSpace(Aabb(getSceneGraph().getSceneMin(), getSceneGraph().getSceneMax()));
	scomp->setSpatialOrigin(Vec3(0.0f));
}

PhysicsDebugNode::~PhysicsDebugNode()
{
}

void PhysicsDebugNode::draw(RenderQueueDrawContext& ctx)
{
	if(ctx.m_debugDraw)
	{
		m_physDbgDrawer.start(ctx.m_viewProjectionMatrix, ctx.m_commandBuffer, ctx.m_stagingGpuAllocator);
		m_physDbgDrawer.drawWorld(getSceneGraph().getPhysicsWorld());
		m_physDbgDrawer.end();
	}
}

} // end namespace anki
