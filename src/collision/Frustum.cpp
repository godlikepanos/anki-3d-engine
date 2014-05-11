#include "anki/collision/Frustum.h"
#include "anki/collision/LineSegment.h"
#include "anki/collision/Aabb.h"
#include <limits>

namespace anki {

//==============================================================================
// Frustum                                                                     =
//==============================================================================

//==============================================================================
Frustum& Frustum::operator=(const Frustum& b)
{
	ANKI_ASSERT(m_type == b.m_type);
	m_near = b.m_near;
	m_far = b.m_far;
	m_planes = b.m_planes;
	m_trf = b.m_trf;
	return *this;
}

//==============================================================================
Bool Frustum::insideFrustum(const CollisionShape& b) const
{
	for(const Plane& plane : m_planes)
	{
		if(b.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}

//==============================================================================
void Frustum::computeAabb(Aabb& aabb) const
{
	switch(m_type)
	{
	case FrustumType::PERSPECTIVE:
		static_cast<const PerspectiveFrustum*>(this)->computeAabb(aabb);
		break;
	case FrustumType::ORTHOGRAPHIC:
		static_cast<const OrthographicFrustum*>(this)->computeAabb(aabb);
		break;
	default:
		ANKI_ASSERT(0);
	}
}

//==============================================================================
// PerspectiveFrustum                                                          =
//==============================================================================

//==============================================================================
PerspectiveFrustum& PerspectiveFrustum::operator=(const PerspectiveFrustum& b)
{
	Frustum::operator=(b);
	m_fovX = b.m_fovX;
	m_fovY = b.m_fovY;
	m_eye = b.m_eye;
	m_dirs = b.m_dirs;
	return *this;
}

//==============================================================================
F32 PerspectiveFrustum::testPlane(const Plane& p) const
{
	// At this point all are in the front side of the plane, all the are 
	// intersecting or all on the negative side

	LineSegment ls(m_eye, m_dirs[0]);
	F32 o = ls.testPlane(p);

	for(U i = 1; i < m_dirs.getSize(); i++)
	{
		LineSegment ls(m_eye, m_dirs[i]);
		F32 t = ls.testPlane(p);

		if(t == 0.0)
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
void PerspectiveFrustum::transformShape()
{
	m_eye.transform(m_trf);

	for(Vec3& dir : m_dirs)
	{
		dir = m_trf.getRotation() * dir;
	}
}

//==============================================================================
void PerspectiveFrustum::transform(const Transform& trf)
{
	m_trf.transform(trf);
	recalculate();
}

//==============================================================================
void PerspectiveFrustum::setTransform(const Transform& trf)
{
	m_trf = trf;
	recalculate();
}

//==============================================================================
void PerspectiveFrustum::computeAabb(Aabb& aabb) const
{
	Array<Vec3, 5> points = {{
		m_dirs[0], m_dirs[1], m_dirs[2], m_dirs[3], Vec3(0.0)}};
	aabb.set(points);
	aabb.getMin() += m_eye;
	aabb.getMax() += m_eye;
}

//==============================================================================
void PerspectiveFrustum::recalculate()
{
	// Planes
	//
	F32 c, s; // cos & sine

	sinCos(getPi<F32>() + m_fovX / 2.0, s, c);
	// right
	m_planes[(U)FrustumPlane::RIGHT] = Plane(Vec3(c, 0.0, s), 0.0);
	// left
	m_planes[(U)FrustumPlane::LEFT] = Plane(Vec3(-c, 0.0, s), 0.0);

	sinCos((getPi<F32>() + m_fovY) * 0.5, s, c);
	// bottom
	m_planes[(U)FrustumPlane::BOTTOM] = Plane(Vec3(0.0, s, c), 0.0);
	// top
	m_planes[(U)FrustumPlane::TOP] = Plane(Vec3(0.0, -s, c), 0.0);

	// near
	m_planes[(U)FrustumPlane::NEAR] = Plane(Vec3(0.0, 0.0, -1.0), m_near);
	// far
	m_planes[(U)FrustumPlane::FAR] = Plane(Vec3(0.0, 0.0, 1.0), -m_far);

	// Rest
	//
	m_eye = Vec3(0.0, 0.0, -m_near);

	F32 x = m_far / tan((getPi<F32>() - m_fovX) / 2.0);
	F32 y = tan(m_fovY / 2.0) * m_far;
	F32 z = -m_far;

	m_dirs[0] = Vec3(x, y, z - m_near); // top right
	m_dirs[1] = Vec3(-x, y, z - m_near); // top left
	m_dirs[2] = Vec3(-x, -y, z - m_near); // bottom left
	m_dirs[3] = Vec3(x, -y, z - m_near); // bottom right

	// Transform
	//
	transformPlanes();
	transformShape();
}

//==============================================================================
Mat4 PerspectiveFrustum::calculateProjectionMatrix() const
{
	ANKI_ASSERT(m_fovX != 0.0 && m_fovY != 0.0);
	Mat4 projectionMat;
	F32 g = m_near - m_far;

#if 1
	projectionMat(0, 0) = 1.0 / tanf(m_fovX * 0.5);
	projectionMat(0, 1) = 0.0;
	projectionMat(0, 2) = 0.0;
	projectionMat(0, 3) = 0.0;
	projectionMat(1, 0) = 0.0;
	projectionMat(1, 1) = 1.0 / tanf(m_fovY * 0.5);
	projectionMat(1, 2) = 0.0;
	projectionMat(1, 3) = 0.0;
	projectionMat(2, 0) = 0.0;
	projectionMat(2, 1) = 0.0;
	projectionMat(2, 2) = (m_far + m_near) / g;
	projectionMat(2, 3) = (2.0 * m_far * m_near) / g;
	projectionMat(3, 0) = 0.0;
	projectionMat(3, 1) = 0.0;
	projectionMat(3, 2) = -1.0;
	projectionMat(3, 3) = 0.0;
#else
	float f = 1.0 / tan(m_fovY * 0.5); // f = cot(m_fovY/2)

	projectionMat(0, 0) = f * m_fovY / m_fovX; // = f/aspectRatio;
	projectionMat(0, 1) = 0.0;
	projectionMat(0, 2) = 0.0;
	projectionMat(0, 3) = 0.0;
	projectionMat(1, 0) = 0.0;
	projectionMat(1, 1) = f;
	projectionMat(1, 2) = 0.0;
	projectionMat(1, 3) = 0.0;
	projectionMat(2, 0) = 0.0;
	projectionMat(2, 1) = 0.0;
	projectionMat(2, 2) = (m_far + m_near) / g;
	projectionMat(2, 3) = (2.0 * m_far * m_near) / g;
	projectionMat(3, 0) = 0.0;
	projectionMat(3, 1) = 0.0;
	projectionMat(3, 2) = -1.0;
	projectionMat(3, 3) = 0.0;
#endif


	return projectionMat;
}

//==============================================================================
// OrthographicFrustum                                                         =
//==============================================================================

//==============================================================================
OrthographicFrustum& OrthographicFrustum::operator=(
	const OrthographicFrustum& b)
{
	Frustum::operator=(b);
	m_left = b.m_left;
	m_right = b.m_right;
	m_top = b.m_top;
	m_bottom = b.m_bottom;
	m_obb = b.m_obb;
	return *this;
}

//==============================================================================
void OrthographicFrustum::transform(const Transform& trf)
{
	m_trf.transform(trf);
	recalculate();
}

//==============================================================================
void OrthographicFrustum::setTransform(const Transform& trf)
{
	m_trf = trf;
	recalculate();
}

//==============================================================================
Mat4 OrthographicFrustum::calculateProjectionMatrix() const
{
	ANKI_ASSERT(m_right != 0.0 && m_left != 0.0 && m_top != 0.0 
		&& m_bottom != 0.0 && m_near != 0.0 && m_far != 0.0);
	F32 difx = m_right - m_left;
	F32 dify = m_top - m_bottom;
	F32 difz = m_far - m_near;
	F32 tx = -(m_right + m_left) / difx;
	F32 ty = -(m_top + m_bottom) / dify;
	F32 tz = -(m_far + m_near) / difz;
	Mat4 m;

	m(0, 0) = 2.0 / difx;
	m(0, 1) = 0.0;
	m(0, 2) = 0.0;
	m(0, 3) = tx;
	m(1, 0) = 0.0;
	m(1, 1) = 2.0 / dify;
	m(1, 2) = 0.0;
	m(1, 3) = ty;
	m(2, 0) = 0.0;
	m(2, 1) = 0.0;
	m(2, 2) = -2.0 / difz;
	m(2, 3) = tz;
	m(3, 0) = 0.0;
	m(3, 1) = 0.0;
	m(3, 2) = 0.0;
	m(3, 3) = 1.0;

	return m;
}

//==============================================================================
void OrthographicFrustum::recalculate()
{
	// Planes
	m_planes[(U)FrustumPlane::LEFT] = Plane(Vec3(1.0, 0.0, 0.0), m_left);
	m_planes[(U)FrustumPlane::RIGHT] = Plane(Vec3(-1.0, 0.0, 0.0), -m_right);
	m_planes[(U)FrustumPlane::NEAR] = Plane(Vec3(0.0, 0.0, -1.0), m_near);
	m_planes[(U)FrustumPlane::FAR] = Plane(Vec3(0.0, 0.0, 1.0), -m_far);
	m_planes[(U)FrustumPlane::TOP] = Plane(Vec3(0.0, -1.0, 0.0), -m_top);
	m_planes[(U)FrustumPlane::BOTTOM] = Plane(Vec3(0.0, 1.0, 0.0), m_bottom);

	// OBB
	Vec3 c((m_right + m_left) * 0.5, 
		(m_top + m_bottom) * 0.5, 
		-(m_far + m_near) * 0.5);
	Vec3 e = Vec3(m_right, m_top, -m_far) - c;
	m_obb = Obb(c, Mat3::getIdentity(), e);

	// Transform
	transformPlanes();
	transformShape();
}

} // end namespace anki
