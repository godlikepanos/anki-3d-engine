// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Reflection probe component.
class ReflectionProbeComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::REFLECTION_PROBE;

	ReflectionProbeComponent(SceneNode* node)
		: SceneComponent(CLASS_TYPE, node)
	{
	}

	const Vec4& getPosition() const
	{
		return m_pos;
	}

	void setPosition(const Vec4& pos)
	{
		m_pos = pos.xyz0();
	}

	F32 getRadius() const
	{
		ANKI_ASSERT(m_radius > 0.0);
		return m_radius;
	}

	void setRadius(F32 radius)
	{
		ANKI_ASSERT(radius > 0.0);
		m_radius = radius;
	}

	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	void setMarkedForRendering(Bool render)
	{
		m_markedForRendering = render;
	}

	void setTextureArrayIndex(U idx)
	{
		m_textureArrayIndex = idx;
	}

	U getTextureArrayIndex() const
	{
		ANKI_ASSERT(m_textureArrayIndex < MAX_U16);
		return m_textureArrayIndex;
	}

	void setupReflectionProbeQueueElement(ReflectionProbeQueueElement& el) const
	{
		el.m_feedbackCallback = reflectionProbeQueueElementFeedbackCallback;
		el.m_userData = const_cast<ReflectionProbeComponent*>(this);
		el.m_uuid = getUuid();
		el.m_worldPosition = m_pos.xyz();
		el.m_radius = m_radius;
#if ANKI_EXTRA_CHECKS
		el.m_renderQueues = {{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
		el.m_textureArrayIndex = MAX_U32;
#endif
	}

private:
	Vec4 m_pos = Vec4(0.0);
	F32 m_radius = 0.0;
	Bool8 m_markedForRendering = false;

	U16 m_textureArrayIndex = MAX_U16; ///< Used by the renderer

	static void reflectionProbeQueueElementFeedbackCallback(Bool fillRenderQueuesOnNextFrame, void* userData)
	{
		ANKI_ASSERT(userData);
		static_cast<ReflectionProbeComponent*>(userData)->m_markedForRendering = fillRenderQueuesOnNextFrame;
	}
};
/// @}

} // end namespace anki
