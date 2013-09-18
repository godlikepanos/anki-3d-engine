#include "anki/event/LightEvent.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
LightEvent::LightEvent(EventManager* manager, F32 startTime, F32 duration,
	U8 flags, Light* light, const LightEventData& data)
	: Event(manager, startTime, duration, light, flags)
{
	*static_cast<LightEventData*>(this) = data;

	switch(light->getLightType())
	{
	case Light::LT_POINT:
		{
			PointLight* plight = static_cast<PointLight*>(light);
			originalRadius = plight->getRadius();
		}
		break;
	case Light::LT_SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	originalDiffColor = light->getDiffuseColor();
	originalSpecColor = light->getSpecularColor();
}

//==============================================================================
void LightEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	F32 factor = sin(getDelta(crntTime) * getPi<F32>());

	switch(light->getLightType())
	{
	case Light::LT_POINT:
		{
			PointLight* plight = static_cast<PointLight*>(light);
			plight->setRadius(
				factor * radiusMultiplier + originalRadius);
		}
		break;
	case Light::LT_SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	light->setDiffuseColor(originalDiffColor + factor * intensityMultiplier);
	light->setSpecularColor(
		originalSpecColor + factor * specularIntensityMultiplier);
}

} // end namespace anki
