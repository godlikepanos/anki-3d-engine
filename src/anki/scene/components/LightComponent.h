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

// Forward
class Frustum;
class OrthographicFrustum;

/// @addtogroup scene
/// @{

enum class LightComponentType : U8
{
	POINT,
	SPOT,
	DIRECTIONAL, ///< Basically the sun.
	COUNT
};

/// Light component. It's a dummy component used to identify lights
class LightComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::LIGHT;

	LightComponent(LightComponentType type, U64 uuid);

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
		m_point.m_radius = x;
		m_flags.set(DIRTY);
	}

	F32 getRadius() const
	{
		return m_point.m_radius;
	}

	void setDistance(F32 x)
	{
		m_spot.m_distance = x;
		m_flags.set(DIRTY);
	}

	F32 getDistance() const
	{
		return m_spot.m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_spot.m_innerAngleCos = cos(ang / 2.0);
		m_spot.m_innerAngle = ang;
		m_flags.set(DIRTY);
	}

	F32 getInnerAngleCos() const
	{
		return m_spot.m_innerAngleCos;
	}

	F32 getInnerAngle() const
	{
		return m_spot.m_innerAngle;
	}

	void setOuterAngle(F32 ang)
	{
		m_spot.m_outerAngleCos = cos(ang / 2.0);
		m_spot.m_outerAngle = ang;
		m_flags.set(DIRTY);
	}

	F32 getOuterAngle() const
	{
		return m_spot.m_outerAngle;
	}

	F32 getOuterAngleCos() const
	{
		return m_spot.m_outerAngleCos;
	}

	Bool getShadowEnabled() const
	{
		return m_flags.get(SHADOW);
	}

	void setShadowEnabled(const Bool x)
	{
		m_flags.set(SHADOW, x);
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

	void setupPointLightQueueElement(PointLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::POINT);
		el.m_uuid = m_uuid;
		el.m_worldPosition = m_trf.getOrigin().xyz();
		el.m_radius = m_point.m_radius;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_userData = this;
		el.m_drawCallback = pointLightDebugDrawCallback;
	}

	void setupSpotLightQueueElement(SpotLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::SPOT);
		el.m_uuid = m_uuid;
		el.m_worldTransform = Mat4(m_trf);
		el.m_textureMatrix = m_spot.m_textureMat;
		el.m_distance = m_spot.m_distance;
		el.m_outerAngle = m_spot.m_outerAngle;
		el.m_innerAngle = m_spot.m_innerAngle;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_userData = this;
		el.m_drawCallback = spotLightDebugDrawCallback;
	}

	/// Setup a directional queue element.
	/// @param[in] frustum The frustum that is looking that directional light. Used to calculate the cascades.
	/// @param overrideFrustumFar Override frustum's far or set it to <0.0 to ignore that value.
	/// @param[out] el The queue element to fill out.
	/// @param[out] cascadeFrustums Fill those frustums as well. The size of this array is the count of the cascades.
	void setupDirectionalLightQueueElement(const Frustum& frustum,
		F32 overrideFrustumFar,
		DirectionalLightQueueElement& el,
		WeakArray<OrthographicFrustum> cascadeFrustums) const;

private:
	U64 m_uuid;
	Vec4 m_diffColor = Vec4(0.5f);
	Transform m_trf = Transform::getIdentity();

	struct Point
	{
		F32 m_radius;
	};

	struct Spot
	{
		Mat4 m_textureMat;
		F32 m_distance;
		F32 m_innerAngleCos;
		F32 m_outerAngleCos;
		F32 m_outerAngle;
		F32 m_innerAngle;
	};

	struct Dir
	{
		Vec3 m_sceneMin;
		Vec3 m_sceneMax;
	};

	union
	{
		Point m_point;
		Spot m_spot;
		Dir m_dir;
	};

	enum
	{
		SHADOW = 1 << 0,
		DIRTY = 1 << 1,
		TRF_DIRTY = 1 << 2
	};

	LightComponentType m_type;
	BitMask<U8> m_flags = BitMask<U8>(DIRTY | TRF_DIRTY);

	static void pointLightDebugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}

	static void spotLightDebugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}

	static void derectionalLightDebugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
