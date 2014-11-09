// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/LightEvent.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
Error LightEvent::create(EventManager* manager, F32 startTime, F32 duration,
	Light* light, const LightEventData& data)
{
	Error err = Event::create(manager, startTime, duration, light);
	if(err) return err;

	*static_cast<LightEventData*>(this) = data;

	switch(light->getLightType())
	{
	case Light::Type::POINT:
		{
			PointLight* plight = static_cast<PointLight*>(light);
			m_originalRadius = plight->getRadius();
		}
		break;
	case Light::Type::SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	m_originalDiffColor = light->getDiffuseColor();
	m_originalSpecColor = light->getSpecularColor();

	return err;
}

//==============================================================================
Error LightEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	F32 factor = sin(getDelta(crntTime) * getPi<F32>());
	Light* light = static_cast<Light*>(getSceneNode());

	switch(light->getLightType())
	{
	case Light::Type::POINT:
		{
			PointLight* plight = static_cast<PointLight*>(light);
			plight->setRadius(
				factor * m_radiusMultiplier + m_originalRadius);
		}
		break;
	case Light::Type::SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	light->setDiffuseColor(
		m_originalDiffColor + factor * m_intensityMultiplier);
	light->setSpecularColor(
		m_originalSpecColor + factor * m_specularIntensityMultiplier);

	return ErrorCode::NONE;
}

} // end namespace anki
