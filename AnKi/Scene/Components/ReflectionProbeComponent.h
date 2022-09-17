// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Collision/Aabb.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Reflection probe component.
class ReflectionProbeComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ReflectionProbeComponent)

public:
	ReflectionProbeComponent(SceneNode* node);

	~ReflectionProbeComponent();

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

	void setupReflectionProbeQueueElement(ReflectionProbeQueueElement& el) const
	{
		el.m_feedbackCallback = reflectionProbeQueueElementFeedbackCallback;
		el.m_feedbackCallbackUserData = const_cast<ReflectionProbeComponent*>(this);
		el.m_uuid = m_uuid;
		el.m_worldPosition = m_worldPos;
		el.m_aabbMin = -m_halfSize + m_worldPos;
		el.m_aabbMax = m_halfSize + m_worldPos;
		el.m_textureArrayIndex = kMaxU32;
		el.m_debugDrawCallback = [](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			ANKI_ASSERT(userData.getSize() == 1);
			static_cast<const ReflectionProbeComponent*>(userData[0])->draw(ctx);
		};
		el.m_debugDrawCallbackUserData = this;
	}

	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = m_markedForUpdate;
		m_markedForUpdate = false;
		return Error::kNone;
	}

private:
	SceneNode* m_node = nullptr;
	U64 m_uuid = 0;
	Vec3 m_worldPos = Vec3(0.0f);
	Vec3 m_halfSize = Vec3(1.0f);
	Bool m_markedForRendering : 1;
	Bool m_markedForUpdate : 1;

	ImageResourcePtr m_debugImage;

	static void reflectionProbeQueueElementFeedbackCallback(Bool fillRenderQueuesOnNextFrame, void* userData)
	{
		ANKI_ASSERT(userData);
		static_cast<ReflectionProbeComponent*>(userData)->m_markedForRendering = fillRenderQueuesOnNextFrame;
	}

	void draw(RenderQueueDrawContext& ctx) const;
};
/// @}

} // end namespace anki
