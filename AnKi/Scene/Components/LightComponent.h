// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Math.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

// Forward
class Frustum;

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

	~LightComponent();

	void setLightComponentType(LightComponentType type);

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

	void setRadius(F32 x)
	{
		m_point.m_radius = x;
		m_shapeUpdated = true;
	}

	F32 getRadius() const
	{
		return m_point.m_radius;
	}

	void setDistance(F32 x)
	{
		m_spot.m_distance = x;
		m_shapeUpdated = true;
	}

	F32 getDistance() const
	{
		return m_spot.m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_spot.m_innerAngle = ang;
		m_shapeUpdated = true;
	}

	F32 getInnerAngle() const
	{
		return m_spot.m_innerAngle;
	}

	void setOuterAngle(F32 ang)
	{
		m_spot.m_outerAngle = ang;
		m_shapeUpdated = true;
	}

	F32 getOuterAngle() const
	{
		return m_spot.m_outerAngle;
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	void setShadowEnabled(const Bool x)
	{
		m_shadow = x;
		m_shapeUpdated = true;
	}

	ANKI_INTERNAL WeakArray<Frustum> getFrustums() const
	{
		ANKI_ASSERT(m_shadow);
		return WeakArray<Frustum>(m_frustums, m_frustumCount);
	}

	void setupPointLightQueueElement(PointLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::kPoint);
		el.m_uuid = m_uuid;
		el.m_worldPosition = m_worldTransform.getOrigin().xyz();
		el.m_radius = m_point.m_radius;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_shadowLayer = kMaxU8;
		ANKI_ASSERT(m_gpuSceneLightIndex != kMaxU32);
		el.m_index = m_gpuSceneLightIndex;
	}

	void setupSpotLightQueueElement(SpotLightQueueElement& el) const
	{
		ANKI_ASSERT(m_type == LightComponentType::kSpot);
		el.m_uuid = m_uuid;
		el.m_worldTransform = Mat4(m_worldTransform);
		el.m_textureMatrix = m_spot.m_textureMat;
		el.m_distance = m_spot.m_distance;
		el.m_outerAngle = m_spot.m_outerAngle;
		el.m_innerAngle = m_spot.m_innerAngle;
		el.m_diffuseColor = m_diffColor.xyz();
		el.m_edgePoints = m_spot.m_edgePointsWspace;
		el.m_shadowLayer = kMaxU8;
		ANKI_ASSERT(m_gpuSceneLightIndex != kMaxU32);
		el.m_index = m_gpuSceneLightIndex;
	}

	/// Setup a directional queue element.
	/// @param[in] cameraFrustum The frustum that is looking that directional light. Used to calculate the cascades.
	/// @param[out] el The queue element to fill out.
	/// @param[out] cascadeFrustums Fill those frustums as well. The size of this array is the count of the cascades.
	void setupDirectionalLightQueueElement(const Frustum& cameraFrustum, DirectionalLightQueueElement& el,
										   WeakArray<Frustum> cascadeFrustums) const;

private:
	SceneNode* m_node;
	U64 m_uuid;
	Vec4 m_diffColor = Vec4(0.5f);
	Transform m_worldTransform = Transform::getIdentity();

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

	Spatial m_spatial;

	Frustum* m_frustums = nullptr;

	U32 m_gpuSceneLightIndex = kMaxU32;

	LightComponentType m_type;

	U8 m_shadow : 1 = false;
	U8 m_shapeUpdated : 1 = true;
	U8 m_typeChanged : 1 = true;
	U8 m_frustumCount : 4 = 0; ///< The size of m_frustums array.

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	void onDestroy(SceneNode& node);
};
/// @}

} // end namespace anki
