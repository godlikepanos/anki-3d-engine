#include "anki/collision/Ray.h"
#include "anki/collision/Plane.h"

namespace anki {

//==============================================================================
Ray Ray::getTransformed(const Transform& transform) const
{
	Ray out;
	out.origin = origin.getTransformed(transform);
	out.dir = transform.getRotation() * dir;
	return out;
}

//==============================================================================
float Ray::testPlane(const Plane& p) const
{
	const Ray& r = *this;
	float dist = p.test(r.getOrigin());
	float cos_ = p.getNormal().dot(r.getDirection());

	if(cos_ > 0.0) // the ray points to the same half-space as the plane
	{
		if(dist < 0.0) // the ray's origin is behind the plane
		{
			return 0.0;
		}
		else
		{
			return dist;
		}
	}
	else
	{
		if(dist > 0.0)
		{
			return 0.0;
		}
		else
		{
			return dist;
		}
	}
}

//==============================================================================
void Ray::getAabb(Aabb&) const
{
	ANKI_ASSERT(0 && "Can't do that");
}

} // end namespace
