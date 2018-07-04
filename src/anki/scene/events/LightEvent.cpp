// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/events/LightEvent.h>
#include <anki/scene/components/LightComponent.h>
#include <anki/scene/components/LensFlareComponent.h>

namespace anki
{

Error LightEvent::init(Second startTime, Second duration, SceneNode* light)
{
	Event::init(startTime, duration);
	m_associatedNodes.emplaceBack(getAllocator(), light);

	LightComponent& lightc = light->getComponent<LightComponent>();

	switch(lightc.getLightComponentType())
	{
	case LightComponentType::POINT:
		m_originalRadius = lightc.getRadius();
		break;
	case LightComponentType::SPOT:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	m_originalDiffColor = lightc.getDiffuseColor();

	return Error::NONE;
}

Error LightEvent::update(Second prevUpdateTime, Second crntTime)
{
	F32 freq = randRange(m_freq - m_freqDeviation, m_freq + m_freqDeviation);

	F32 factor = sin(crntTime * freq * PI) / 2.0 + 0.5;
	LightComponent& lightc = m_associatedNodes[0]->getComponent<LightComponent>();

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

		LensFlareComponent* lfc = m_associatedNodes[0]->tryGetComponent<LensFlareComponent>();

		if(lfc && lfc->getColorMultiplier().xyz() == lightc.getDiffuseColor().xyz())
		{
			lfc->setColorMultiplier(Vec4(outCol.xyz(), lfc->getColorMultiplier().w()));
		}

		lightc.setDiffuseColor(outCol);
	}

	return Error::NONE;
}

} // end namespace anki
