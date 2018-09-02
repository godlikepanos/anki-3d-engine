// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/RenderQueue.h>

namespace anki
{

U RenderQueue::countAllRenderables() const
{
	U drawableCount = 0;
	drawableCount += m_earlyZRenderables.getSize();
	drawableCount += m_renderables.getSize();
	drawableCount += m_forwardShadingRenderables.getSize();

	for(const SpotLightQueueElement* slight : m_shadowSpotLights)
	{
		if(slight->m_shadowRenderQueue)
		{
			drawableCount += slight->m_shadowRenderQueue->countAllRenderables();
		}
	}

	for(const PointLightQueueElement* plight : m_shadowPointLights)
	{
		for(U i = 0; i < 6; ++i)
		{
			if(plight->m_shadowRenderQueues[i])
			{
				drawableCount += plight->m_shadowRenderQueues[i]->countAllRenderables();
			}
		}
	}

	for(const ReflectionProbeQueueElement& probe : m_reflectionProbes)
	{
		for(U i = 0; i < 6; ++i)
		{
			if(probe.m_renderQueues[i])
			{
				drawableCount += probe.m_renderQueues[i]->countAllRenderables();
			}
		}
	}

	return drawableCount;
}

} // end namespace anki