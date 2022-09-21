// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/PhysicsDebugNode.h>
#include <AnKi/Scene/Components/RenderComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

PhysicsDebugNode::PhysicsDebugNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
	, m_physDbgDrawer(&scene->getDebugDrawer())
{
	RenderComponent* rcomp = newComponent<RenderComponent>();
	rcomp->setFlags(RenderComponentFlag::kNone);
	rcomp->initRaster(
		[](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			static_cast<PhysicsDebugNode*>(userData[0])->draw(ctx);
		},
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
