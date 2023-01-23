// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Frustum.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

Frustum::Frustum()
{
	// Set some default values
	init(FrustumType::kPerspective, nullptr);
	for(U i = 0; i < m_maxLodDistances.getSize(); ++i)
	{
		const F32 dist = (m_common.m_far - m_common.m_near) / F32(kMaxLodCount + 1);
		m_maxLodDistances[i] = m_common.m_near + dist * F32(i + 1);
	}

	update(true, Transform::getIdentity());
}

Frustum::~Frustum()
{
	m_depthMap.destroy(*m_pool);
}

void Frustum::init(FrustumType type, HeapMemoryPool* pool)
{
	ANKI_ASSERT(type < FrustumType::kCount);
	m_pool = pool;
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

Bool Frustum::update(Bool worldTransformUpdated, const Transform& worldTransform)
{
	Bool updated = false;

	for(U32 i = kPrevMatrixHistory - 1; i != 0; --i)
	{
		m_prevViewProjMats[i] = m_prevViewProjMats[i - 1];
		m_prevViewMats[i] = m_prevViewMats[i - 1];
		m_prevProjMats[i] = m_prevProjMats[i - 1];
	}

	m_prevViewProjMats[0] = m_viewProjMat;
	m_prevViewMats[0] = m_viewMat;
	m_prevProjMats[0] = m_projMat;

	// Update the shape
	if(m_shapeMarkedForUpdate)
	{
		updated = true;

		if(m_frustumType == FrustumType::kPerspective)
		{
			m_projMat = Mat4::calculatePerspectiveProjectionMatrix(m_perspective.m_fovX, m_perspective.m_fovY,
																   m_perspective.m_near, m_perspective.m_far);

			computeEdgesOfFrustum(m_perspective.m_far, m_perspective.m_fovX, m_perspective.m_fovY,
								  &m_perspective.m_edgesL[0]);

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
			m_projMat = Mat4::calculateOrthographicProjectionMatrix(m_ortho.m_right, m_ortho.m_left, m_ortho.m_top,
																	m_ortho.m_bottom, m_ortho.m_near, m_ortho.m_far);

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
	if(worldTransformUpdated)
	{
		updated = true;
		m_viewMat = Mat3x4(worldTransform.getInverse());
	}

	// Fixup the misc data
	if(m_miscMarkedForUpdate)
	{
		updated = true;
		const F32 frustumFraction = (m_common.m_far - m_common.m_near) / 100.0f;

		for(U32 i = 0; i < m_shadowCascadeCount; ++i)
		{
			if(m_shadowCascadeDistances[i] <= m_common.m_near || m_shadowCascadeDistances[i] > m_common.m_far)
			{
				m_shadowCascadeDistances[i] =
					clamp(m_shadowCascadeDistances[i], m_common.m_near + kEpsilonf, m_common.m_far);
			}

			if(i != 0 && m_shadowCascadeDistances[i - 1] > m_shadowCascadeDistances[i])
			{
				m_shadowCascadeDistances[i] = m_shadowCascadeDistances[i - 1] + frustumFraction;
			}
		}

		for(U32 i = 0; i < m_maxLodDistances.getSize(); ++i)
		{
			if(m_maxLodDistances[i] <= m_common.m_near || m_maxLodDistances[i] > m_common.m_far)
			{
				m_maxLodDistances[i] = clamp(m_maxLodDistances[i], m_common.m_near + kEpsilonf, m_common.m_far);
			}

			if(i != 0 && m_maxLodDistances[i - 1] > m_maxLodDistances[i])
			{
				m_maxLodDistances[i] = m_maxLodDistances[i - 1] + frustumFraction;
			}
		}
	}

	// Updates that are affected by transform & shape updates
	if(updated)
	{
		m_viewProjMat = m_projMat * Mat4(m_viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m_shapeMarkedForUpdate = false;
		m_miscMarkedForUpdate = false;

		if(m_frustumType == FrustumType::kPerspective)
		{
			m_perspective.m_edgesW[0] = worldTransform.getOrigin();
			m_perspective.m_edgesW[1] = worldTransform.transform(m_perspective.m_edgesL[0]);
			m_perspective.m_edgesW[2] = worldTransform.transform(m_perspective.m_edgesL[1]);
			m_perspective.m_edgesW[3] = worldTransform.transform(m_perspective.m_edgesL[2]);
			m_perspective.m_edgesW[4] = worldTransform.transform(m_perspective.m_edgesL[3]);

			m_perspective.m_hull = ConvexHullShape(&m_perspective.m_edgesW[0], m_perspective.m_edgesW.getSize());
		}
		else
		{
			m_ortho.m_obbW = m_ortho.m_obbL.getTransformed(worldTransform);
		}

		for(FrustumPlaneType planeId : EnumIterable<FrustumPlaneType>())
		{
			m_viewPlanesW[planeId] = m_viewPlanesL[planeId].getTransformed(worldTransform);
		}
	}

	return updated;
}

void Frustum::setCoverageBuffer(F32* depths, U32 width, U32 height)
{
	const U32 elemCount = width * height;
	if(m_depthMap.getSize() != elemCount) [[unlikely]]
	{
		m_depthMap.resize(*m_pool, elemCount);
	}

	if(depths && elemCount > 0) [[likely]]
	{
		memcpy(m_depthMap.getBegin(), depths, elemCount * sizeof(F32));
	}
}

} // end namespace anki
