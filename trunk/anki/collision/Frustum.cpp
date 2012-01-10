#include "anki/collision/Frustum.h"
#include "anki/collision/LineSegment.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
// Frustum                                                                     =
//==============================================================================

//==============================================================================
Frustum& Frustum::operator=(const Frustum& b)
{
	planes = b.planes;
	zNear = b.zNear;
	zFar = b.zFar;
	return *this;
}


//==============================================================================
bool Frustum::insideFrustum(const CollisionShape& b) const
{
	BOOST_FOREACH(const Plane& plane, planes)
	{
		if(b.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}


//==============================================================================
void Frustum::transform(const Transform& trf)
{
	// Planes
	for(uint i = 0; i < planes.size(); ++i)
	{
		planes[i].transform(trf);
	}
}


//==============================================================================
// PerspectiveFrustum                                                          =
//==============================================================================

//==============================================================================
PerspectiveFrustum& PerspectiveFrustum::operator=(const PerspectiveFrustum& b)
{
	Frustum::operator=(*this);
	eye = b.eye;
	dirs = b.dirs;
	fovX = b.fovX;
	fovY = b.fovY;
	return *this;
}


//==============================================================================
float PerspectiveFrustum::testPlane(const Plane& p) const
{
	float o = 0.0;

	BOOST_FOREACH(const Vec3& dir, dirs)
	{
		LineSegment ls(eye, dir);
		float t = ls.testPlane(p);

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
void PerspectiveFrustum::transform(const Transform& trf)
{
	Frustum::transform(trf);

	eye.transform(trf);

	for(uint i = 0; i < dirs.size(); i++)
	{
		dirs[i] = trf.getRotation() * dirs[i];
	}
}


//==============================================================================
void PerspectiveFrustum::recalculate()
{
	// Planes
	//
	float c, s; // cos & sine

	Math::sinCos(Math::PI + fovX / 2, s, c);
	// right
	planes[FP_RIGHT] = Plane(Vec3(c, 0.0, s), 0.0);
	// left
	planes[FP_LEFT] = Plane(Vec3(-c, 0.0, s), 0.0);

	Math::sinCos((3 * Math::PI - fovY) * 0.5, s, c);
	// top
	planes[FP_TOP] = Plane(Vec3(0.0, s, c), 0.0);
	// bottom
	planes[FP_BOTTOM] = Plane(Vec3(0.0, -s, c), 0.0);

	// near
	planes[FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), zNear);
	// far
	planes[FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), -zFar);

	// Rest
	//
	eye = Vec3(0.0, 0.0, -zNear);

	float x = zFar / tan((Math::PI - fovX) / 2.0);
	float y = tan(fovY / 2.0) * zFar;
	float z = -zFar;

	dirs[0] = Vec3(x, y, z - zNear); // top right
	dirs[1] = Vec3(-x, y, z - zNear); // top left
	dirs[2] = Vec3(-x, -y, z - zNear); // bottom left
	dirs[3] = Vec3(x, -y, z - zNear); // bottom right
}


} // end namespace
