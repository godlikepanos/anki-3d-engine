// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_COLLISION_SHAPE_H
#define ANKI_PHYSICS_PHYSICS_COLLISION_SHAPE_H

#include <anki/physics/PhysicsObject.h>

namespace anki {

/// @addtogroup physics
/// @{

/// The base of all collision shapes.
class PhysicsCollisionShape: public PhysicsObject
{
public:
	/// Type of supported physics collision shapes.
	enum class ShapeType: U8
	{
		NONE,
		SPHERE,
		BOX,
		STATIC_TRIANGLE_SOUP
	};

	/// Standard initializer for all collision shapes.
	struct Initializer
	{
		// Empty for now
	};

	PhysicsCollisionShape(PhysicsWorld* world)
	:	PhysicsObject(Type::COLLISION_SHAPE, world)
	{}

	~PhysicsCollisionShape();

	static Bool classof(const PhysicsObject& c)
	{
		return c.getType() == Type::COLLISION_SHAPE;
	}

	/// @privatesection
	/// @{
	NewtonCollision* _getNewtonShape() const
	{
		ANKI_ASSERT(m_shape);
		return m_shape;
	}
	/// @}

protected:
	NewtonCollision* m_shape = nullptr;
	void* m_sceneCollisionProxy = nullptr;
	static I32 m_gid;
};

/// Sphere collision shape.
class PhysicsSphere final: public PhysicsCollisionShape
{
public:
	PhysicsSphere(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsSphere()
	{}

	ANKI_USE_RESULT Error create(Initializer& init, F32 radius);
};

/// Box collision shape.
class PhysicsBox final: public PhysicsCollisionShape
{
public:
	PhysicsBox(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsBox()
	{}

	ANKI_USE_RESULT Error create(Initializer& init, const Vec3& extend);
};

/// Convex hull collision shape.
class PhysicsConvexHull final: public PhysicsCollisionShape
{
public:
	PhysicsConvexHull(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsConvexHull()
	{}

	ANKI_USE_RESULT Error create(Initializer& init, 
		const Vec3* positions, U32 positionsCount, U32 positionsStride);
};

/// Static triangle mesh shape.
class PhysicsTriangleSoup final: public PhysicsCollisionShape
{
public:
	PhysicsTriangleSoup(PhysicsWorld* world)
	:	PhysicsCollisionShape(world)
	{}

	~PhysicsTriangleSoup()
	{}

	ANKI_USE_RESULT Error create(Initializer& init,
		const Vec3* positions, U32 positionsStride, 
		const U16* indices, U32 indicesCount);
};
/// @}

} // end namespace anki

#endif

