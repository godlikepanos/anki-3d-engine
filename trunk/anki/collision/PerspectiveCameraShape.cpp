#include "anki/collision/PerspectiveCameraShape.h"
#include "anki/collision/LineSegment.h"


namespace anki {


//==============================================================================
float PerspectiveCameraShape::testPlane(const Plane& p) const
{
	const PerspectiveCameraShape& pcs = *this;
	float o = 0.0;

	for(uint i = 0; i < 4; i++)
	{
		LineSegment ls(pcs.getEye(), pcs.getDirections()[i]);
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
void PerspectiveCameraShape::setAll(float fovX, float fovY,
	float zNear, float zFar, const Transform& trf)
{
	eye = Vec3(0.0, 0.0, -zNear);

	float x = zFar / tan((Math::PI - fovX) / 2.0);
	float y = tan(fovY / 2.0) * zFar;
	float z = -zFar;

	dirs[0] = Vec3(x, y, z - zNear); // top right
	dirs[1] = Vec3(-x, y, z - zNear); // top left
	dirs[2] = Vec3(-x, -y, z - zNear); // bottom left
	dirs[3] = Vec3(x, -y, z - zNear); // bottom right

	eye.transform(trf);
	for(uint i = 0; i < 4; i++)
	{
		dirs[i] = trf.getRotation() * dirs[i];
	}
}


//==============================================================================
PerspectiveCameraShape PerspectiveCameraShape::getTransformed(
	const Transform& trf) const
{
	PerspectiveCameraShape o;
	o.eye = eye.getTransformed(trf);

	for(uint i = 0; i < 4; i++)
	{
		o.dirs[i] = trf.getRotation() * dirs[i];
	}

	return o;
}


} // end namespace
