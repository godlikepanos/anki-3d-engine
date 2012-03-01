#include "anki/collision/ToAabb.h"
#include "anki/collision/Collision.h"
#include "anki/util/Assert.h"


namespace anki {


//=============================================================================
void ToAbb::visit(LineSegment& ls)
{
	Vec3 min = pls.getOrigin();
	Vec3 max = pls.getOrigin() + ls.getDirection();

	for(uint i = 0; i < 3; ++i)
	{
		if(max[i] < min[i])
		{
			float tmp = max[i];
			max[i] = min[i];
			min[i] = tmp;
		}
	}

	aabb.setMax(max);
	aabb.setMin(min);
}


//=============================================================================
void ToAbb::visit(Obb& obb)
{
	// XXX
}


//=============================================================================
void ToAbb::visit(Frustum&)
{
	/// XXX
}


//=============================================================================
void ToAbb::visit(Plane&)
{
	ANKI_ASSERT(0 && "Can't do that");
}


//=============================================================================
void ToAbb::visit(Ray&)
{
	ANKI_ASSERT(0 && "Can't do that");
}


//=============================================================================
void ToAbb::visit(Sphere& s)
{
	Vec3 r = Vec3(s.getRadius());
	aabb.setMin(s.getCenter() - r);
	aabb.setMax(s.getCenter() + r);
}


//=============================================================================
void ToAbb::visit(Aabb& b)
{
	aabb = b;
}


} // end namespace anki
