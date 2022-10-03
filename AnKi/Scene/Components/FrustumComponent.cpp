// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(FrustumComponent)

FrustumComponent::FrustumComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_shapeMarkedForUpdate(true)
	, m_trfMarkedForUpdate(true)
{
	ANKI_ASSERT(&m_perspective.m_far == &m_ortho.m_far);
	ANKI_ASSERT(node);

	// Set some default values
	setFrustumType(FrustumType::kPerspective);
	updateInternal();
}

FrustumComponent::~FrustumComponent()
{
	m_coverageBuff.m_depthMap.destroy(m_node->getMemoryPool());
}

Bool FrustumComponent::updateInternal()
{
	ANKI_ASSERT(m_frustumType != FrustumType::kCount);

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
	if(m_trfMarkedForUpdate)
	{
		updated = true;
		m_viewMat = Mat3x4(m_trf.getInverse());
	}

	// Updates that are affected by transform & shape updates
	if(updated)
	{
		m_viewProjMat = m_projMat * Mat4(m_viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m_shapeMarkedForUpdate = false;
		m_trfMarkedForUpdate = false;

		if(m_frustumType == FrustumType::kPerspective)
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

		for(FrustumPlaneType planeId : EnumIterable<FrustumPlaneType>())
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

	self.m_coverageBuff.m_depthMap.destroy(self.m_node->getMemoryPool());
	self.m_coverageBuff.m_depthMap.create(self.m_node->getMemoryPool(), width * height);
	memcpy(&self.m_coverageBuff.m_depthMap[0], depthValues, self.m_coverageBuff.m_depthMap.getSizeInBytes());

	self.m_coverageBuff.m_depthMapWidth = width;
	self.m_coverageBuff.m_depthMapHeight = height;
}

void FrustumComponent::setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag bits)
{
	m_flags = FrustumComponentVisibilityTestFlag::kNone;
	m_flags |= bits;

#if ANKI_ENABLE_ASSERTIONS
	if(!!(m_flags & FrustumComponentVisibilityTestFlag::kRenderComponents)
	   || !!(m_flags & FrustumComponentVisibilityTestFlag::kShadowCasterRenderComponents))
	{
		if((m_flags & FrustumComponentVisibilityTestFlag::kRenderComponents)
		   == (m_flags & FrustumComponentVisibilityTestFlag::kShadowCasterRenderComponents))
		{
			ANKI_ASSERT(0 && "Cannot have them both");
		}
	}

	// TODO
#endif
}

} // end namespace anki
