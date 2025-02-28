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

	ANKI_INTERNAL Vec3 getBoxVolumeSize() const
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

	ANKI_INTERNAL Vec3 getWorldPosition() const
	{
		ANKI_ASSERT(m_worldPos.x() != kMaxF32);
		return m_worldPos;
	}

	/// The radius around the probe's center that can infuence the rendering of the env texture.
	ANKI_INTERNAL F32 getRenderRadius() const;

	ANKI_INTERNAL F32 getShadowsRenderRadius() const;

	ANKI_INTERNAL Texture& getReflectionTexture() const
	{
		return *m_reflectionTex;
	}

	ANKI_INTERNAL const GpuSceneArrays::ReflectionProbe::Allocation& getGpuSceneAllocation() const
	{
		return m_gpuSceneProbe;
	}

private:
	Vec3 m_worldPos = Vec3(kMaxF32);
	Vec3 m_halfSize = Vec3(1.0f);

	GpuSceneArrays::ReflectionProbe::Allocation m_gpuSceneProbe;

	TexturePtr m_reflectionTex;
	U32 m_reflectionTexBindlessIndex = kMaxU32;

	Bool m_dirty = true;
	Bool m_reflectionNeedsRefresh = true;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
