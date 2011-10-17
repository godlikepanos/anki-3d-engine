#include "anki/collision/PlaneTests.h"
#include "anki/collision/Collision.h"


namespace anki {


//==============================================================================
float PlaneTests::test(const Plane& p, const CollisionShape& cs)
{
	float t;
	switch(cs.getCollisionShapeType())
	{
		case CollisionShape::CST_LINE_SEG:
			t = test(p, static_cast<const LineSegment&>(cs));
			break;
		case CollisionShape::CST_RAY:
			t = test(p, static_cast<const Ray&>(cs));
			break;
		case CollisionShape::CST_PLANE:
			t = test(p, static_cast<const Plane&>(cs));
			break;
		case CollisionShape::CST_SPHERE:
			t = test(p, static_cast<const Sphere&>(cs));
			break;
		case CollisionShape::CST_AABB:
			t = test(p, static_cast<const Aabb&>(cs));
			break;
		case CollisionShape::CST_OBB:
			t = test(p, static_cast<const Obb&>(cs));
			break;
		case CollisionShape::CST_PERSPECTIVE_CAMERA_FRUSTRUM:
			t = test(p, static_cast<const PerspectiveCameraShape&>(cs));
			break;
		default:
			ASSERT(0 && "Forgot something");
	}
	return t;
}


//==============================================================================
float PlaneTests::test(const Plane& p, const LineSegment& ls)
{
	const Vec3& p0 = ls.getOrigin();
	Vec3 p1 = ls.getOrigin() + ls.getDirection();

	float dist0 = p.test(p0);
	float dist1 = p.test(p1);

	if(dist0 > 0.0)
	{
		if(dist1 > 0.0)
		{
			return std::min(dist0, dist1);
		}
		else
		{
			return 0.0;
		}
	}
	else
	{
		if(dist1 < 0.0)
		{
			return std::max(dist0, dist1);
		}
		else
		{
			return 0.0;
		}
	}
}


//==============================================================================
float PlaneTests::test(const Plane& p, const Obb& obb)
{
	Vec3 xNormal = obb.getRotation().getTransposed() * p.getNormal();

	// maximum extent in direction of plane normal
	float r = fabs(obb.getExtend().x() * xNormal.x()) +
		fabs(obb.getExtend().y() * xNormal.y()) +
		fabs(obb.getExtend().z() * xNormal.z());
	// signed distance between box center and plane
	float d = p.test(obb.getCenter());

	// return signed distance
	if(fabs(d) < r)
	{
		return 0.0;
	}
	else if(d < 0.0)
	{
		return d + r;
	}
	else
	{
		return d - r;
	}
}


//==============================================================================
float PlaneTests::test(const Plane& p, const PerspectiveCameraShape& pcs)
{
	float o = 0.0;

	for(uint i = 0; i < 4; i++)
	{
		LineSegment ls(pcs.getEye(), pcs.getDirections()[i]);
		float t = test(p, ls);

		if(t == 0)
		{
			return 0.0;
		}
		else if(t < 0.0)
		{
			o = std::max(o, t);
		}
		else
		{
			o = std::min(o, t);
		}
	}

	return o;
}


//==============================================================================
float PlaneTests::test(const Plane& /*p*/, const Plane& /*p1*/)
{
	ASSERT(0 && "Ambiguous call");
	return 0.0;
}


//==============================================================================
float PlaneTests::test(const Plane& p, const Ray& r)
{
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
float PlaneTests::test(const Plane& p, const Sphere& s)
{
	float dist = p.test(s.getCenter());

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
float PlaneTests::test(const Plane& p, const Aabb& aabb)
{
	Vec3 diagMin, diagMax;
	// set min/max values for x,y,z direction
	for(int i = 0; i < 3; i++)
	{
		if(p.getNormal()[i] >= 0.0)
		{
			diagMin[i] = aabb.getMin()[i];
			diagMax[i] = aabb.getMax()[i];
		}
		else
		{
			diagMin[i] = aabb.getMax()[i];
			diagMax[i] = aabb.getMin()[i];
		}
	}

	// minimum on positive side of plane, box on positive side
	float test = p.test(diagMin);
	if(test > 0.0)
	{
		return test;
	}

	test = p.test(diagMax);
	// min on non-positive side, max on non-negative side, intersection
	if(test >= 0.0)
	{
		return 0.0;
	}
	// max on negative side, box on negative side
	else
	{
		return test;
	}
}


} // end namespace
