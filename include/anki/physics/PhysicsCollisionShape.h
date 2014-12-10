// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_PHYSICS_COLLISION_SHAPE_H
#define ANKI_PHYSICS_PHYSICS_COLLISION_SHAPE_H

#include "anki/physics/Common.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Standard initializer for all collision shapes.
struct PhysicsCollisionShapeInitializer
{
	PhysicsWorld* m_world = nullptr;
};

/// The base of all collision shapes.
class PhysicsCollisionShape
{
public:
	using Initializer = PhysicsCollisionShapeInitializer;

	PhysicsCollisionShape();
	~PhysicsCollisionShape();

	/// Create sphere.
	ANKI_USE_RESULT Error createSphere(Initializer& init, F32 radius);

	/// Create box.
	ANKI_USE_RESULT Error createBox(Initializer& init, const Vec3& extend);

	/// Create convex hull.
	ANKI_USE_RESULT Error createConvexHull(Initializer& init, 
		const Vec3* positions, U32 positionsCount, U32 positionsStride);

	/// Create face soup.
	ANKI_USE_RESULT Error createStaticTriangleSoup(Initializer& init,
		const Vec3* positions, U32 positionsStride, U16* indices);

	NewtonCollision* _getNewtonShape()
	{
		ANKI_ASSERT(m_shape);
		return m_shape;
	}

private:
	NewtonCollision* m_shape = nullptr;
	static I32 id;
};
/// @}

} // end namespace anki

#endif

