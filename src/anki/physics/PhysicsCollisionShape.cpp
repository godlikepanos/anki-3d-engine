// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsSphere::PhysicsSphere(PhysicsWorld* world, F32 radius)
	: PhysicsCollisionShape(world, ShapeType::SPHERE)
{
	m_sphere.init(radius);
	m_sphere->setMargin(getWorld().getCollisionMargin());
	m_sphere->setUserPointer(static_cast<PhysicsObject*>(this));
}

PhysicsSphere::~PhysicsSphere()
{
	m_sphere.destroy();
}

PhysicsBox::PhysicsBox(PhysicsWorld* world, const Vec3& extend)
	: PhysicsCollisionShape(world, ShapeType::BOX)
{
	m_box.init(toBt(extend));
	m_box->setMargin(getWorld().getCollisionMargin());
	m_box->setUserPointer(static_cast<PhysicsObject*>(this));
}

PhysicsBox::~PhysicsBox()
{
	m_box.destroy();
}

PhysicsTriangleSoup::PhysicsTriangleSoup(
	PhysicsWorld* world, ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices, Bool convex)
	: PhysicsCollisionShape(world, ShapeType::TRI_MESH)
{
	if(!convex)
	{
		ANKI_ASSERT((indices.getSize() % 3) == 0);

		m_mesh.init();

		for(U i = 0; i < indices.getSize(); i += 3)
		{
			m_mesh->addTriangle(
				toBt(positions[indices[i]]), toBt(positions[indices[i + 1]]), toBt(positions[indices[i + 2]]));
		}

		// Create the dynamic shape
		m_triMesh.m_dynamic.init(m_mesh.get());
		m_triMesh.m_dynamic->setMargin(getWorld().getCollisionMargin());
		m_triMesh.m_dynamic->updateBound();
		m_triMesh.m_dynamic->setUserPointer(static_cast<PhysicsObject*>(this));

		// And the static one
		m_triMesh.m_static.init(m_mesh.get(), true);
		m_triMesh.m_static->setMargin(getWorld().getCollisionMargin());
		m_triMesh.m_static->setUserPointer(static_cast<PhysicsObject*>(this));
	}
	else
	{
		m_type = ShapeType::CONVEX; // Fake the type

		m_convex.init(&positions[0][0], positions.getSize(), sizeof(Vec3));
		m_convex->setMargin(getWorld().getCollisionMargin());
		m_convex->setUserPointer(static_cast<PhysicsObject*>(this));
	}
}

PhysicsTriangleSoup::~PhysicsTriangleSoup()
{
	if(m_type == ShapeType::TRI_MESH)
	{
		m_triMesh.m_dynamic.destroy();
		m_triMesh.m_static.destroy();
		m_mesh.destroy();
	}
	else
	{
		ANKI_ASSERT(m_type == ShapeType::CONVEX);
		m_convex.destroy();
	}
}

} // end namespace anki
