// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/LightEvent.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
Error LightEvent::create(EventManager* manager, F32 startTime, F32 duration,
	Light* light)
{
	Error err = Event::create(manager, startTime, duration, light);
	if(err) return err;
	
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

	return err;
}

//==============================================================================
Error LightEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	F32 factor = sin(getDelta(crntTime) * getPi<F32>());
	LightComponent& lightc = getSceneNode()->getComponent<LightComponent>();

	switch(lightc.getLightType())
	{
	case LightComponent::LightType::POINT:
		{
			lightc.setRadius(
				factor * m_radiusMultiplier + m_originalRadius);
		}
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
