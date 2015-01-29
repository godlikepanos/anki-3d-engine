// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/FrustumComponent.h"
#include "anki/scene/Visibility.h"

namespace anki {

//==============================================================================
void FrustumComponent::setVisibilityTestResults(VisibilityTestResults* visible)
{
	ANKI_ASSERT(m_visible == nullptr);
	m_visible = visible;

	m_stats.m_renderablesCount = visible->getRenderablesCount();
	m_stats.m_lightsCount = visible->getLightsCount();
}

//==============================================================================
Error FrustumComponent::update(SceneNode& node, F32, F32, Bool& updated)
{
	updated = false;

	if(m_flags & SHAPE_MARKED_FOR_UPDATE)
	{
		updated = true;
		m_pm = m_frustum->calculateProjectionMatrix();
	}

	if(m_flags & TRANSFORM_MARKED_FOR_UPDATE)
	{
		updated = true;
		m_vm = Mat4(m_frustum->getTransform().getInverse());
	}

	if(updated)
	{
		m_vpm = m_pm * m_vm;
		m_flags = 0;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
