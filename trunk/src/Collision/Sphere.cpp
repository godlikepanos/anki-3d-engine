#include "Sphere.h"
#include "Plane.h"


//======================================================================================================================
// getTransformed                                                                                                      =
//======================================================================================================================
Sphere Sphere::getTransformed(const Vec3& translate, const Mat3& rotate, float scale) const
{
	Sphere newSphere;
	newSphere.center = center.getTransformed(translate, rotate, scale);
	newSphere.radius = radius * scale;
	return newSphere;
}


//======================================================================================================================
// testPlane                                                                                                           =
//======================================================================================================================
float Sphere::testPlane(const Plane& plane) const
{
	float dist = plane.test(center);

	if(dist > radius)
	{
		return dist - radius;
	}
	else if(-dist > radius)
	{
		return dist + radius;
	}
	else
	{
		return 0.0;
	}
}

//======================================================================================================================
//                                                                                                                     =
//======================================================================================================================
void Sphere::set(const float* pointer, size_t stride, int count)
{
	void* tmpPtr = (void*)pointer;
	Vec3 min(*(Vec3*)tmpPtr);
	Vec3 max(*(Vec3*)tmpPtr);

	// for all the Vec3 calc the max and min
	for(int i=1; i<count; i++)
	{
		tmpPtr = (char*)tmpPtr + stride;

		const Vec3& tmp = *((Vec3*)tmpPtr);

		for(int j=0; j<3; j++)
		{
			if(tmp[j] > max[j])
			{
				max[j] = tmp[j];
			}
			else if(tmp[j] < min[j])
			{
				min[j] = tmp[j];
			}
		}
	}

	center = (min + max) * 0.5; // average

	tmpPtr = (void*)pointer;
	float maxDist = (*((Vec3*)tmpPtr) - center).getLengthSquared(); // max distance between center and the vec3 arr
	for(int i=1; i<count; i++)
	{
		tmpPtr = (char*)tmpPtr + stride;

		const Vec3& vertco = *((Vec3*)tmpPtr);
		float dist = (vertco - center).getLengthSquared();
		if(dist > maxDist)
			maxDist = dist;
	}

	radius = M::sqrt(maxDist);
}
