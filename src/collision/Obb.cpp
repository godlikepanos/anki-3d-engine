#include "anki/collision/Obb.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Aabb.h"

namespace anki {

//==============================================================================
Obb::Obb()
	:	CollisionShape(Type::OBB),
		m_center(Vec3(0.0)),
		m_rotation(Mat3::getIdentity()),
		m_extends(Vec3(getEpsilon<F32>()))
{}

//==============================================================================
Obb::Obb(const Obb& b)
	: 	CollisionShape(Type::OBB), 
		m_center(b.m_center), 
		m_rotation(b.m_rotation),
		m_extends(b.m_extends)
{}

//==============================================================================
Obb::Obb(const Vec3& center, const Mat3& rotation, const Vec3& extends)
	:	CollisionShape(Type::OBB), 
		m_center(center), 
		m_rotation(rotation),
		m_extends(extends)
{}

//==============================================================================
F32 Obb::testPlane(const Plane& p) const
{
	const Obb& obb = *this;
	Vec3 xNormal = obb.getRotation().getTransposed() * p.getNormal();

	// maximum extent in direction of plane normal
	F32 r =
		fabs(obb.getExtend().x() * xNormal.x()) +
		fabs(obb.getExtend().y() * xNormal.y()) +
		fabs(obb.getExtend().z() * xNormal.z());
	// signed distance between box center and plane
	F32 d = p.test(obb.getCenter());

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
Obb Obb::getTransformed(const Transform& transform) const
{
	Obb out;
	out.m_extends = m_extends * transform.getScale();
	out.m_center = m_center.getTransformed(transform);
	out.m_rotation = transform.getRotation() * m_rotation;
	return out;
}

//==============================================================================
Obb Obb::getCompoundShape(const Obb& b) const
{
	Obb out;

	Array<Vec3, 8> points0;
	Array<Vec3, 8> points1;

	getExtremePoints(points0);
	b.getExtremePoints(points1);

	Array<Vec3, 16> points;
	for(U i = 0; i < 8; i++)
	{
		points[i] = points0[i];
		points[i + 8] = points1[i];
	}

	out.setFromPointCloud(&points[0], points.size(), sizeof(Vec3), 
		sizeof(Vec3) * points.size());
	return out;
}

//==============================================================================
void Obb::getExtremePoints(Array<Vec3, 8>& points) const
{
	// L: left, R: right, T: top, B: bottom, F: front, B: back
	enum
	{
		RTF,
		LTF,
		LBF,
		RBF,
		RTB,
		LTB,
		LBB,
		RBB
	};

	Vec3 er = m_rotation * m_extends; // extend rotated

	points[RTF] = er;
	points[LBB] = -er;

	Vec3 xAxis = m_rotation.getColumn(0);
	Vec3 yAxis = m_rotation.getColumn(1);
	Vec3 zAxis = m_rotation.getColumn(2);

	// Reflection: x1' = 2n|x1.n| - x1

	points[RBB] = 2.0f * er.dot(xAxis) * xAxis - er;
	points[LTB] = 2.0f * er.dot(yAxis) * yAxis - er;
	points[LBF] = 2.0f * er.dot(zAxis) * zAxis - er;

	points[LTF] = 2.0f * points[LBB].dot(-xAxis) * -xAxis - points[LBB];
	points[RTB] = 2.0f * points[LTF].dot(yAxis) * yAxis - points[LTF];
	points[RBF] = 2.0f * points[LTF].dot(zAxis) * zAxis - points[LTF];

	for(Vec3& point : points)
	{
		point += m_center;
	}
}

//==============================================================================
void Obb::computeAabb(Aabb& aabb) const
{
	Mat3 absM;
	for(U i = 0; i < 9; ++i)
	{
		absM[i] = fabs(m_rotation[i]);
	}

	Vec3 newE = absM * m_extends;

	// Add a small epsilon to avoid some assertions
	aabb = Aabb(m_center - newE, 
		m_center + newE + Vec3(getEpsilon<F32>() * 100.0));
}

//==============================================================================
void Obb::setFromPointCloud(
	const void* buff, U count, PtrSize stride, PtrSize buffSize)
{
	Vec3 min = Vec3(MAX_F32);
	Vec3 max = Vec3(MIN_F32);

	iteratePointCloud(buff, count, stride, buffSize, [&](const Vec3& pos)
	{
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

	// Set the locals
	m_center = (max + min) / 2.0;
	m_rotation = Mat3::getIdentity();
	m_extends = max - m_center;
}

} // end namespace anki
