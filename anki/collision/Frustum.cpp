#include "anki/collision/Frustum.h"
#include "anki/collision/LineSegment.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
Frustum& Frustum::operator=(const Frustum& b)
{
	type = b.type;
	planes = b.planes;

	eye = b.eye;
	dirs = b.dirs;

	obb = b.obb;
	return *this;
}


//==============================================================================
float Frustum::testPlane(const Plane& p) const
{
	if(type == FT_ORTHOGRAPHIC)
	{
		return obb.testPlane(p);
	}
	else
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
}


//==============================================================================
Frustum Frustum::getTransformed(const Transform& trf) const
{
	Frustum o;

	// Planes
	for(uint i = 0; i < planes.size(); ++i)
	{
		o.planes[i] = planes[i].getTransformed(trf);
	}

	// Other
	if(type == FT_ORTHOGRAPHIC)
	{
		o.obb = obb.getTransformed(trf);
	}
	else
	{
		o.eye = eye.getTransformed(trf);

		for(uint i = 0; i < dirs.size(); i++)
		{
			o.dirs[i] = trf.getRotation() * dirs[i];
		}
	}

	return o;
}


//==============================================================================
void Frustum::setPerspective(float fovX, float fovY,
	float zNear, float zFar, const Transform& trf)
{
	type = FT_PERSPECTIVE;

	eye = Vec3(0.0, 0.0, -zNear);

	float x = zFar / tan((Math::PI - fovX) / 2.0);
	float y = tan(fovY / 2.0) * zFar;
	float z = -zFar;

	dirs[0] = Vec3(x, y, z - zNear); // top right
	dirs[1] = Vec3(-x, y, z - zNear); // top left
	dirs[2] = Vec3(-x, -y, z - zNear); // bottom left
	dirs[3] = Vec3(x, -y, z - zNear); // bottom right

	eye.transform(trf);
	BOOST_FOREACH(Vec3& dir, dirs)
	{
		dir = trf.getRotation() * dir;
	}
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


} // end namespace
