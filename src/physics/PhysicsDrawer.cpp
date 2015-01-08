// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsDrawer.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
void PhysicsDrawer::drawWorld(const PhysicsWorld& world)
{
	NewtonWorld* nworld = world._getNewtonWorld();
	for(NewtonBody* body = NewtonWorldGetFirstBody(nworld); 
		body != nullptr; 
		body = NewtonWorldGetNextBody(nworld, body)) 
	{
		if(m_drawAabbs)
		{
			drawAabb(body);
		}
	}
}

//==============================================================================
void PhysicsDrawer::drawAabb(const NewtonBody* body)
{
	Vec4 p0; 
	Vec4 p1; 
	Mat4 matrix;

	NewtonCollision* collision = NewtonBodyGetCollision(body);
	NewtonBodyGetMatrix(body, &matrix[0]);
	NewtonCollisionCalculateAABB(collision, &matrix[0], &p0[0], &p1[0]);

	Vec3 lines[] = {
		Vec3(p0.x(), p0.y(), p0.z()),
		Vec3(p1.x(), p0.y(), p0.z()),
		Vec3(p0.x(), p1.y(), p0.z()),
		Vec3(p1.x(), p1.y(), p0.z()),
		Vec3(p0.x(), p1.y(), p1.z()),
		Vec3(p1.x(), p1.y(), p1.z()),
		Vec3(p0.x(), p0.y(), p1.z()),
		Vec3(p1.x(), p0.y(), p1.z()),
		Vec3(p0.x(), p0.y(), p0.z()),
		Vec3(p0.x(), p1.y(), p0.z()),
		Vec3(p1.x(), p0.y(), p0.z()),
		Vec3(p1.x(), p1.y(), p0.z()),
		Vec3(p0.x(), p0.y(), p1.z()),
		Vec3(p0.x(), p1.y(), p1.z()),
		Vec3(p1.x(), p0.y(), p1.z()),
		Vec3(p1.x(), p1.y(), p1.z()),
		Vec3(p0.x(), p0.y(), p0.z()),
		Vec3(p0.x(), p0.y(), p1.z()),
		Vec3(p1.x(), p0.y(), p0.z()),
		Vec3(p1.x(), p0.y(), p1.z()),
		Vec3(p0.x(), p1.y(), p0.z()),
		Vec3(p0.x(), p1.y(), p1.z()),
		Vec3(p1.x(), p1.y(), p0.z()),
		Vec3(p1.x(), p1.y(), p1.z())};

	drawLines(lines, sizeof(lines) / sizeof(Vec3), Vec4(0.0, 0.0, 1.0, 0.5));
}

} // end namespace anki

