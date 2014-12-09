// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsCollisionShape.cpp"

namespace anki {

//==============================================================================
PhysicsCollisionShape::PhysicsCollisionShape()
{}

//==============================================================================
PhysicsCollisionShape::~PhysicsCollisionShape()
{
	if(m_shape)
	{
		NewtonDestroyCollision(m_shape);
	}
}

} // end namespace anki

