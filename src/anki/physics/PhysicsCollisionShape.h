// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// The base of all collision shapes.
class PhysicsCollisionShape : public PhysicsObject
{
public:
	static const PhysicsObjectType CLASS_TYPE = PhysicsObjectType::COLLISION_SHAPE;

anki_internal:
	const btCollisionShape* getBtShape(Bool forDynamicBodies = false) const
	{
		return getBtShapeInternal(forDynamicBodies);
	}

	btCollisionShape* getBtShape(Bool forDynamicBodies = false)
	{
		return const_cast<btCollisionShape*>(getBtShapeInternal(forDynamicBodies));
	}

protected:
	enum class ShapeType : U8
	{
		BOX,
		SPHERE,
		CONVEX,
		TRI_MESH
	};

	class TriMesh
	{
	public:
		BtClassWrapper<btGImpactMeshShape> m_dynamic;
		BtClassWrapper<btBvhTriangleMeshShape> m_static;
	};

	// All shapes
	union
	{
		BtClassWrapper<btBoxShape> m_box;
		BtClassWrapper<btSphereShape> m_sphere;
		BtClassWrapper<btConvexHullShape> m_convex;
		TriMesh m_triMesh;
	};

	ShapeType m_type;

	PhysicsCollisionShape(PhysicsWorld* world, ShapeType type)
		: PhysicsObject(CLASS_TYPE, world)
		, m_type(type)
	{
	}

	const btCollisionShape* getBtShapeInternal(Bool forDynamicBodies) const
	{
		switch(m_type)
		{
		case ShapeType::BOX:
			return m_box.get();
		case ShapeType::SPHERE:
			return m_sphere.get();
		case ShapeType::CONVEX:
			return m_convex.get();
		case ShapeType::TRI_MESH:
			if(forDynamicBodies)
			{
				return m_triMesh.m_dynamic.get();
			}
			else
			{
				return m_triMesh.m_static.get();
			}
		default:
			ANKI_ASSERT(0);
			return nullptr;
		}
	}
};

/// Sphere collision shape.
class PhysicsSphere final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsSphere(PhysicsWorld* world, F32 radius);

	~PhysicsSphere();
};

/// Box collision shape.
class PhysicsBox final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsBox(PhysicsWorld* world, const Vec3& extend);

	~PhysicsBox();
};

/// Convex hull collision shape.
class PhysicsConvexHull final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsConvexHull(PhysicsWorld* world, const Vec3* positions, U32 positionsCount, U32 positionsStride);

	~PhysicsConvexHull();
};

/// Static triangle mesh shape.
class PhysicsTriangleSoup final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	BtClassWrapper<btTriangleMesh> m_mesh;

	PhysicsTriangleSoup(
		PhysicsWorld* world, ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices, Bool convex = false);

	~PhysicsTriangleSoup();
};
/// @}

} // end namespace anki
