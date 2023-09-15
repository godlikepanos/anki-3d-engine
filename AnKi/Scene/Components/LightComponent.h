// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Math.h>

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
		m_otherDirty = true;
	}

	void setRadius(F32 x)
	{
		m_point.m_radius = x;
		m_shapeDirty = true;
	}

	F32 getRadius() const
	{
		return m_point.m_radius;
	}

	void setDistance(F32 x)
	{
		m_spot.m_distance = x;
		m_shapeDirty = true;
	}

	F32 getDistance() const
	{
		return m_spot.m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_spot.m_innerAngle = ang;
		m_shapeDirty = true;
	}

	F32 getInnerAngle() const
	{
		return m_spot.m_innerAngle;
	}

	void setOuterAngle(F32 ang)
	{
		m_spot.m_outerAngle = ang;
		m_shapeDirty = true;
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
		if(x != m_shadow)
		{
			m_shadow = x;
			m_shapeDirty = m_otherDirty = true;
		}
	}

	Vec3 getDirection() const
	{
		return -m_worldTransform.getRotation().getZAxis().xyz();
	}

	Vec3 getWorldPosition() const
	{
		return m_worldTransform.getOrigin().xyz();
	}

	const Mat4& getSpotLightViewProjectionMatrix() const
	{
		ANKI_ASSERT(m_type == LightComponentType::kSpot);
		return m_spot.m_viewProjMat;
	}

	const Mat3x4& getSpotLightViewMatrix() const
	{
		ANKI_ASSERT(m_type == LightComponentType::kSpot);
		return m_spot.m_viewMat;
	}

	/// Calculate some matrices for each cascade. For dir lights.
	/// @param cameraFrustum Who is looking at the light.
	/// @param cascadeDistances The distances of the cascades.
	/// @param cascadeProjMats View projection matrices for each cascade.
	/// @param cascadeViewMats View matrices for each cascade. Optional.
	void computeCascadeFrustums(const Frustum& cameraFrustum, ConstWeakArray<F32> cascadeDistances, WeakArray<Mat4> cascadeProjMats,
								WeakArray<Mat3x4> cascadeViewMats = {}) const;

	U32 getUuid() const
	{
		ANKI_ASSERT(m_uuid);
		return m_uuid;
	}

	ANKI_INTERNAL void setShadowAtlasUvViewports(ConstWeakArray<Vec4> viewports);

	const GpuSceneArrays::Light::Allocation& getGpuSceneLightAllocation() const
	{
		return m_gpuSceneLight;
	}

private:
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
		Mat3x4 m_viewMat = Mat3x4::getIdentity();
		Mat4 m_viewProjMat = Mat4::getIdentity();
		F32 m_distance = 1.0f;
		F32 m_outerAngle = toRad(30.0f);
		F32 m_innerAngle = toRad(15.0f);
	};

	Point m_point;
	Spot m_spot;

	GpuSceneArrays::Light::Allocation m_gpuSceneLight;
	GpuSceneArrays::LightVisibleRenderablesHash::Allocation m_hash;

	Array<Vec4, 6> m_shadowAtlasUvViewports;

	U32 m_uuid = 0;

	LightComponentType m_type;

	U8 m_shadow : 1 = false;
	U8 m_shapeDirty : 1 = true;
	U8 m_otherDirty : 1 = true;
	U8 m_shadowAtlasUvViewportCount : 3 = 0;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
