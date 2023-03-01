// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsDrawer.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

void PhysicsDrawer::drawWorld(const PhysicsWorld& world)
{
	btDynamicsWorld& btWorld = const_cast<btDynamicsWorld&>(world.getBtWorld());

	btWorld.setDebugDrawer(&m_debugDraw);
	btWorld.debugDrawWorld();
}

} // end namespace anki
