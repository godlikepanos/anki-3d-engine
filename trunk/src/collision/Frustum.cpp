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
	m_frustumDirty = b.m_frustumDirty;
	return *this;
}

//==============================================================================
Bool Frustum::insideFrustum(const CollisionShape& b)
{
	if(m_frustumDirty)
	{
		m_frustumDirty = false;
		recalculate();

		// recalculate() reset the tranformations so re-transform
		transform(m_trf);
	}

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
void Frustum::transform(const Transform& trf)
{
	if(m_frustumDirty)
	{
		m_frustumDirty = false;
		recalculate();
	}

	m_trf.transform(trf);

	// Transform the compound
	CompoundShape::transform(trf);

	// Transform the planes
	for(Plane& p : m_planes)
	{
		p.transform(trf);
	}
}

//==============================================================================
void Frustum::resetTransform()
{
	recalculate();
	m_frustumDirty = false;
	m_trf.setIdentity();
}

//==============================================================================
// PerspectiveFrustum                                                          =
//==============================================================================

//==============================================================================
PerspectiveFrustum::PerspectiveFrustum()
	: Frustum(Type::PERSPECTIVE)
{
	for(LineSegment& ls : m_segments)
	{
		addShape(&ls);
	}
}

//==============================================================================
PerspectiveFrustum& PerspectiveFrustum::operator=(const PerspectiveFrustum& b)
{
	Frustum::operator=(b);
	m_fovX = b.m_fovX;
	m_fovY = b.m_fovY;
	m_segments = b.m_segments;
	return *this;
}

//==============================================================================
void PerspectiveFrustum::recalculate()
{
	// Planes
	//
	F32 c, s; // cos & sine

	sinCos(getPi<F32>() + m_fovX / 2.0, s, c);
	// right
	m_planes[(U)PlaneType::RIGHT] = Plane(Vec3(c, 0.0, s), 0.0);
	// left
	m_planes[(U)PlaneType::LEFT] = Plane(Vec3(-c, 0.0, s), 0.0);

	sinCos((getPi<F32>() + m_fovY) * 0.5, s, c);
	// bottom
	m_planes[(U)PlaneType::BOTTOM] = Plane(Vec3(0.0, s, c), 0.0);
	// top
	m_planes[(U)PlaneType::TOP] = Plane(Vec3(0.0, -s, c), 0.0);

	// near
	m_planes[(U)PlaneType::NEAR] = Plane(Vec3(0.0, 0.0, -1.0), m_near);
	// far
	m_planes[(U)PlaneType::FAR] = Plane(Vec3(0.0, 0.0, 1.0), -m_far);

	// Rest
	//
	Vec3 eye = Vec3(0.0, 0.0, -m_near);
	for(LineSegment& ls : m_segments)
	{
		ls.setOrigin(eye);
	}

	F32 x = m_far / tan((getPi<F32>() - m_fovX) / 2.0);
	F32 y = tan(m_fovY / 2.0) * m_far;
	F32 z = -m_far;

	m_segments[0].setDirection(Vec3(x, y, z - m_near)); // top right
	m_segments[1].setDirection(Vec3(-x, y, z - m_near)); // top left
	m_segments[2].setDirection(Vec3(-x, -y, z - m_near)); // bottom left
	m_segments[3].setDirection(Vec3(x, -y, z - m_near)); // bottom right
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
OrthographicFrustum::OrthographicFrustum()
	: Frustum(Type::ORTHOGRAPHIC)
{
	addShape(&m_obb);
}

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
	m_planes[(U)PlaneType::LEFT] = Plane(Vec3(1.0, 0.0, 0.0), m_left);
	m_planes[(U)PlaneType::RIGHT] = Plane(Vec3(-1.0, 0.0, 0.0), -m_right);
	m_planes[(U)PlaneType::NEAR] = Plane(Vec3(0.0, 0.0, -1.0), m_near);
	m_planes[(U)PlaneType::FAR] = Plane(Vec3(0.0, 0.0, 1.0), -m_far);
	m_planes[(U)PlaneType::TOP] = Plane(Vec3(0.0, -1.0, 0.0), -m_top);
	m_planes[(U)PlaneType::BOTTOM] = Plane(Vec3(0.0, 1.0, 0.0), m_bottom);

	// OBB
	Vec3 c((m_right + m_left) * 0.5, 
		(m_top + m_bottom) * 0.5, 
		-(m_far + m_near) * 0.5);
	Vec3 e = Vec3(m_right, m_top, -m_far) - c;
	m_obb = Obb(c, Mat3::getIdentity(), e);
}

} // end namespace anki
