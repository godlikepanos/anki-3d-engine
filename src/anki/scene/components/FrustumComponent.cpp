// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/FrustumComponent.h>

namespace anki
{

FrustumComponent::FrustumComponent(SceneNode* node, Frustum* frustum)
	: SceneComponent(CLASS_TYPE, node)
	, m_frustum(frustum)
	, m_flags(0)
{
	// WARNING: Never touch m_frustum in constructor
	ANKI_ASSERT(frustum);
	markShapeForUpdate();
	markTransformForUpdate();

	setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
}

FrustumComponent::~FrustumComponent()
{
	m_coverageBuff.m_depthMap.destroy(getAllocator());
}

Error FrustumComponent::update(Second, Second, Bool& updated)
{
	updated = false;
	m_prevViewProjMat = m_viewProjMat;

	if(m_flags.get(SHAPE_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_projMat = m_frustum->calculateProjectionMatrix();
	}

	if(m_flags.get(TRANSFORM_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_viewMat = Mat4(m_frustum->getTransform().getInverse());
	}

	if(updated)
	{
		m_viewProjMat = m_projMat * m_viewMat;
		m_flags.unset(SHAPE_MARKED_FOR_UPDATE | TRANSFORM_MARKED_FOR_UPDATE);
	}

	return Error::NONE;
}

} // end namespace anki
