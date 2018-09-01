// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Sphere.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Aabb.h>

namespace anki
{

F32 Sphere::testPlane(const Plane& p) const
{
	F32 dist = p.test(m_center);

	F32 out = dist - m_radius;
	if(out > 0)
	{
		// Do nothing
	}
	else
	{
		out = dist + m_radius;
		if(out < 0)
		{
			// Do nothing
		}
		else
		{
			out = 0.0;
		}
	}

	return out;
}

Sphere Sphere::getTransformed(const Transform& trf) const
{
	Sphere newSphere;

	newSphere.m_center = trf.transform(m_center);
	newSphere.m_radius = m_radius * trf.getScale();
	newSphere.m_radiusSq = newSphere.m_radius * newSphere.m_radius;
	return newSphere;
}

Sphere Sphere::getCompoundShape(const Sphere& b) const
{
	const Sphere& a = *this;

	/// TODO test this
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

	Vec4 c = b.getCenter() - a.getCenter(); // Vector from one center to the
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

	Vec4 bnorm = c / cLen;

	Vec4 ca = (-bnorm) * a.getRadius() + a.getCenter();
	Vec4 cb = (bnorm)*b.getRadius() + b.getCenter();

	return Sphere((ca + cb) / 2.0, (ca - cb).getLength() / 2.0);
}

void Sphere::computeAabb(Aabb& aabb) const
{
	aabb.setMin((m_center - m_radius).xyz0());
	aabb.setMax((m_center + m_radius).xyz0());
}

void Sphere::setFromPointCloud(const void* buff, U count, PtrSize stride, PtrSize buffSize)
{
	// Calc min/max
	Vec4 min(Vec3(MAX_F32), 0.0);
	Vec4 max(Vec3(MIN_F32), 0.0);

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos) {
		for(U j = 0; j < 3; j++)
		{
			if(pos[j] > max[j])
			{
				max[j] = pos[j];
			}
			else if(pos[j] < min[j])
			{
				min[j] = pos[j];
			}
		}
	});

	m_center = (min + max) * 0.5; // average

	// max distance between center and the vec3 arr
	F32 maxDist = MIN_F32;

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos) {
		F32 dist = (Vec4(pos, 0.0) - m_center).getLengthSquared();
		if(dist > maxDist)
		{
			maxDist = dist;
		}
	});

	m_radius = sqrt(maxDist);
	m_radiusSq = m_radius * m_radius;
}

Vec4 Sphere::computeSupport(const Vec4& dir) const
{
	return m_center + dir.getNormalized() * m_radius;
}

Bool Sphere::intersectsRay(
	const Vec4& rayDir, const Vec4& rayOrigin, Array<Vec4, 2>& intersectionPoints, U& intersectionPointCount) const
{
	ANKI_ASSERT(isZero(rayDir.getLengthSquared() - 1.0));
	ANKI_ASSERT(rayDir.w() == 0.0 && rayOrigin.w() == 0.0);

	// See https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	const Vec4& o = rayOrigin;
	const Vec4& l = rayDir;
	const Vec4& c = m_center;
	F32 R2 = m_radiusSq;

	Vec4 o_c = o - c;

	F32 a = l.dot(o_c);
	F32 b = a * a - o_c.getLengthSquared() + R2;

	if(b < 0.0)
	{
		intersectionPointCount = 0;
		return false;
	}
	else if(b == 0.0)
	{
		intersectionPointCount = 1;
		intersectionPoints[0] = -a * l + o;
		return true;
	}
	else
	{
		F32 d = -a - sqrt(b);
		intersectionPointCount = 0;
		if(d > 0.0)
		{
			intersectionPointCount = 1;
			intersectionPoints[0] = d * l + o;
		}

		d = -a + sqrt(b);
		if(d > 0.0)
		{
			intersectionPoints[intersectionPointCount++] = d * l + o;
		}

		return intersectionPointCount > 0;
	}
}

/// https://bartwronski.com/2017/04/13/cull-that-cone/
Bool Sphere::intersectsCone(const Vec4& coneOrigin, const Vec4& coneDir, F32 coneLength, F32 coneAngle) const
{
	ANKI_ASSERT(coneOrigin.w() == 0.0f && coneDir.w() == 0.0f);

	coneAngle /= 2.0f;
	const Vec4 V = m_center - coneOrigin;
	const F32 VlenSq = V.dot(V);
	const F32 V1len = V.dot(coneDir);
	const F32 distanceClosestPoint = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

	const Bool angleCull = distanceClosestPoint > m_radius;
	const Bool frontCull = V1len > m_radius + coneLength;
	const Bool backCull = V1len < -m_radius;
	return !(angleCull || frontCull || backCull);
}

} // end namespace anki
