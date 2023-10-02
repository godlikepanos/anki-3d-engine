// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Frustum.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

Array<Mat3x4, 6> Frustum::m_omnidirectionalRotations = {Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, -kPi / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi))),
														Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, kPi / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi))),
														Mat3x4(Vec3(0.0f), Mat3(Euler(kPi / 2.0f, 0.0f, 0.0f))),
														Mat3x4(Vec3(0.0f), Mat3(Euler(-kPi / 2.0f, 0.0f, 0.0f))),
														Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, kPi, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi))),
														Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, 0.0f, kPi)))};

Frustum::Frustum()
{
	// Set some default values
	init(FrustumType::kPerspective);
	update();
}

Frustum::~Frustum()
{
}

void Frustum::init(FrustumType type)
{
	ANKI_ASSERT(type < FrustumType::kCount);
	m_frustumType = type;
	setNear(kDefaultNear);
	setFar(kDefaultFar);
	if(m_frustumType == FrustumType::kPerspective)
	{
		setFovX(kDefaultFovAngle);
		setFovY(kDefaultFovAngle);
	}
	else
	{
		setLeft(-5.0f);
		setRight(5.0f);
		setBottom(-1.0f);
		setTop(1.0f);
	}
}

Bool Frustum::update()
{
	Bool updated = false;

	// Update the shape
	if(m_shapeDirty)
	{
		updated = true;

		if(m_frustumType == FrustumType::kPerspective)
		{
			m_projMat =
				Mat4::calculatePerspectiveProjectionMatrix(m_perspective.m_fovX, m_perspective.m_fovY, m_perspective.m_near, m_perspective.m_far);

			computeEdgesOfFrustum(m_perspective.m_far, m_perspective.m_fovX, m_perspective.m_fovY, &m_perspective.m_edgesL[0]);

			// Planes
			F32 c, s; // cos & sine

			sinCos(kPi + m_perspective.m_fovX / 2.0f, s, c);
			// right
			m_viewPlanesL[FrustumPlaneType::kRight] = Plane(Vec4(c, 0.0f, s, 0.0f), 0.0f);
			// left
			m_viewPlanesL[FrustumPlaneType::kLeft] = Plane(Vec4(-c, 0.0f, s, 0.0f), 0.0f);

			sinCos((kPi + m_perspective.m_fovY) * 0.5f, s, c);
			// bottom
			m_viewPlanesL[FrustumPlaneType::kBottom] = Plane(Vec4(0.0f, s, c, 0.0f), 0.0f);
			// top
			m_viewPlanesL[FrustumPlaneType::kTop] = Plane(Vec4(0.0f, -s, c, 0.0f), 0.0f);

			// near
			m_viewPlanesL[FrustumPlaneType::kNear] = Plane(Vec4(0.0f, 0.0f, -1.0, 0.0f), m_perspective.m_near);
			// far
			m_viewPlanesL[FrustumPlaneType::kFar] = Plane(Vec4(0.0f, 0.0f, 1.0, 0.0f), -m_perspective.m_far);
		}
		else
		{
			m_projMat = Mat4::calculateOrthographicProjectionMatrix(m_ortho.m_right, m_ortho.m_left, m_ortho.m_top, m_ortho.m_bottom, m_ortho.m_near,
																	m_ortho.m_far);

			// OBB
			const Vec4 c((m_ortho.m_right + m_ortho.m_left) * 0.5f, (m_ortho.m_top + m_ortho.m_bottom) * 0.5f,
						 -(m_ortho.m_far + m_ortho.m_near) * 0.5f, 0.0f);
			const Vec4 e = Vec4(m_ortho.m_right, m_ortho.m_top, -m_ortho.m_far, 0.0f) - c;

			m_ortho.m_obbL = Obb(c, Mat3x4::getIdentity(), e);

			// Planes
			m_viewPlanesL[FrustumPlaneType::kLeft] = Plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), m_ortho.m_left);
			m_viewPlanesL[FrustumPlaneType::kRight] = Plane(Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -m_ortho.m_right);
			m_viewPlanesL[FrustumPlaneType::kNear] = Plane(Vec4(0.0f, 0.0f, -1.0f, 0.0f), m_ortho.m_near);
			m_viewPlanesL[FrustumPlaneType::kFar] = Plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -m_ortho.m_far);
			m_viewPlanesL[FrustumPlaneType::kTop] = Plane(Vec4(0.0f, -1.0f, 0.0f, 0.0f), -m_ortho.m_top);
			m_viewPlanesL[FrustumPlaneType::kBottom] = Plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), m_ortho.m_bottom);
		}
	}

	// Update transform related things
	if(m_worldTransformDirty)
	{
		updated = true;
		m_viewMat = Mat3x4(m_worldTransform.getInverse());
	}

	// Updates that are affected by transform & shape updates
	if(updated)
	{
		m_shapeDirty = false;
		m_worldTransformDirty = false;

		m_viewProjMat = m_projMat * Mat4(m_viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));

		if(m_frustumType == FrustumType::kPerspective)
		{
			m_perspective.m_edgesW[0] = m_worldTransform.getOrigin();
			m_perspective.m_edgesW[1] = m_worldTransform.transform(m_perspective.m_edgesL[0].xyz0());
			m_perspective.m_edgesW[2] = m_worldTransform.transform(m_perspective.m_edgesL[1].xyz0());
			m_perspective.m_edgesW[3] = m_worldTransform.transform(m_perspective.m_edgesL[2].xyz0());
			m_perspective.m_edgesW[4] = m_worldTransform.transform(m_perspective.m_edgesL[3].xyz0());

			m_perspective.m_hull = ConvexHullShape(&m_perspective.m_edgesW[0], m_perspective.m_edgesW.getSize());
		}
		else
		{
			m_ortho.m_obbW = m_ortho.m_obbL.getTransformed(m_worldTransform);
		}

		for(FrustumPlaneType planeId : EnumIterable<FrustumPlaneType>())
		{
			m_viewPlanesW[planeId] = m_viewPlanesL[planeId].getTransformed(m_worldTransform);
		}
	}

	return updated;
}

} // end namespace anki
