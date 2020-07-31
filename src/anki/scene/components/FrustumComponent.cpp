// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/components/SpatialComponent.h>

namespace anki
{

FrustumComponent::FrustumComponent(SceneNode* node, FrustumType frustumType)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
	, m_frustumType(frustumType)
{
	ANKI_ASSERT(&m_perspective.m_far == &m_ortho.m_far);
	ANKI_ASSERT(node);
	ANKI_ASSERT(frustumType < FrustumType::COUNT);

	// Set some default values
	if(frustumType == FrustumType::PERSPECTIVE)
	{
		setPerspective(0.1f, 100.0f, toRad(45.0f), toRad(45.0f));
	}
	else
	{
		setOrthographic(0.1f, 100.0f, 5.0f, -5.0f, 5.0f, -5.0f);
	}

	updateInternal();
}

FrustumComponent::~FrustumComponent()
{
	m_coverageBuff.m_depthMap.destroy(m_node->getAllocator());
}

Bool FrustumComponent::updateInternal()
{
	ANKI_ASSERT(m_frustumType != FrustumType::COUNT);

	Bool updated = false;
	m_prevViewProjMat = m_viewProjMat;

	// Update the shape
	if(m_shapeMarkedForUpdate)
	{
		updated = true;

		if(m_frustumType == FrustumType::PERSPECTIVE)
		{
			m_projMat = Mat4::calculatePerspectiveProjectionMatrix(m_perspective.m_fovX, m_perspective.m_fovY,
																   m_perspective.m_near, m_perspective.m_far);

			computeEdgesOfFrustum(m_perspective.m_far, m_perspective.m_fovY, m_perspective.m_fovY,
								  &m_perspective.m_edgesL[0]);

			// Planes
			F32 c, s; // cos & sine

			sinCos(PI + m_perspective.m_fovX / 2.0f, s, c);
			// right
			m_viewPlanesL[FrustumPlaneType::RIGHT] = Plane(Vec4(c, 0.0f, s, 0.0f), 0.0f);
			// left
			m_viewPlanesL[FrustumPlaneType::LEFT] = Plane(Vec4(-c, 0.0f, s, 0.0f), 0.0f);

			sinCos((PI + m_perspective.m_fovY) * 0.5f, s, c);
			// bottom
			m_viewPlanesL[FrustumPlaneType::BOTTOM] = Plane(Vec4(0.0f, s, c, 0.0f), 0.0f);
			// top
			m_viewPlanesL[FrustumPlaneType::TOP] = Plane(Vec4(0.0f, -s, c, 0.0f), 0.0f);

			// near
			m_viewPlanesL[FrustumPlaneType::NEAR] = Plane(Vec4(0.0f, 0.0f, -1.0, 0.0f), m_perspective.m_near);
			// far
			m_viewPlanesL[FrustumPlaneType::FAR] = Plane(Vec4(0.0f, 0.0f, 1.0, 0.0f), -m_perspective.m_far);
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
			m_viewPlanesL[FrustumPlaneType::LEFT] = Plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), m_ortho.m_left);
			m_viewPlanesL[FrustumPlaneType::RIGHT] = Plane(Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -m_ortho.m_right);
			m_viewPlanesL[FrustumPlaneType::NEAR] = Plane(Vec4(0.0f, 0.0f, -1.0f, 0.0f), m_ortho.m_near);
			m_viewPlanesL[FrustumPlaneType::FAR] = Plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -m_ortho.m_far);
			m_viewPlanesL[FrustumPlaneType::TOP] = Plane(Vec4(0.0f, -1.0f, 0.0f, 0.0f), -m_ortho.m_top);
			m_viewPlanesL[FrustumPlaneType::BOTTOM] = Plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), m_ortho.m_bottom);
		}
	}

	// Update transform related things
	if(m_trfMarkedForUpdate)
	{
		updated = true;
		m_viewMat = Mat4(m_trf.getInverse());
	}

	// Updates that are affected by transform & shape updates
	if(updated)
	{
		m_viewProjMat = m_projMat * m_viewMat;
		m_shapeMarkedForUpdate = false;
		m_trfMarkedForUpdate = false;

		if(m_frustumType == FrustumType::PERSPECTIVE)
		{
			m_perspective.m_edgesW[0] = m_trf.getOrigin();
			m_perspective.m_edgesW[1] = m_trf.transform(m_perspective.m_edgesL[0]);
			m_perspective.m_edgesW[2] = m_trf.transform(m_perspective.m_edgesL[1]);
			m_perspective.m_edgesW[3] = m_trf.transform(m_perspective.m_edgesL[2]);
			m_perspective.m_edgesW[4] = m_trf.transform(m_perspective.m_edgesL[3]);

			m_perspective.m_hull = ConvexHullShape(&m_perspective.m_edgesW[0], m_perspective.m_edgesW.getSize());
		}
		else
		{
			m_ortho.m_obbW = m_ortho.m_obbL.getTransformed(m_trf);
		}

		for(FrustumPlaneType planeId = FrustumPlaneType::FIRST; planeId < FrustumPlaneType::COUNT; ++planeId)
		{
			m_viewPlanesW[planeId] = m_viewPlanesL[planeId].getTransformed(m_trf);
		}
	}

	return updated;
}

void FrustumComponent::fillCoverageBufferCallback(void* userData, F32* depthValues, U32 width, U32 height)
{
	ANKI_ASSERT(userData && depthValues && width > 0 && height > 0);
	FrustumComponent& self = *static_cast<FrustumComponent*>(userData);

	self.m_coverageBuff.m_depthMap.destroy(self.m_node->getAllocator());
	self.m_coverageBuff.m_depthMap.create(self.m_node->getAllocator(), width * height);
	memcpy(&self.m_coverageBuff.m_depthMap[0], depthValues, self.m_coverageBuff.m_depthMap.getSizeInBytes());

	self.m_coverageBuff.m_depthMapWidth = width;
	self.m_coverageBuff.m_depthMapHeight = height;
}

void FrustumComponent::setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag bits)
{
	m_flags = FrustumComponentVisibilityTestFlag::NONE;
	m_flags |= bits;

#if ANKI_ENABLE_ASSERTS
	if(!!(m_flags & FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS)
	   || !!(m_flags & FrustumComponentVisibilityTestFlag::SHADOW_CASTERS))
	{
		if((m_flags & FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS)
		   == (m_flags & FrustumComponentVisibilityTestFlag::SHADOW_CASTERS))
		{
			ANKI_ASSERT(0 && "Cannot have them both");
		}
	}

	// TODO
#endif
}

} // end namespace anki
