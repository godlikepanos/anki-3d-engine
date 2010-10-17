#ifndef PLANE_H
#define PLANE_H

#include "CollisionShape.h"
#include "Math.h"
#include "Properties.h"


/// Plane collision shape
class Plane: public CollisionShape
{
	PROPERTY_RW(Vec3, normal, setNormal, getNormal)
	PROPERTY_RW(float, offset, setOffset, getOffset)

	public:
		Plane(): CollisionShape(CST_PLANE) {}

		void setFrom3Vec3(const Vec3& p0, const Vec3& p1, const Vec3& p2);
};


#endif
