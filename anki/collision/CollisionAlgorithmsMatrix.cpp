#include "anki/collision/CollisionAlgorithmsMatrix.h"
#include "anki/collision/Collision.h"
#include "anki/util/Assert.h"
#include "anki/math/Math.h"
#include <limits>


namespace anki {


//==============================================================================
template<typename T>
bool CollisionAlgorithmsMatrix::tcollide(const CollisionShape& a,
	const CollisionShape& b)
{
	const T& t = static_cast<const T&>(a);
	bool out;

	switch(b.getCollisionShapeType())
	{
		case CollisionShape::CST_LINE_SEG:
			out = collide(t, static_cast<const LineSegment&>(b));
		case CollisionShape::CST_RAY:
			out = collide(t, static_cast<const Ray&>(b));
		case CollisionShape::CST_PLANE:
			out = collide(t, static_cast<const Plane&>(b));
		case CollisionShape::CST_SPHERE:
			out = collide(t, static_cast<const Sphere&>(b));
		case CollisionShape::CST_AABB:
			out = collide(t, static_cast<const Aabb&>(b));
		case CollisionShape::CST_OBB:
			out = collide(t, static_cast<const Obb&>(b));
		case CollisionShape::CST_PERSPECTIVE_CAMERA_FRUSTRUM:
			out = collide(t, static_cast<const PerspectiveCameraShape&>(b));
		default:
			ANKI_ASSERT(0 && "Forgot something");
	}

	return out;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const CollisionShape& a,
	const CollisionShape& b)
{
	bool out;
	switch(a.getCollisionShapeType())
	{
		case CollisionShape::CST_LINE_SEG:
			out = tcollide<LineSegment>(a, b);
		case CollisionShape::CST_RAY:
			out = tcollide<Ray>(a, b);
		case CollisionShape::CST_PLANE:
			out = tcollide<Plane>(a, b);
		case CollisionShape::CST_SPHERE:
			out = tcollide<Sphere>(a, b);
		case CollisionShape::CST_AABB:
			out = tcollide<Aabb>(a, b);
		case CollisionShape::CST_OBB:
			out = tcollide<Obb>(a, b);
		case CollisionShape::CST_PERSPECTIVE_CAMERA_FRUSTRUM:
			out = tcollide<PerspectiveCameraShape>(a, b);
		default:
			ANKI_ASSERT(0 && "Forgot something");
	}

	return out;
}


//==============================================================================
// 1st row                                                                     =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& /*a*/, const Ls& /*b*/)
{
	ANKI_ASSERT(0 && "N/A");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& ls, const Obb& obb)
{
	float maxS = std::numeric_limits<float>::min();
	float minT = std::numeric_limits<float>::max();

	// compute difference vector
	Vec3 diff = obb.getCenter() - ls.getOrigin();

	// for each axis do
	for(int i = 0; i < 3; ++i)
	{
		// get axis i
		Vec3 axis = obb.getRotation().getColumn(i);

		// project relative vector onto axis
		float e = axis.dot(diff);
		float f = ls.getDirection().dot(axis);

		// ray is parallel to plane
		if(Math::isZero(f))
		{
			// ray passes by box
			if(-e - obb.getExtend()[i] > 0.0 || -e + obb.getExtend()[i] > 0.0)
			{
				return false;
			}
			continue;
		}

		float s = (e - obb.getExtend()[i]) / f;
		float t = (e + obb.getExtend()[i]) / f;

		// fix order
		if(s > t)
		{
			float temp = s;
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

	// done, have intersection
	return true;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& /*a*/, const Pcs& /*b*/)
{
	ANKI_ASSERT(0 && "Not implemented yet");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& ls, const Plane& p)
{
	return ls.testPlane(p) == 0.0;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& /*a*/, const Ray& /*b*/)
{
	ANKI_ASSERT(0 && "N/A");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& ls, const Sphere& s)
{
	const Vec3& v = ls.getDirection();
	Vec3 w0 = s.getCenter() - ls.getOrigin();
	float w0dv = w0.dot(v);
	float rsq = s.getRadius() * s.getRadius();

	if(w0dv < 0.0) // if the ang is >90
	{
		return w0.getLengthSquared() <= rsq;
	}

	Vec3 w1 = w0 - v; // aka center - P1, where P1 = seg.origin + seg.dir
	float w1dv = w1.dot(v);

	if(w1dv > 0.0) // if the ang is <90
	{
		return w1.getLengthSquared() <= rsq;
	}

	// the big parenthesis is the projection of w0 to v
	Vec3 tmp = w0 - (v * (w0.dot(v) / v.getLengthSquared()));
	return tmp.getLengthSquared() <= rsq;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ls& ls, const Aabb& aabb)
{
	float maxS = std::numeric_limits<float>::min();
	float minT = std::numeric_limits<float>::max();

	// do tests against three sets of planes
	for(int i = 0; i < 3; ++i)
	{
		// segment is parallel to plane
		if(Math::isZero(ls.getDirection()[i]))
		{
			// segment passes by box
			if(ls.getOrigin()[i] < aabb.getMin()[i] ||
			   ls.getOrigin()[i] > aabb.getMax()[i])
			{
				return false;
			}
		}
		else
		{
			// compute intersection parameters and sort
			float s = (aabb.getMin()[i] - ls.getOrigin()[i]) /
				ls.getDirection()[i];
			float t = (aabb.getMax()[i] - ls.getOrigin()[i]) /
				ls.getDirection()[i];
			if(s > t)
			{
				float temp = s;
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
// 2nd row                                                                     =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Obb& o0, const Obb& o1)
{
	// extent vectors
	const Vec3& a = o0.getExtend();
	const Vec3& b = o1.getExtend();

	// test factors
	float cTest, aTest, bTest;
	bool parallelAxes = false;

	// transpose of rotation of B relative to A, i.e. (R_b^T * R_a)^T
	Mat3 rt = o0.getRotation().getTransposed() * o1.getRotation();

	// absolute value of relative rotation matrix
	Mat3 rabs;
	for(uint i = 0; i < 3; ++i)
	{
		for(uint j = 0; j < 3; ++j)
		{
			rabs(i, j) = fabs(rt(i, j));
			// if magnitude of dot product between axes is close to one
			if(rabs(i, j) + Math::EPSILON >= 1.0)
			{
				// then box A and box B have near-parallel axes
				parallelAxes = true;
			}
		}
	}

	// relative translation (in A's frame)
	Vec3 c = o0.getRotation().getTransposed() *
		(o1.getCenter() - o0.getCenter());

	// separating axis A0
	cTest = fabs(c.x());
	aTest = a.x();
	bTest = b.x() * rabs(0, 0) + b.y() * rabs(0, 1) + b.z() * rabs(0, 2);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A1
	cTest = fabs(c.y());
	aTest = a.y();
	bTest = b.x() * rabs(1, 0) + b.y() * rabs(1, 1) + b.z() * rabs(1, 2);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A2
	cTest = fabs(c.z());
	aTest = a.z();
	bTest = b.x() * rabs(2, 0) + b.y() * rabs(2, 1) + b.z() * rabs(2, 2);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis B0
	cTest = fabs(c.x() * rt(0, 0) + c.y() * rt(1, 0) + c.z() * rt(2, 0));
	aTest = a.x() * rabs(0, 0) + a.y() * rabs(1, 0) + a.z() * rabs(2, 0);
	bTest = b.x();
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis B1
	cTest = fabs(c.x() * rt(0, 1) + c.y() * rt(1, 1) + c.z() * rt(2, 1));
	aTest = a.x() * rabs(0, 1) + a.y() * rabs(1, 1) + a.z() * rabs(2, 1);
	bTest = b.y();
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis B2
	cTest = fabs(c.x() * rt(0, 2) + c.y() * rt(1, 2) + c.z() * rt(2, 2));
	aTest = a.x() * rabs(0, 2) + a.y() * rabs(1, 2) + a.z() * rabs(2, 2);
	bTest = b.z();
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// if the two boxes have parallel axes, we're done, intersection
	if(parallelAxes)
	{
		return true;
	}

	// separating axis A0 x B0
	cTest = fabs(c.z() * rt(1, 0) - c.y() * rt(2, 0));
	aTest = a.y() * rabs(2, 0) + a.z() * rabs(1, 0);
	bTest = b.y() * rabs(0, 2) + b.z() * rabs(0, 1);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A0 x B1
	cTest = fabs(c.z() * rt(1, 1) - c.y() * rt(2, 1));
	aTest = a.y() * rabs(2, 1) + a.z() * rabs(1, 1);
	bTest = b.x() * rabs(0, 2) + b.z() * rabs(0, 0);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A0 x B2
	cTest = fabs(c.z() * rt(1, 2) - c.y() * rt(2, 2));
	aTest = a.y() * rabs(2, 2) + a.z() * rabs(1, 2);
	bTest = b.x() * rabs(0, 1) + b.y() * rabs(0, 0);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A1 x B0
	cTest = fabs(c.x() * rt(2, 0) - c.z() * rt(0, 0));
	aTest = a.x() * rabs(2, 0) + a.z() * rabs(0, 0);
	bTest = b.y() * rabs(1, 2) + b.z() * rabs(1, 1);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A1 x B1
	cTest = fabs(c.x() * rt(2, 1) - c.z() * rt(0, 1));
	aTest = a.x() * rabs(2, 1) + a.z() * rabs(0, 1);
	bTest = b.x() * rabs(1, 2) + b.z() * rabs(1, 0);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A1 x B2
	cTest = fabs(c.x() * rt(2, 2) - c.z() * rt(0, 2));
	aTest = a.x() * rabs(2, 2) + a.z() * rabs(0, 2);
	bTest = b.x() * rabs(1, 1) + b.y() * rabs(1, 0);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A2 x B0
	cTest = fabs(c.y() * rt(0, 0) - c.x() * rt(1, 0));
	aTest = a.x() * rabs(1, 0) + a.y() * rabs(0, 0);
	bTest = b.y() * rabs(2, 2) + b.z() * rabs(2, 1);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A2 x B1
	cTest = fabs(c.y() * rt(0, 1) - c.x() * rt(1, 1));
	aTest = a.x() * rabs(1, 1) + a.y() * rabs(0, 1);
	bTest = b.x() * rabs(2, 2) + b.z() * rabs(2, 0);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// separating axis A2 x B2
	cTest = fabs(c.y() * rt(0, 2) - c.x() * rt(1, 2));
	aTest = a.x() * rabs(1, 2) + a.y() * rabs(0, 2);
	bTest = b.x() * rabs(2, 1) + b.y() * rabs(2, 0);
	if(cTest > aTest + bTest)
	{
		return false;
	}

	// all tests failed, have intersection
	return true;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Obb& a, const Pcs& b)
{
	ANKI_ASSERT(0 && "Not impelented yet");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Obb& a, const Plane& b)
{
	return a.testPlane(b) == 0.0;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Obb& obb, const Ray& r)
{
	Aabb aabb_(-obb.getExtend(), obb.getExtend());
	Ray newray;
	Mat3 rottrans = obb.getRotation().getTransposed();

	newray.getOrigin() = rottrans * (r.getOrigin() - obb.getCenter());
	newray.getDirection() = rottrans * r.getDirection();

	return collide(newray, aabb_);
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Obb& obb, const Sphere& s)
{
	Aabb aabb_(-obb.getExtend(), obb.getExtend()); // aabb_ is in "this" frame
	Vec3 newCenter = obb.getRotation().getTransposed() *
		(s.getCenter() - obb.getCenter());
	Sphere sphere_(newCenter, s.getRadius()); // sphere1 to "this" fame

	return collide(sphere_, aabb_);
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Obb& obb, const Aabb& aabb)
{
	Vec3 center_ = (aabb.getMax() + aabb.getMin()) * 0.5;
	Vec3 extends_ = (aabb.getMax() - aabb.getMin()) * 0.5;
	Obb obb_(center_, Mat3::getIdentity(), extends_);

	return collide(obb, obb_);
}


//==============================================================================
// 3rd line (PCS)                                                              =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Pcs& a, const Pcs& b)
{
	ANKI_ASSERT(0 && "Not implemented yet");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Pcs& a, const Plane& b)
{
	ANKI_ASSERT(0 && "Not implemented yet");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Pcs& a, const Ray& b)
{
	ANKI_ASSERT(0 && "Not implemented yet");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Pcs& a, const Sphere& b)
{
	ANKI_ASSERT(0 && "Not implemented yet");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Pcs& a, const Aabb& b)
{
	ANKI_ASSERT(0 && "Not implemented yet");
	return false;
}


//==============================================================================
// 4th line (P)                                                                =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Plane& p0, const Plane& p1)
{
	return p0.getNormal() != p1.getNormal();
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Plane& p, const Ray& r)
{
	return r.testPlane(p) == 0.0;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Plane& p, const Sphere& s)
{
	return s.testPlane(p) == 0.0;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Plane& p, const Aabb& aabb)
{
	return aabb.testPlane(p) == 0.0;
}


//==============================================================================
// 5th line (R)                                                                =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ray& a, const Ray& b)
{
	ANKI_ASSERT(0 && "N/A");
	return false;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ray& r, const Sphere& s)
{
	Vec3 w(s.getCenter() - r.getOrigin());
	const Vec3& v = r.getDirection();
	float proj = v.dot(w);
	float wsq = w.getLengthSquared();
	float rsq = s.getRadius() * s.getRadius();

	if(proj < 0.0 && wsq > rsq)
	{
		return false;
	}

	float vsq = v.getLengthSquared();

	return (vsq * wsq - proj * proj <= vsq * rsq);
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Ray& r, const Aabb& aabb)
{
	float maxS = std::numeric_limits<float>::min();
	float minT = std::numeric_limits<float>::max();

	// do tests against three sets of planes
	for(int i = 0; i < 3; ++i)
	{
		// ray is parallel to plane
		if(Math::isZero(r.getDirection()[i]))
		{
			// ray passes by box
			if(r.getOrigin()[i] < aabb.getMin()[i] ||
				r.getOrigin()[i] > aabb.getMax()[i])
			{
				return false;
			}
		}
		else
		{
			// compute intersection parameters and sort
			float s = (aabb.getMin()[i] - r.getOrigin()[i]) /
				r.getDirection()[i];
			float t = (aabb.getMax()[i] - r.getOrigin()[i]) /
				r.getDirection()[i];
			if(s > t)
			{
				float temp = s;
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
			if(minT < 0.0 || maxS > minT)
			{
				return false;
			}
		}
	}

	// done, have intersection
	return true;
}


//==============================================================================
// 6th line (S)                                                                =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Sphere& a, const Sphere& b)
{
	float tmp = a.getRadius() + b.getRadius();
	return (a.getCenter() - b.getCenter()).getLengthSquared() <= tmp * tmp;
}


//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Sphere& s, const Aabb& aabb)
{
	const Vec3& c = s.getCenter();

	// find the box's closest point to the sphere
	Vec3 cp; // Closest Point
	for(uint i = 0; i < 3; i++)
	{
		// if the center is greater than the max then the closest
		// point is the max
		if(c[i] > aabb.getMax()[i])
		{
			cp[i] = aabb.getMax()[i];
		}
		else if(c[i] < aabb.getMin()[i]) // relative to the above
		{
			cp[i] = aabb.getMin()[i];
		}
		else
		{
			// the c lies between min and max
			cp[i] = c[i];
		}
	}

	float rsq = s.getRadius() * s.getRadius();

	// if the c lies totaly inside the box then the sub is the zero,
	// this means that the length is also zero and thus its always smaller
	// than rsq
	Vec3 sub = c - cp;

	if(sub.getLengthSquared() <= rsq)
	{
		return true;
	}

	return false;
}


//==============================================================================
// 7th line (AABB)                                                             =
//==============================================================================

//==============================================================================
bool CollisionAlgorithmsMatrix::collide(const Aabb& a, const Aabb& b)
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


} // end namespace
