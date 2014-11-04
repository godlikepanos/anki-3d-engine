// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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

} // end namespace anki
