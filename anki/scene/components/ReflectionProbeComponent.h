// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/collision/Aabb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Reflection probe component.
class ReflectionProbeComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ReflectionProbeComponent)

public:
	ReflectionProbeComponent(SceneNode* node);

	/// Set the local size of the probe volume.
	void setBoxVolumeSize(const Vec3& sizeXYZ)
	{
		m_halfSize = sizeXYZ / 2.0f;
		m_markedForUpdate = true;
	}

	Vec3 getBoxVolumeSize() const
	{
		return m_halfSize * 2.0f;
	}

	Vec3 getWorldPosition() const
	{
		return m_worldPos;
	}

	void setWorldPosition(const Vec3& pos)
	{
		m_worldPos = pos;
		m_markedForUpdate = true;
	}

	Aabb getAabbWorldSpace() const
	{
		return Aabb(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
	}

	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	void setDrawCallback(RenderQueueDrawCallback callback, const void* userData)
	{
		m_drawCallback = callback;
		m_drawCallbackUserData = userData;
	}

	void setupReflectionProbeQueueElement(ReflectionProbeQueueElement& el) const
	{
		el.m_feedbackCallback = reflectionProbeQueueElementFeedbackCallback;
		el.m_feedbackCallbackUserData = const_cast<ReflectionProbeComponent*>(this);
		el.m_uuid = m_uuid;
		el.m_worldPosition = m_worldPos;
		el.m_aabbMin = -m_halfSize + m_worldPos;
		el.m_aabbMax = m_halfSize + m_worldPos;
		el.m_textureArrayIndex = MAX_U32;
		el.m_debugDrawCallback = m_drawCallback;
		el.m_debugDrawCallbackUserData = m_drawCallbackUserData;
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = m_markedForUpdate;
		m_markedForUpdate = false;
		return Error::NONE;
	}

private:
	U64 m_uuid;
	RenderQueueDrawCallback m_drawCallback = nullptr;
	const void* m_drawCallbackUserData = nullptr;
	Vec3 m_worldPos = Vec3(0.0f);
	Vec3 m_halfSize = Vec3(1.0f);
	Bool m_markedForRendering : 1;
	Bool m_markedForUpdate : 1;

	static void reflectionProbeQueueElementFeedbackCallback(Bool fillRenderQueuesOnNextFrame, void* userData)
	{
		ANKI_ASSERT(userData);
		static_cast<ReflectionProbeComponent*>(userData)->m_markedForRendering = fillRenderQueuesOnNextFrame;
	}
};
/// @}

} // end namespace anki
