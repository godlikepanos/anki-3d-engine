// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup scene
/// @{

enum class LightComponentType : U8
{
	POINT,
	SPOT,
	COUNT
};

/// Light component. It's a dummy component used to identify lights
class LightComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::LIGHT;

	/// The near plane on the shadow map frustums.
	/// WARNING: If you change here update the shaders.
	static constexpr F32 FRUSTUM_NEAR_PLANE = 0.1 / 4.0;

	LightComponent(SceneNode* node, LightComponentType type);

	LightComponentType getLightComponentType() const
	{
		return m_type;
	}

	const Vec4& getDiffuseColor() const
	{
		return m_diffColor;
	}

	void setDiffuseColor(const Vec4& x)
	{
		m_diffColor = x;
	}

	const Vec4& getSpecularColor() const
	{
		return m_specColor;
	}

	void setSpecularColor(const Vec4& x)
	{
		m_specColor = x;
	}

	void setRadius(F32 x)
	{
		m_radius = x;
		m_dirty = true;
	}

	F32 getRadius() const
	{
		return m_radius;
	}

	void setDistance(F32 x)
	{
		m_distance = x;
		m_dirty = true;
	}

	F32 getDistance() const
	{
		return m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_innerAngleCos = cos(ang / 2.0);
		m_innerAngle = ang;
		m_dirty = true;
	}

	F32 getInnerAngleCos() const
	{
		return m_innerAngleCos;
	}

	F32 getInnerAngle() const
	{
		return m_innerAngle;
	}

	void setOuterAngle(F32 ang)
	{
		m_outerAngleCos = cos(ang / 2.0);
		m_outerAngle = ang;
		m_dirty = true;
	}

	F32 getOuterAngle() const
	{
		return m_outerAngle;
	}

	F32 getOuterAngleCos() const
	{
		return m_outerAngleCos;
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	void setShadowEnabled(const Bool x)
	{
		m_shadow = x;
	}

	U getShadowMapIndex() const
	{
		return static_cast<U>(m_shadowMapIndex);
	}

	void setShadowMapIndex(const U i)
	{
		ANKI_ASSERT(i < 0xFF);
		m_shadowMapIndex = static_cast<U8>(i);
	}

	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;

private:
	LightComponentType m_type;
	Vec4 m_diffColor = Vec4(0.5);
	Vec4 m_specColor = Vec4(0.5);
	union
	{
		F32 m_radius;
		F32 m_distance;
	};
	F32 m_innerAngleCos;
	F32 m_outerAngleCos;
	F32 m_outerAngle;
	F32 m_innerAngle;

	Bool8 m_shadow = false;
	U8 m_shadowMapIndex = 0xFF; ///< Used by the renderer

	Bool8 m_dirty = true;
};
/// @}

} // end namespace anki
