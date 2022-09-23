// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Scene/Components/SceneComponent.h>

namespace anki {

/// @addtogroup scene
/// @{

enum class LightComponentType : U8
{
	kPoint,
	kSpot,
	kDirectional, ///< Basically the sun.

	kCount,
	kFirst = 0
};

/// Light component. Contains all the info of lights.
class LightComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LightComponent)

public:
	LightComponent(SceneNode* node);

	~LightComponent()
	{
	}

	void setLightComponentType(LightComponentType type)
	{
		ANKI_ASSERT(type >= LightComponentType::kFirst && type < LightComponentType::kCount);
		m_type = type;
		m_markedForUpdate = true;
	}

	LightComponentType getLightComponentType() const
	{
		return m_type;
	}

	void setWorldTransform(const Transform& trf)
	{
		m_worldtransform = trf;
		m_markedForUpdate = true;
	}

	const Transform& getWorldTransform() const
	{
		return m_worldtransform;
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
		m_markedForUpdate = true;
	}

	F32 getRadius() const
	{
		return m_point.m_radius;
	}

	void setDistance(F32 x)
	{
		m_spot.m_distance = x;
		m_markedForUpdate = true;
	}

	F32 getDistance() const
	{
		return m_spot.m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_spot.m_innerAngleCos = cos(ang / 2.0f);
		m_spot.m_innerAngle = ang;
		m_markedForUpdate = true;
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
		m_spot.m_outerAngleCos = cos(ang / 2.0f);
		m_spot.m_outerAngle = ang;
		m_markedForUpdate = true;
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
		return m_shadow;
	}

	void setShadowEnabled(const Bool x)
	{
		m_shadow = x;
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void setupPointLightQueueElement(PointLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::kPoint);
		el.m_uuid = m_uuid;
		el.m_worldPosition = m_worldtransform.getOrigin().xyz();
		el.m_radius = m_point.m_radius;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_debugDrawCallback = [](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			ANKI_ASSERT(userData.getSize() == 1);
			static_cast<const LightComponent*>(userData[0])->draw(ctx);
		};
		el.m_debugDrawCallbackUserData = this;
		el.m_shadowLayer = kMaxU8;
	}

	void setupSpotLightQueueElement(SpotLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::kSpot);
		el.m_uuid = m_uuid;
		el.m_worldTransform = Mat4(m_worldtransform);
		el.m_textureMatrix = m_spot.m_textureMat;
		el.m_distance = m_spot.m_distance;
		el.m_outerAngle = m_spot.m_outerAngle;
		el.m_innerAngle = m_spot.m_innerAngle;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_edgePoints = m_spot.m_edgePointsWspace;
		el.m_debugDrawCallback = [](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			ANKI_ASSERT(userData.getSize() == 1);
			static_cast<const LightComponent*>(userData[0])->draw(ctx);
		};
		el.m_debugDrawCallbackUserData = this;
		el.m_shadowLayer = kMaxU8;
	}

	/// Setup a directional queue element.
	/// @param[in] frustumComp The frustum that is looking that directional light. Used to calculate the cascades.
	/// @param[out] el The queue element to fill out.
	/// @param[out] cascadeFrustumComponents Fill those frustums as well. The size of this array is the count of the
	///             cascades.
	void setupDirectionalLightQueueElement(const FrustumComponent& frustumComp, DirectionalLightQueueElement& el,
										   WeakArray<FrustumComponent> cascadeFrustumComponents) const;

private:
	SceneNode* m_node = nullptr;
	U64 m_uuid;
	Vec4 m_diffColor = Vec4(0.5f);
	Transform m_worldtransform = Transform::getIdentity();

	class Point
	{
	public:
		F32 m_radius = 1.0f;
	};

	class Spot
	{
	public:
		Mat4 m_textureMat = Mat4::getIdentity();
		F32 m_distance = 1.0f;
		F32 m_outerAngle = toRad(30.0f);
		F32 m_innerAngle = toRad(15.0f);
		F32 m_outerAngleCos = cos(m_outerAngle / 2.0f);
		F32 m_innerAngleCos = cos(m_innerAngle / 2.0f);
		Array<Vec3, 4> m_edgePointsWspace = {};
	};

	class Dir
	{
	public:
		Vec3 m_sceneMin = Vec3(-1.0f);
		Vec3 m_sceneMax = Vec3(1.0f);
	};

	Point m_point;
	Spot m_spot;
	Dir m_dir;

	ImageResourcePtr m_pointDebugImage;
	ImageResourcePtr m_spotDebugImage;

	LightComponentType m_type;

	U8 m_shadow : 1;
	U8 m_markedForUpdate : 1;

	void draw(RenderQueueDrawContext& ctx) const;
};
/// @}

} // end namespace anki
