// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Tests.h"
#include "anki/collision/Aabb.h"
#include "anki/collision/Sphere.h"
#include "anki/collision/CompoundShape.h"
#include "anki/collision/LineSegment.h"
#include "anki/collision/Plane.h"
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
Bool test(const Aabb& aabb, const LineSegment& ls)
{
	F32 maxS = MIN_F32;
	F32 minT = MAX_F32;

	// do tests against three sets of planes
	for(U i = 0; i < 3; ++i)
	{
		// segment is parallel to plane
		if(isZero(ls.getDirection()[i]))
		{
			// segment passes by box
			if(ls.getOrigin()[i] < aabb.getMin()[i] 
				|| ls.getOrigin()[i] > aabb.getMax()[i])
			{
				return false;
			}
		}
		else
		{
			// compute intersection parameters and sort
			F32 s = (aabb.getMin()[i] - ls.getOrigin()[i]) 
				/ ls.getDirection()[i];
			F32 t = (aabb.getMax()[i] - ls.getOrigin()[i]) 
				/ ls.getDirection()[i];
			if(s > t)
			{
				F32 temp = s;
				s = t;
				t = temp;
			}

			// adjust min and max values
			if(s > maxS)
			{
				maxS = s;
			}
			if(t < minT)
			{
				minT = t;
			}

			// check for intersection failure
			if(minT < 0.0 || maxS > 1.0 || maxS > minT)
			{
				return false;
			}
		}
	}

	// done, have intersection
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

/// Test plane.
template<typename A>
Bool tp(const CollisionShape& a, const CollisionShape& p)
{
	return dcast<const A&>(a).testPlane(dcast<const Plane&>(p));
}

using Callback = Bool(*)(const CollisionShape& a, const CollisionShape& b);

static const U COUNT = U(CollisionShape::Type::COUNT);

static const Callback matrix[COUNT][COUNT] = {
/*          AABB                  Comp     LS                    OBB      PL       S               */
/* AABB */ {t<Aabb, Aabb>,        nullptr, t<Aabb, LineSegment>, gjk,     nullptr, nullptr          },  
/* Comp */ {nullptr,              nullptr, nullptr,              nullptr, nullptr, nullptr          },  
/* LS   */ {t<Aabb, LineSegment>, nullptr, nullptr,              nullptr, nullptr, nullptr          },
/* OBB  */ {gjk,                  nullptr, nullptr,              gjk,     nullptr, nullptr          },
/* PL   */ {tp<Aabb>,             nullptr, nullptr,              nullptr, nullptr, nullptr          },
/* S    */ {nullptr,              nullptr, nullptr,              gjk,     nullptr, t<Sphere, Sphere>}};

} // end namespace anki

