// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Sphere.h>

namespace anki {

/// @addtogroup scene
/// @{

/// @memberof FogDensityComponent
enum class FogDensityComponentShape : U8
{
	kSphere,
	kBox,
	kCount
};

/// Fog density component. Controls the fog density.
class FogDensityComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(FogDensityComponent)

public:
	static constexpr F32 kMinShapeSize = 1.0_cm;

	FogDensityComponent(SceneNode* node);

	~FogDensityComponent();

	void setShapeType(FogDensityComponentShape type)
	{
		if(type != m_type)
		{
			m_type = type;
			m_dirty = true;
		}
	}

	FogDensityComponentShape getShapeType() const
	{
		return m_type;
	}

	void setDensity(F32 d)
	{
		ANKI_ASSERT(d >= 0.0f);
		m_dirty = true;
		m_density = d;
	}

	F32 getDensity() const
	{
		return m_density;
	}

private:
	GpuSceneArrays::FogDensityVolume::Allocation m_gpuSceneVolume;

	F32 m_density = 1.0f;

	FogDensityComponentShape m_type = FogDensityComponentShape::kSphere;

	Bool m_dirty = true;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;
};

} // end namespace anki
