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
anki_internal:
	virtual btCollisionShape* getBtShape(Bool forDynamicBodies = false) const
	{
		ANKI_ASSERT(m_shape);
		return m_shape;
	}

protected:
	btCollisionShape* m_shape = nullptr;

	PhysicsCollisionShape(PhysicsWorld* world)
		: PhysicsObject(PhysicsObjectType::COLLISION_SHAPE, world)
	{
	}

	~PhysicsCollisionShape();
};

/// Sphere collision shape.
class PhysicsSphere final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsSphere(PhysicsWorld* world, F32 radius);
};

/// Box collision shape.
class PhysicsBox final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsBox(PhysicsWorld* world, const Vec3& extend);
};

/// Convex hull collision shape.
class PhysicsConvexHull final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	PhysicsConvexHull(PhysicsWorld* world, const Vec3* positions, U32 positionsCount, U32 positionsStride);
};

/// Static triangle mesh shape.
class PhysicsTriangleSoup final : public PhysicsCollisionShape
{
	ANKI_PHYSICS_OBJECT

private:
	btTriangleMesh* m_mesh = nullptr;
	btCollisionShape* m_staticShape = nullptr;

	PhysicsTriangleSoup(PhysicsWorld* world, ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices);

	~PhysicsTriangleSoup();

	btCollisionShape* getBtShape(Bool forDynamicBodies = false) const override;
};
/// @}

} // end namespace anki
