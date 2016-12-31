// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/LightEvent.h>
#include <anki/scene/Light.h>
#include <anki/scene/LensFlareComponent.h>

namespace anki
{

Error LightEvent::init(F32 startTime, F32 duration, SceneNode* light)
{
	Event::init(startTime, duration, light);

	LightComponent& lightc = light->getComponent<LightComponent>();

	switch(lightc.getLightComponentType())
	{
	case LightComponentType::POINT:
	{
		m_originalRadius = lightc.getRadius();
	}
	break;
	case LightComponentType::SPOT:
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

Error LightEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	F32 freq = randRange(m_freq - m_freqDeviation, m_freq + m_freqDeviation);

	F32 factor = sin(crntTime * freq * PI) / 2.0 + 0.5;
	LightComponent& lightc = getSceneNode()->getComponent<LightComponent>();

	// Update radius
	if(m_radiusMultiplier != 0.0)
	{
		switch(lightc.getLightComponentType())
		{
		case LightComponentType::POINT:
			lightc.setRadius(m_originalRadius + factor * m_radiusMultiplier);
			break;
		case LightComponentType::SPOT:
			ANKI_ASSERT("TODO");
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	// Update the color and the lens flare's color if they are the same
	if(m_intensityMultiplier != Vec4(0.0))
	{
		Vec4 outCol = m_originalDiffColor + factor * m_intensityMultiplier;

		LensFlareComponent* lfc = getSceneNode()->tryGetComponent<LensFlareComponent>();

		if(lfc && lfc->getColorMultiplier().xyz() == lightc.getDiffuseColor().xyz())
		{
			lfc->setColorMultiplier(Vec4(outCol.xyz(), lfc->getColorMultiplier().w()));
		}

		lightc.setDiffuseColor(outCol);
	}

	lightc.setSpecularColor(m_originalSpecColor + factor * m_specularIntensityMultiplier);

	return ErrorCode::NONE;
}

} // end namespace anki
