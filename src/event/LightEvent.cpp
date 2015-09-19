// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/LightEvent.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
Error LightEvent::init(F32 startTime, F32 duration, SceneNode* light)
{
	Event::init(startTime, duration, light);

	LightComponent& lightc = light->getComponent<LightComponent>();

	switch(lightc.getLightType())
	{
	case LightComponent::LightType::POINT:
		{
			m_originalRadius = lightc.getRadius();
		}
		break;
	case LightComponent::LightType::SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	m_originalDiffColor = lightc.getDiffuseColor();
	m_originalSpecColor = lightc.getSpecularColor();

	return ErrorCode::NONE;
}

//==============================================================================
Error LightEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	F32 freq = randRange(m_freq - m_freqDeviation, m_freq + m_freqDeviation);

	F32 factor = sin(crntTime * freq * getPi<F32>()) / 2.0 + 0.5;
	LightComponent& lightc = getSceneNode()->getComponent<LightComponent>();

	switch(lightc.getLightType())
	{
	case LightComponent::LightType::POINT:
		lightc.setRadius(m_originalRadius + factor * m_radiusMultiplier);
		break;
	case LightComponent::LightType::SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	lightc.setDiffuseColor(
		m_originalDiffColor + factor * m_intensityMultiplier);
	lightc.setSpecularColor(
		m_originalSpecColor + factor * m_specularIntensityMultiplier);

	return ErrorCode::NONE;
}

} // end namespace anki
