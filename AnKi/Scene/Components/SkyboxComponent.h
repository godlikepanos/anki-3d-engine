// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Math.h>

namespace anki {

// Forward
class SkyboxQueueElement;

/// @addtogroup scene
/// @{

/// @memberof SkyboxComponent
enum class SkyboxType : U8
{
	kSolidColor,
	kImage2D
};

/// Skybox config.
class SkyboxComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SkyboxComponent)

public:
	SkyboxComponent(SceneNode* node);

	~SkyboxComponent();

	void setSolidColor(const Vec3& color)
	{
		m_type = SkyboxType::kSolidColor;
		m_color = color.max(Vec3(0.0f));
	}

	void setImage(CString filename);

	void setMinFogDensity(F32 density)
	{
		m_fog.m_minDensity = clamp(density, 0.0f, 100.0f);
	}

	F32 getMinFogDensity() const
	{
		return m_fog.m_minDensity;
	}

	void setMaxFogDensity(F32 density)
	{
		m_fog.m_maxDensity = clamp(density, 0.0f, 100.0f);
	}

	F32 getMaxFogDensity() const
	{
		return m_fog.m_maxDensity;
	}

	/// The height (units) where fog density is getMinFogDensity().
	void setHeightOfMinFogDensity(F32 height)
	{
		m_fog.m_heightOfMinDensity = height;
	}

	F32 getHeightOfMinFogDensity() const
	{
		return m_fog.m_heightOfMinDensity;
	}

	/// The height (units) where fog density is getMaxFogDensity().
	void setHeightOfMaxFogDensity(F32 height)
	{
		m_fog.m_heightOfMaxDensity = height;
	}

	F32 getHeightOfMaxFogDensity() const
	{
		return m_fog.m_heightOfMaxDensity;
	}

	void setFogScatteringCoefficient(F32 coeff)
	{
		m_fog.m_scatteringCoeff = coeff;
	}

	F32 getFogScatteringCoefficient() const
	{
		return m_fog.m_scatteringCoeff;
	}

	void setFogAbsorptionCoefficient(F32 coeff)
	{
		m_fog.m_absorptionCoeff = coeff;
	}

	F32 getFogAbsorptionCoefficient() const
	{
		return m_fog.m_absorptionCoeff;
	}

	void setFogDiffuseColor(const Vec3& color)
	{
		m_fog.m_diffuseColor = color;
	}

	const Vec3& getFogDiffuseColor() const
	{
		return m_fog.m_diffuseColor;
	}

	void setupSkyboxQueueElement(SkyboxQueueElement& queueElement) const;

private:
	SceneNode* m_node;
	SkyboxType m_type = SkyboxType::kSolidColor;
	Vec3 m_color = Vec3(0.0f, 0.0f, 0.5f);
	ImageResourcePtr m_image;

	// Fog
	class
	{
	public:
		F32 m_minDensity = 0.0f;
		F32 m_maxDensity = 0.9f;
		F32 m_heightOfMinDensity = 20.0f; ///< The height (meters) where fog density is max.
		F32 m_heightOfMaxDensity = 0.0f; ///< The height (meters) where fog density is the min value.
		F32 m_scatteringCoeff = 0.01f;
		F32 m_absorptionCoeff = 0.02f;
		Vec3 m_diffuseColor = Vec3(1.0f);
	} m_fog;
};
/// @}

} // end namespace anki
