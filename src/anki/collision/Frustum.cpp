// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Frustum.h>
#include <anki/collision/LineSegment.h>
#include <anki/collision/Aabb.h>

namespace anki
{

Frustum& Frustum::operator=(const Frustum& b)
{
	ANKI_ASSERT(m_type == b.m_type);
	m_near = b.m_near;
	m_far = b.m_far;
	m_planesL = b.m_planesL;
	m_planesW = b.m_planesW;
	m_trf = b.m_trf;
	return *this;
}

void Frustum::accept(MutableVisitor& v)
{
	CompoundShape::accept(v);
}

void Frustum::accept(ConstVisitor& v) const
{
	CompoundShape::accept(v);
}

F32 Frustum::testPlane(const Plane& p) const
{
	return CompoundShape::testPlane(p);
}

void Frustum::computeAabb(Aabb& aabb) const
{
	CompoundShape::computeAabb(aabb);
}

Bool Frustum::insideFrustum(const CollisionShape& b) const
{
	for(const Plane& plane : m_planesW)
	{
		if(b.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}

void Frustum::transform(const Transform& trf)
{
	Transform trfa = m_trf.combineTransformations(trf);
	resetTransform(trfa);
}

void Frustum::resetTransform(const Transform& trf)
{
	m_trf = trf;
	update();
}

void Frustum::update()
{
	recalculate();

	// Transform derived
	onTransform();

	// Transform the planes
	for(U i = 0; i < m_planesL.getSize(); ++i)
	{
		m_planesW[i] = m_planesL[i].getTransformed(m_trf);
	}
}

PerspectiveFrustum::PerspectiveFrustum()
	: Frustum(FrustumType::PERSPECTIVE)
{
	addShape(&m_hull);
	m_hull.initStorage(&m_pointsW[0], m_pointsW.getSize());
}

PerspectiveFrustum& PerspectiveFrustum::operator=(const PerspectiveFrustum& b)
{
	Frustum::operator=(b);
	m_fovX = b.m_fovX;
	m_fovY = b.m_fovY;
	m_pointsL = b.m_pointsL;
	m_pointsW = b.m_pointsW;
	return *this;
}

void PerspectiveFrustum::onTransform()
{
	// Eye
	m_pointsW[0] = m_trf.getOrigin();

	for(U i = 1; i < 5; ++i)
	{
		m_pointsW[i] = m_trf.transform(m_pointsL[i - 1]);
	}
}

void PerspectiveFrustum::recalculate()
{
	// Planes
	//
	F32 c, s; // cos & sine

	sinCos(PI + m_fovX / 2.0, s, c);
	// right
	m_planesL[FrustumPlaneType::RIGHT] = Plane(Vec4(c, 0.0, s, 0.0), 0.0);
	// left
	m_planesL[FrustumPlaneType::LEFT] = Plane(Vec4(-c, 0.0, s, 0.0), 0.0);

	sinCos((PI + m_fovY) * 0.5, s, c);
	// bottom
	m_planesL[FrustumPlaneType::BOTTOM] = Plane(Vec4(0.0, s, c, 0.0), 0.0);
	// top
	m_planesL[FrustumPlaneType::TOP] = Plane(Vec4(0.0, -s, c, 0.0), 0.0);

	// near
	m_planesL[FrustumPlaneType::NEAR] = Plane(Vec4(0.0, 0.0, -1.0, 0.0), m_near);
	// far
	m_planesL[FrustumPlaneType::FAR] = Plane(Vec4(0.0, 0.0, 1.0, 0.0), -m_far);

	// Points
	//

	// This came from unprojecting. It works, don't touch it
	F32 x = m_far * tan(m_fovY / 2.0) * m_fovX / m_fovY;
	F32 y = tan(m_fovY / 2.0) * m_far;
	F32 z = -m_far;

	m_pointsL[0] = Vec4(x, y, z, 0.0); // top right
	m_pointsL[1] = Vec4(-x, y, z, 0.0); // top left
	m_pointsL[2] = Vec4(-x, -y, z, 0.0); // bot left
	m_pointsL[3] = Vec4(x, -y, z, 0.0); // bot right
}

Mat4 PerspectiveFrustum::calculateProjectionMatrix() const
{
	return Mat4::calculatePerspectiveProjectionMatrix(m_fovX, m_fovY, m_near, m_far);
}

OrthographicFrustum::OrthographicFrustum()
	: Frustum(FrustumType::ORTHOGRAPHIC)
{
	addShape(&m_obbW);
}

OrthographicFrustum& OrthographicFrustum::operator=(const OrthographicFrustum& b)
{
	Frustum::operator=(b);
	m_left = b.m_left;
	m_right = b.m_right;
	m_top = b.m_top;
	m_bottom = b.m_bottom;
	m_obbL = b.m_obbL;
	m_obbW = b.m_obbW;
	return *this;
}

Mat4 OrthographicFrustum::calculateProjectionMatrix() const
{
	return Mat4::calculateOrthographicProjectionMatrix(m_right, m_left, m_top, m_bottom, m_near, m_far);
}

void OrthographicFrustum::recalculate()
{
	ANKI_ASSERT(m_left < m_right && m_far > m_near && m_bottom < m_top);

	// Planes
	m_planesL[FrustumPlaneType::LEFT] = Plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), m_left);
	m_planesL[FrustumPlaneType::RIGHT] = Plane(Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -m_right);

	m_planesL[FrustumPlaneType::NEAR] = Plane(Vec4(0.0f, 0.0f, -1.0f, 0.0f), m_near);
	m_planesL[FrustumPlaneType::FAR] = Plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -m_far);
	m_planesL[FrustumPlaneType::TOP] = Plane(Vec4(0.0f, -1.0f, 0.0f, 0.0f), -m_top);
	m_planesL[FrustumPlaneType::BOTTOM] = Plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), m_bottom);

	// OBB
	Vec4 c((m_right + m_left) * 0.5f, (m_top + m_bottom) * 0.5f, -(m_far + m_near) * 0.5f, 0.0f);
	Vec4 e = Vec4(m_right, m_top, -m_far, 0.0f) - c;
	m_obbL = Obb(c, Mat3x4::getIdentity(), e);
}

void OrthographicFrustum::onTransform()
{
	m_obbW = m_obbL.getTransformed(m_trf);
}

} // end namespace anki
