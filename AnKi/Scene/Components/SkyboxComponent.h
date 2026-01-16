// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Math.h>

namespace anki {

enum class SkyboxComponentType : U8
{
	kSolidColor,
	kImage2D,
	kGenerated, // The Renderer will generate the texture based on sun positions etc etc

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SkyboxComponentType)

inline constexpr Array<const Char*, U32(SkyboxComponentType::kCount)> kSkyboxComponentTypeNames = {"Solid Color", "2D Image", "Generated"};

// It controls the skybox and atmoshpere. See the GpuSceneSkybox for comments.
class SkyboxComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SkyboxComponent)

public:
	SkyboxComponent(const SceneComponentInitInfo& init)
		: SceneComponent(kClassType, init)
	{
	}

	~SkyboxComponent()
	{
	}

	SkyboxComponent& setSkyboxComponentType(SkyboxComponentType type)
	{
		m_type = type;
		return *this;
	}

	SkyboxComponentType getSkyboxComponentType() const
	{
		return m_type;
	}

	SkyboxComponent& setSkySolidColor(Vec3 color)
	{
		m_sky.m_color = color.max(Vec3(0.0f));
		return *this;
	}

	Vec3 getSkySolidColor() const
	{
		return m_sky.m_color;
	}

	SkyboxComponent& setSkyImageFilename(CString filename);

	CString getSkyImageFilename() const
	{
		if(m_sky.m_image)
		{
			return m_sky.m_image->getFilename();
		}
		else
		{
			return "*Error*";
		}
	}

	Bool hasSkyImageFilename() const
	{
		return !!m_sky.m_image;
	}

	SkyboxComponent& setSkyImageColorScale(Vec3 s)
	{
		m_sky.m_imageColorScale = s.max(Vec3(0.0f));
		return *this;
	}

	const Vec3& getSkyImageColorScale() const
	{
		return m_sky.m_imageColorScale;
	}

	SkyboxComponent& setSkyImageColorBias(Vec3 s)
	{
		m_sky.m_imageColorBias = s.max(Vec3(0.0f));
		return *this;
	}

	const Vec3& getSkyImageColorBias() const
	{
		return m_sky.m_imageColorBias;
	}

	SkyboxComponent& setMinFogDensity(F32 density)
	{
		m_fog.m_minDensity = clamp(density, 0.0f, 100.0f);
		return *this;
	}

	F32 getMinFogDensity() const
	{
		return m_fog.m_minDensity;
	}

	SkyboxComponent& setMaxFogDensity(F32 density)
	{
		m_fog.m_maxDensity = clamp(density, 0.0f, 100.0f);
		return *this;
	}

	F32 getMaxFogDensity() const
	{
		return m_fog.m_maxDensity;
	}

	// The height (units) where fog density is getMinFogDensity().
	SkyboxComponent& setHeightOfMinFogDensity(F32 height)
	{
		m_fog.m_heightOfMinDensity = height;
		return *this;
	}

	F32 getHeightOfMinFogDensity() const
	{
		return m_fog.m_heightOfMinDensity;
	}

	// The height (units) where fog density is getMaxFogDensity().
	SkyboxComponent& setHeightOfMaxFogDensity(F32 height)
	{
		m_fog.m_heightOfMaxDensity = height;
		return *this;
	}

	F32 getHeightOfMaxFogDensity() const
	{
		return m_fog.m_heightOfMaxDensity;
	}

	SkyboxComponent& setFogScatteringCoefficient(F32 coeff)
	{
		m_fog.m_scatteringCoeff = coeff;
		return *this;
	}

	F32 getFogScatteringCoefficient() const
	{
		return m_fog.m_scatteringCoeff;
	}

	SkyboxComponent& setFogAbsorptionCoefficient(F32 coeff)
	{
		m_fog.m_absorptionCoeff = coeff;
		return *this;
	}

	F32 getFogAbsorptionCoefficient() const
	{
		return m_fog.m_absorptionCoeff;
	}

	SkyboxComponent& setFogDiffuseColor(Vec3 color)
	{
		m_fog.m_diffuseColor = color.max(Vec3(0.0f));
		return *this;
	}

	const Vec3& getFogDiffuseColor() const
	{
		return m_fog.m_diffuseColor;
	}

	ANKI_INTERNAL Texture& getSkyTexture() const
	{
		ANKI_ASSERT(m_type == SkyboxComponentType::kImage2D);
		return m_sky.m_image->getTexture();
	}

private:
	SkyboxComponentType m_type = SkyboxComponentType::kSolidColor;

	class
	{
	public:
		Vec3 m_color = Vec3(0.0f, 0.0f, 0.5f);
		ImageResourcePtr m_image;
		Vec3 m_imageColorScale = Vec3(1.0f);
		Vec3 m_imageColorBias = Vec3(0.0f);
	} m_sky;

	class
	{
	public:
		F32 m_minDensity = 0.0f;
		F32 m_maxDensity = 0.9f;
		F32 m_heightOfMinDensity = 20.0f; // The height (meters) where fog density is max.
		F32 m_heightOfMaxDensity = 0.0f; // The height (meters) where fog density is the min value.
		F32 m_scatteringCoeff = 0.01f;
		F32 m_absorptionCoeff = 0.02f;
		Vec3 m_diffuseColor = Vec3(1.0f);
	} m_fog;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
