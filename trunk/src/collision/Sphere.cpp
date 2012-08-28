#include "anki/collision/Sphere.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Aabb.h"

namespace anki {

//==============================================================================
F32 Sphere::testPlane(const Plane& p) const
{
	const Sphere& s = *this;
	F32 dist = p.test(s.getCenter());

	if(dist > s.getRadius())
	{
		return dist - s.getRadius();
	}
	else if(-dist > s.getRadius())
	{
		return dist + s.getRadius();
	}
	else
	{
		return 0.0;
	}
}

//==============================================================================
Sphere Sphere::getTransformed(const Transform& transform) const
{
	Sphere newSphere;

	newSphere.center = center.getTransformed(
		transform.getOrigin(),
		transform.getRotation(),
		transform.getScale());

	newSphere.radius = radius * transform.getScale();
	return newSphere;
}

//==============================================================================
Sphere Sphere::getCompoundShape(const Sphere& b) const
{
	const Sphere& a = *this;

	/// @todo test this
	/*
	Vec3 centerDiff = b.center - a.center;
	F32 radiusDiff = b.radius - a.radius;
	Vec3 radiusDiffSqr = radiusDiff * radiusDiff;
	F32 lenSqr = centerDiff.getLengthSquared();

	if(radiusDiffSqrt >= 0.0)
	{
		if(radiusDiff >= 0.0)
		{
			return b;
		}
		else
		{
			return a;
		}
	}
	else
	{
		F32 l = sqrt(lenSqr);
		F32 t = (l + b.radius - a.radius) / (2.0 * l);
		return Sphere(a.center + t * centerDiff, (l + a.radius + b.radius) /
			2.0);
	}
	*/

	Vec3 c = b.getCenter() - a.getCenter(); // Vector from one center to the
	                                        // other
	F32 cLen = c.getLength();

	if(cLen + b.getRadius() < a.getRadius())
	{
		return a;
	}
	else if(cLen + a.getRadius() < b.getRadius())
	{
		return b;
	}

	Vec3 bnorm = c / cLen;

	Vec3 ca = (-bnorm) * a.getRadius() + a.getCenter();
	Vec3 cb = (bnorm) * b.getRadius() + b.getCenter();

	return Sphere((ca + cb) / 2.0, (ca - cb).getLength() / 2.0);
}

//==============================================================================
void Sphere::toAabb(Aabb& aabb) const
{
	aabb.setMin(center - radius);
	aabb.setMax(center + radius);
}

} // end namespace
