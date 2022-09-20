// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/ClassWrapper.h>

namespace anki {

/// @addtogroup physics
/// @{

/// The base of all collision shapes.
class PhysicsCollisionShape : public PhysicsObject
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kCollisionShape)

public:
	ANKI_INTERNAL const btCollisionShape* getBtShape(Bool forDynamicBodies = false) const
	{
		return getBtShapeInternal(forDynamicBodies);
	}

	ANKI_INTERNAL btCollisionShape* getBtShape(Bool forDynamicBodies = false)
	{
		return const_cast<btCollisionShape*>(getBtShapeInternal(forDynamicBodies));
	}

protected:
	enum class ShapeType : U8
	{
		kBox,
		kSphere,
		kConvex,
		kTrimesh
	};

	class TriMesh
	{
	public:
		ClassWrapper<btGImpactMeshShape> m_dynamic;
		ClassWrapper<btBvhTriangleMeshShape> m_static;
	};

	// All shapes
	union
	{
		ClassWrapper<btBoxShape> m_box;
		ClassWrapper<btSphereShape> m_sphere;
		ClassWrapper<btConvexHullShape> m_convex;
		TriMesh m_triMesh;
	};

	ShapeType m_type;

	PhysicsCollisionShape(PhysicsWorld* world, ShapeType type)
		: PhysicsObject(kClassType, world)
		, m_type(type)
	{
	}

	const btCollisionShape* getBtShapeInternal(Bool forDynamicBodies) const
	{
		switch(m_type)
		{
		case ShapeType::kBox:
			return m_box.get();
		case ShapeType::kSphere:
			return m_sphere.get();
		case ShapeType::kConvex:
			return m_convex.get();
		case ShapeType::kTrimesh:
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

	void registerToWorld() override
	{
	}

	void unregisterFromWorld() override
	{
	}
};

/// Sphere collision shape.
class PhysicsSphere final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kCollisionShape)

private:
	PhysicsSphere(PhysicsWorld* world, F32 radius);

	~PhysicsSphere();
};

/// Box collision shape.
class PhysicsBox final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kCollisionShape)

private:
	PhysicsBox(PhysicsWorld* world, const Vec3& extend);

	~PhysicsBox();
};

/// Convex hull collision shape.
class PhysicsConvexHull final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kCollisionShape)

private:
	PhysicsConvexHull(PhysicsWorld* world, const Vec3* positions, U32 positionsCount, U32 positionsStride);

	~PhysicsConvexHull();
};

/// Static triangle mesh shape.
class PhysicsTriangleSoup final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kCollisionShape)

private:
	ClassWrapper<btTriangleMesh> m_mesh;

	PhysicsTriangleSoup(PhysicsWorld* world, ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices,
						Bool convex = false);

	~PhysicsTriangleSoup();
};
/// @}

} // end namespace anki
