// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_COLLISION_SHAPE_H
#define ANKI_PHYSICS_PHYSICS_COLLISION_SHAPE_H

#include "anki/physics/PhysicsObject.h"

namespace anki {

/// @addtogroup physics
/// @{

/// The base of all collision shapes.
class PhysicsCollisionShape: public PhysicsObject
{
public:
	/// Type of supported physics collision shapes.
	enum class Type: U8
	{
		NONE,
		SPHERE,
		BOX
	};

	/// Standard initializer for all collision shapes.
	struct Initializer
	{
		// Empty for now
	};

	PhysicsCollisionShape(PhysicsWorld* world)
	:	PhysicsObject(world)
	{}

	~PhysicsCollisionShape();

	/// @privatesection
	/// @{
	NewtonCollision* _getNewtonShape()
	{
		ANKI_ASSERT(m_shape);
		return m_shape;
	}
	/// @}

protected:
	NewtonCollision* m_shape = nullptr;
	static I32 m_gid;
};

/// Sphere collision shape.
class PhysicsSphere: public PhysicsCollisionShape
{
public:
	PhysicsSphere(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsSphere() final
	{}

	ANKI_USE_RESULT Error create(Initializer& init, F32 radius);
};

/// Box collision shape.
class PhysicsBox: public PhysicsCollisionShape
{
public:
	PhysicsBox(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsBox() final
	{}

	ANKI_USE_RESULT Error create(Initializer& init, const Vec3& extend);
};

/// Convex hull collision shape.
class PhysicsConvexHull: public PhysicsCollisionShape
{
public:
	PhysicsConvexHull(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsConvexHull() final
	{}

	ANKI_USE_RESULT Error create(Initializer& init, 
		const Vec3* positions, U32 positionsCount, U32 positionsStride);
};

/// Static triangle mesh shape.
class PhysicsTriangleSoup: public PhysicsCollisionShape
{
public:
	PhysicsTriangleSoup(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsTriangleSoup() final
	{}

	ANKI_USE_RESULT Error createStaticTriangleSoup(Initializer& init,
		const Vec3* positions, U32 positionsStride, U16* indices);
};
/// @}

} // end namespace anki

#endif

