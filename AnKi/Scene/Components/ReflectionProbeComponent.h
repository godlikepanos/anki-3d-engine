// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/Spatial.h>
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
		m_dirty = true;
	}

	Vec3 getBoxVolumeSize() const
	{
		return m_halfSize * 2.0f;
	}

	ANKI_INTERNAL WeakArray<Frustum> getFrustums()
	{
		return WeakArray<Frustum>(m_frustums);
	}

	ANKI_INTERNAL void setupReflectionProbeQueueElement(ReflectionProbeQueueElement& el) const
	{
		ANKI_ASSERT(!m_reflectionNeedsRefresh);
		ANKI_ASSERT(m_worldPos.x() != kMaxF32);
		el.m_worldPosition = m_worldPos;
		el.m_aabbMin = -m_halfSize + m_worldPos;
		el.m_aabbMax = m_halfSize + m_worldPos;
		ANKI_ASSERT(el.m_textureBindlessIndex != kMaxU32);
		el.m_textureBindlessIndex = m_reflectionTexBindlessIndex;
	}

	ANKI_INTERNAL void setupReflectionProbeQueueElementForRefresh(ReflectionProbeQueueElementForRefresh& el) const
	{
		ANKI_ASSERT(m_reflectionNeedsRefresh);
		el.m_worldPosition = m_worldPos;
		el.m_reflectionTexture = m_reflectionTex.get();
	}

	ANKI_INTERNAL Bool getReflectionNeedsRefresh() const
	{
		return m_reflectionNeedsRefresh;
	}

	ANKI_INTERNAL void setReflectionNeedsRefresh(Bool needsRefresh)
	{
		m_reflectionNeedsRefresh = needsRefresh;
	}

private:
	Vec3 m_worldPos = Vec3(kMaxF32);
	Vec3 m_halfSize = Vec3(1.0f);

	U32 m_gpuSceneOffset = kMaxU32;

	Spatial m_spatial;

	Array<Frustum, 6> m_frustums;

	TexturePtr m_reflectionTex;
	TextureViewPtr m_reflectionView;
	U32 m_reflectionTexBindlessIndex = kMaxU32;

	Bool m_dirty = true;
	Bool m_reflectionNeedsRefresh = true;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	void onDestroy(SceneNode& node);
};
/// @}

} // end namespace anki
