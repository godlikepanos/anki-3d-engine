// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics2/PhysicsWorld.h>
#include <AnKi/Physics2/PhysicsCollisionShape.h>

namespace anki {

void Physics2CollisionShapePtrDeleter::operator()(Physics2CollisionShape* ptr)
{
	Physics2World::getSingleton().destroyCollisionShape(ptr);
}

Physics2CollisionShapePtr Physics2World::newSphereCollisionShape(F32 radius)
{
	auto it = m_collisionShapes.emplace(Physics2CollisionShape::ShapeType::kSphere);
	it->m_sphere.construct(radius);
	it->m_sphere->SetEmbedded();
	it->m_arrayIndex = it.getArrayIndex();
	return Physics2CollisionShapePtr(&(*it));
}

Physics2CollisionShapePtr Physics2World::newBoxCollisionShape(Vec3 extend)
{
	auto it = m_collisionShapes.emplace(Physics2CollisionShape::ShapeType::kBox);
	it->m_box.construct(toJPH(extend));
	it->m_box->SetEmbedded();
	it->m_arrayIndex = it.getArrayIndex();
	return Physics2CollisionShapePtr(&(*it));
}

void Physics2World::destroyCollisionShape(Physics2CollisionShape* ptr)
{
	m_collisionShapes.erase(ptr->m_arrayIndex);
}

} // end namespace anki
