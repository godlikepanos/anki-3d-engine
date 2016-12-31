// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsDrawer.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

struct CallbackData
{
	const NewtonBody* m_body;
	PhysicsDrawer* m_drawer;
};

void PhysicsDrawer::drawWorld(const PhysicsWorld& world)
{
	NewtonWorld* nworld = world.getNewtonWorld();
	for(NewtonBody* body = NewtonWorldGetFirstBody(nworld); body != nullptr;
		body = NewtonWorldGetNextBody(nworld, body))
	{
		if(m_drawAabbs)
		{
			drawAabb(body);
		}

		if(m_drawCollision)
		{
			drawCollision(body);
		}
	}
}

void PhysicsDrawer::drawAabb(const NewtonBody* body)
{
	Vec4 p0;
	Vec4 p1;
	Mat4 matrix;

	NewtonCollision* collision = NewtonBodyGetCollision(body);
	NewtonBodyGetMatrix(body, &matrix[0]);
	NewtonCollisionCalculateAABB(collision, &matrix[0], &p0[0], &p1[0]);

	Vec3 lines[] = {Vec3(p0.x(), p0.y(), p0.z()),
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

	const U32 linesCount = sizeof(lines) / sizeof(Vec3) / 2;
	drawLines(lines, linesCount, Vec4(0.0, 0.0, 1.0, 0.5));
}

void PhysicsDrawer::drawCollision(const NewtonBody* body)
{
	Mat4 matrix;
	NewtonBodyGetMatrix(body, &matrix[0]);

	CallbackData data;
	data.m_body = body;
	data.m_drawer = this;

	NewtonCollisionForEachPolygonDo(
		NewtonBodyGetCollision(body), &matrix[0], drawGeometryCallback, static_cast<void*>(&data));
}

void PhysicsDrawer::drawGeometryCallback(void* userData, int vertexCount, const dFloat* const faceVertec, int id)
{
	CallbackData* data = static_cast<CallbackData*>(userData);
	const NewtonBody* body = data->m_body;

	Vec4 color(1.0);
	if(NewtonBodyGetType(body) == NEWTON_DYNAMIC_BODY)
	{
		I sleepState = NewtonBodyGetSleepState(body);
		if(sleepState == 1)
		{
			// Sleeping
			color = Vec4(0.3, 0.3, 0.3, 1.0);
		}
	}

	U i = vertexCount - 1;

	Array<Vec3, 2> points;
	points[0] = Vec3(faceVertec[i * 3 + 0], faceVertec[i * 3 + 1], faceVertec[i * 3 + 2]);

	for(I i = 0; i < vertexCount; i++)
	{
		points[1] = Vec3(faceVertec[i * 3 + 0], faceVertec[i * 3 + 1], faceVertec[i * 3 + 2]);

		data->m_drawer->drawLines(&points[0], 1, color);

		points[0] = points[1];
	}
}

} // end namespace anki
