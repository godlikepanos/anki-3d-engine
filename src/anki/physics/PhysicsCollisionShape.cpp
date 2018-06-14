// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>

namespace anki
{

PhysicsCollisionShape::~PhysicsCollisionShape()
{
	getAllocator().deleteInstance(m_shape);
}

PhysicsSphere::PhysicsSphere(PhysicsWorld* world, F32 radius)
	: PhysicsCollisionShape(world)
{
	m_shape = getAllocator().newInstance<btSphereShape>(radius);
}

PhysicsBox::PhysicsBox(PhysicsWorld* world, const Vec3& extend)
	: PhysicsCollisionShape(world)
{
	m_shape = getAllocator().newInstance<btBoxShape>(toBt(extend));
}

PhysicsTriangleSoup::PhysicsTriangleSoup(
	PhysicsWorld* world, ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices)
	: PhysicsCollisionShape(world)
{
	ANKI_ASSERT((indices.getSize() % 3) == 0);

	m_mesh = getAllocator().newInstance<btTriangleMesh>();
	for(U i = 0; i < indices.getSize(); i += 3)
	{
		m_mesh->addTriangle(
			toBt(positions[indices[i]]), toBt(positions[indices[i + 1]]), toBt(positions[indices[i + 2]]));
	}

	m_shape = getAllocator().newInstance<btGImpactMeshShape>(m_mesh);
}

PhysicsTriangleSoup::~PhysicsTriangleSoup()
{
	getAllocator().deleteInstance(m_mesh);
}

} // end namespace anki
