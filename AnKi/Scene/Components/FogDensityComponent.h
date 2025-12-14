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
		if(ANKI_EXPECT(type < FogDensityComponentShape::kCount) && type != m_type)
		{
			m_type = type;
			m_dirty = true;
		}
	}

	FogDensityComponentShape getShapeType() const
	{
		return m_type;
	}

	void setDensity(F32 density)
	{
		if(ANKI_EXPECT(density >= 0.0f) && m_density != density)
		{
			m_dirty = true;
			m_density = density;
		}
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

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
