// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Collision/Aabb.h>

namespace anki {

/// @addtogroup scene
/// @{

inline NumericCVar<U32> g_reflectionProbeResolutionCVar("Scene", "ReflectionProbeResolution", 128, 8, 2048,
														"The resolution of the reflection probe's reflection");

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
		m_reflectionNeedsRefresh = true;
	}

	Vec3 getBoxVolumeSize() const
	{
		return m_halfSize * 2.0f;
	}

	ANKI_INTERNAL Bool getEnvironmentTextureNeedsRefresh() const
	{
		return m_reflectionNeedsRefresh;
	}

	ANKI_INTERNAL void setEnvironmentTextureAsRefreshed()
	{
		m_reflectionNeedsRefresh = false;
		m_dirty = true; // To force update of the gpu scene
	}

	U32 getUuid() const
	{
		return m_uuid;
	}

	Vec3 getWorldPosition() const
	{
		ANKI_ASSERT(m_worldPos.x() != kMaxF32);
		return m_worldPos;
	}

	/// The radius around the probe's center that can infuence the rendering of the env texture.
	F32 getRenderRadius() const;

	F32 getShadowsRenderRadius() const;

	Texture& getReflectionTexture() const
	{
		return *m_reflectionTex;
	}

	const GpuSceneArrays::ReflectionProbe::Allocation& getGpuSceneAllocation() const
	{
		return m_gpuSceneProbe;
	}

private:
	Vec3 m_worldPos = Vec3(kMaxF32);
	Vec3 m_halfSize = Vec3(1.0f);

	GpuSceneArrays::ReflectionProbe::Allocation m_gpuSceneProbe;

	TexturePtr m_reflectionTex;
	U32 m_reflectionTexBindlessIndex = kMaxU32;
	U32 m_uuid = 0;

	Bool m_dirty = true;
	Bool m_reflectionNeedsRefresh = true;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
