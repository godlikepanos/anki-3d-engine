// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_CONVEX_SHAPE_H
#define ANKI_COLLISION_CONVEX_SHAPE_H

#include "anki/collision/CollisionShape.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Abstact class for convex collision shapes
class ConvexShape: public CollisionShape
{
public:
	/// @name Constructors & destructor
	/// @{
	ConvexShape(Type cid)
		: CollisionShape(cid)
	{}
	/// @}

	/// Get a support vector for the GJK algorithm
	virtual Vec4 computeSupport(const Vec4& dir) const = 0;
};

/// @}

} // end namespace anki

#endif

