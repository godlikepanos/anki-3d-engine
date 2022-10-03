// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/LightEvent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>

namespace anki {

Error LightEvent::init(Second startTime, Second duration, SceneNode* light)
{
	Event::init(startTime, duration);
	m_associatedNodes.emplaceBack(getMemoryPool(), light);

	LightComponent& lightc = light->getFirstComponentOfType<LightComponent>();

	switch(lightc.getLightComponentType())
	{
	case LightComponentType::kPoint:
		m_originalRadius = lightc.getRadius();
		break;
	case LightComponentType::kSpot:
		ANKI_ASSERT("TODO");
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	m_originalDiffColor = lightc.getDiffuseColor();

	return Error::kNone;
}

Error LightEvent::update([[maybe_unused]] Second prevUpdateTime, Second crntTime)
{
	const F32 freq = getRandomRange(m_freq - m_freqDeviation, m_freq + m_freqDeviation);

	F32 factor = F32(sin(crntTime * freq * kPi)) / 2.0f + 0.5f;
	LightComponent& lightc = m_associatedNodes[0]->getFirstComponentOfType<LightComponent>();

	// Update radius
	if(m_radiusMultiplier != 0.0)
	{
		switch(lightc.getLightComponentType())
		{
		case LightComponentType::kPoint:
			lightc.setRadius(m_originalRadius + factor * m_radiusMultiplier);
			break;
		case LightComponentType::kSpot:
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

		LensFlareComponent* lfc = m_associatedNodes[0]->tryGetFirstComponentOfType<LensFlareComponent>();

		if(lfc && lfc->getColorMultiplier().xyz() == lightc.getDiffuseColor().xyz())
		{
			lfc->setColorMultiplier(Vec4(outCol.xyz(), lfc->getColorMultiplier().w()));
		}

		lightc.setDiffuseColor(outCol);
	}

	return Error::kNone;
}

} // end namespace anki
