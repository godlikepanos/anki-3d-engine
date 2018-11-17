// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
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

	ReflectionProbeComponent(U64 uuid)
		: SceneComponent(CLASS_TYPE)
		, m_uuid(uuid)
	{
	}

	Vec4 getPosition() const
	{
		return m_pos.xyz0();
	}

	void setPosition(const Vec4& pos)
	{
		m_pos = pos.xyz();
	}

	void setBoundingBox(const Vec4& min, const Vec4& max)
	{
		m_aabbMin = min.xyz();
		m_aabbMax = max.xyz();
	}

	Vec4 getBoundingBoxMin() const
	{
		return m_aabbMin.xyz0();
	}

	Vec4 getBoundingBoxMax() const
	{
		return m_aabbMax.xyz0();
	}

	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	void setupReflectionProbeQueueElement(ReflectionProbeQueueElement& el) const
	{
		ANKI_ASSERT(m_aabbMin < m_aabbMax);
		ANKI_ASSERT(m_pos > m_aabbMin && m_pos < m_aabbMax);
		el.m_feedbackCallback = reflectionProbeQueueElementFeedbackCallback;
		el.m_userData = const_cast<ReflectionProbeComponent*>(this);
		el.m_uuid = m_uuid;
		el.m_worldPosition = m_pos;
		el.m_aabbMin = m_aabbMin;
		el.m_aabbMax = m_aabbMax;
		el.m_textureArrayIndex = MAX_U32;
		el.m_drawCallback = debugDrawCallback;
	}

private:
	U64 m_uuid;
	Vec3 m_pos = Vec3(0.0f);
	Vec3 m_aabbMin = Vec3(+1.0f);
	Vec3 m_aabbMax = Vec3(-1.0f);
	Bool8 m_markedForRendering = false;

	static void reflectionProbeQueueElementFeedbackCallback(Bool fillRenderQueuesOnNextFrame, void* userData)
	{
		ANKI_ASSERT(userData);
		static_cast<ReflectionProbeComponent*>(userData)->m_markedForRendering = fillRenderQueuesOnNextFrame;
	}

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
