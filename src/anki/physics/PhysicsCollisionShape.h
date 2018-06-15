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
	PhysicsCollisionShape(PhysicsWorld* world)
		: PhysicsObject(PhysicsObjectType::COLLISION_SHAPE, world)
	{
	}

	~PhysicsCollisionShape();

anki_internal:
	virtual btCollisionShape* getBtShape(Bool forDynamicBodies = false) const
	{
		ANKI_ASSERT(m_shape);
		return m_shape;
	}

protected:
	btCollisionShape* m_shape = nullptr;
};

/// Sphere collision shape.
class PhysicsSphere final : public PhysicsCollisionShape
{
public:
	PhysicsSphere(PhysicsWorld* world, F32 radius);
};

/// Box collision shape.
class PhysicsBox final : public PhysicsCollisionShape
{
public:
	PhysicsBox(PhysicsWorld* world, const Vec3& extend);
};

/// Convex hull collision shape.
class PhysicsConvexHull final : public PhysicsCollisionShape
{
public:
	PhysicsConvexHull(PhysicsWorld* world, const Vec3* positions, U32 positionsCount, U32 positionsStride);
};

/// Static triangle mesh shape.
class PhysicsTriangleSoup final : public PhysicsCollisionShape
{
public:
	PhysicsTriangleSoup(PhysicsWorld* world, ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices);

	~PhysicsTriangleSoup();

private:
	btTriangleMesh* m_mesh = nullptr;
	btCollisionShape* m_staticShape = nullptr;

	btCollisionShape* getBtShape(Bool forDynamicBodies = false) const override;
};
/// @}

} // end namespace anki
