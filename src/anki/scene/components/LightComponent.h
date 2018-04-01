// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/renderer/RenderQueue.h>
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
	static constexpr F32 FRUSTUM_NEAR_PLANE = 0.1f / 4.0f;

	LightComponent(SceneNode* node, LightComponentType type);

	LightComponentType getLightComponentType() const
	{
		return m_type;
	}

	void updateWorldTransform(const Transform& trf)
	{
		m_trf = trf;
		m_flags.set(TRF_DIRTY);
	}

	const Vec4& getDiffuseColor() const
	{
		return m_diffColor;
	}

	void setDiffuseColor(const Vec4& x)
	{
		m_diffColor = x;
	}

	void setRadius(F32 x)
	{
		m_radius = x;
		m_flags.set(DIRTY);
	}

	F32 getRadius() const
	{
		return m_radius;
	}

	void setDistance(F32 x)
	{
		m_distance = x;
		m_flags.set(DIRTY);
	}

	F32 getDistance() const
	{
		return m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_innerAngleCos = cos(ang / 2.0);
		m_innerAngle = ang;
		m_flags.set(DIRTY);
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
		m_flags.set(DIRTY);
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
		return m_flags.get(SHADOW);
	}

	void setShadowEnabled(const Bool x)
	{
		m_flags.set(SHADOW, x);
	}

	ANKI_USE_RESULT Error update(SceneNode&, Second, Second, Bool& updated) override;

	void setupPointLightQueueElement(PointLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::POINT);
		el.m_uuid = getUuid();
		el.m_worldPosition = m_trf.getOrigin().xyz();
		el.m_radius = m_radius;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_userData = this;
		el.m_drawCallback = pointLightDebugDrawCallback;
	}

	void setupSpotLightQueueElement(SpotLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::SPOT);
		el.m_uuid = getUuid();
		el.m_worldTransform = Mat4(m_trf);
		el.m_textureMatrix = m_spotTextureMatrix;
		el.m_distance = m_distance;
		el.m_outerAngle = m_outerAngle;
		el.m_innerAngle = m_innerAngle;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_userData = this;
		el.m_drawCallback = spotLightDebugDrawCallback;
	}

private:
	LightComponentType m_type;
	Vec4 m_diffColor = Vec4(0.5f);
	union
	{
		F32 m_radius;
		F32 m_distance;
	};
	F32 m_innerAngleCos;
	F32 m_outerAngleCos;
	F32 m_outerAngle;
	F32 m_innerAngle;

	Transform m_trf = Transform::getIdentity();
	Mat4 m_spotTextureMatrix = Mat4::getIdentity();

	enum
	{
		SHADOW = 1 << 0,
		DIRTY = 1 << 1,
		TRF_DIRTY = 1 << 2
	};

	BitMask<U8> m_flags = BitMask<U8>(DIRTY | TRF_DIRTY);

	static void pointLightDebugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}

	static void spotLightDebugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
