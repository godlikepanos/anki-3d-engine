// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/CollisionShape.h>

namespace anki {

/// @addtogroup collision
/// @{

/// Abstact class for convex collision shapes
class ConvexShape: public CollisionShape
{
public:
	using Base = CollisionShape;

	ConvexShape(Type cid)
	:	Base(cid)
	{}

	ConvexShape(const ConvexShape& b)
	:	Base(b)
	{
		operator=(b);
	}

	/// Copy.
	ConvexShape& operator=(const ConvexShape& b)
	{
		Base::operator=(b);
		return *this;
	}

	/// Get a support vector for the GJK algorithm
	virtual Vec4 computeSupport(const Vec4& dir) const = 0;

	static Bool classof(const CollisionShape& c)
	{
		return c.getType() >= Type::AABB && c.getType() <= Type::LAST_CONVEX;
	}
};
/// @}

} // end namespace anki

