// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Tests.h"
#include "anki/collision/Aabb.h"
#include "anki/collision/Sphere.h"
#include "anki/collision/CompoundShape.h"
#include "anki/collision/GjkEpa.h"
#include "anki/util/Rtti.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
static Bool gjk(const CollisionShape& a, const CollisionShape& b)
{
	Gjk gjk;
	return gjk.intersect(
		dcast<const ConvexShape&>(a), dcast<const ConvexShape&>(b));
}

//==============================================================================
static Bool test(const Aabb& a, const Aabb& b)
{
	// if separated in x direction
	if(a.getMin().x() > b.getMax().x() || b.getMin().x() > a.getMax().x())
	{
		return false;
	}

	// if separated in y direction
	if(a.getMin().y() > b.getMax().y() || b.getMin().y() > a.getMax().y())
	{
		return false;
	}

	// if separated in z direction
	if(a.getMin().z() > b.getMax().z() || b.getMin().z() > a.getMax().z())
	{
		return false;
	}

	// no separation, must be intersecting
	return true;
}

//==============================================================================
static Bool test(const Sphere& a, const Sphere& b)
{
	F32 tmp = a.getRadius() + b.getRadius();
	return (a.getCenter() - b.getCenter()).getLengthSquared() <= tmp * tmp;
}

//==============================================================================
// Matrix                                                                      =
//==============================================================================

template<typename A, typename B>
Bool t(const CollisionShape& a, const CollisionShape& b)
{
	return test(dcast<const A&>(a), dcast<const B&>(b));
}

using Callback = Bool (*)(const CollisionShape& a, const CollisionShape& b);

static const U COUNT = U(CollisionShape::Type::COUNT);

static const Callback matrix[COUNT][COUNT] = {
	// AABB         Comp     LS       OBB      PL       S
	{t<Aabb, Aabb>, nullptr, nullptr, gjk,     nullptr, nullptr          },  // AABB
	{nullptr,       nullptr, nullptr, nullptr, nullptr, nullptr          },  // Comp
	{nullptr,       nullptr, nullptr, nullptr, nullptr, nullptr          },  // LS
	{gjk,           nullptr, nullptr, gjk,     nullptr, nullptr          },  // OBB
	{nullptr,       nullptr, nullptr, nullptr, nullptr, nullptr          },  // PL
	{nullptr,       nullptr, nullptr, gjk,     nullptr, t<Sphere, Sphere>}}; // S

} // end namespace anki

