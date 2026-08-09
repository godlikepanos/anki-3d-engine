// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Math.h>
#include <AnKi/Collision/Common.h>

namespace anki {

// Forward
class Frustum;

enum class LightComponentType : U8
{
	kPoint,
	kSpot,
	kDirectional, // Basically the sun.

	kCount,
	kFirst = 0
};

inline constexpr Array<const Char*, U32(LightComponentType::kCount)> kLightComponentTypeNames = {"Point", "Spot", "Directional"};

// Light component. Contains all the info of lights.
class LightComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LightComponent)

public:
	static constexpr F32 kMinSourceRadius = 10.0_cm;
	static constexpr F32 kMaxSourceRadius = 10.0_m;
	static constexpr F32 kMaxInfluenceRadius = 100.0_m;
	static constexpr F32 kMaxInfluenceDistance = 100.0_m;

	LightComponent(const SceneComponentInitInfo& init);

	~LightComponent();

	// Distance past which a light's illuminance falls under the reference and it can be culled. Makes the energy the cull discards a declared number
	// Ω=A/r^2 => A=Ω*r^2
	// I=Φ/Ω
	// E(r)=Φ/Α => E=Φ/(Ω*r^2) => E=(Φ/Ω)/r^2 => E=I/r^2
	// Where Ω: solid angle, Φ: luminous power, A: area, I: luminous intensity, E: illuminance, r: distance/radius
	// To compute r we need a base so we define another E' which is low and acts as our epsilon. E=E' and solve for r which is r=r'*sqrt(I/I')
	// The defaults are a candle (~1cd) seen from 50cm, so E'=4lx
	static F32 computeLightInfluenceRadius(F32 luminousIntensity, F32 refLuminousIntensity = 1.0f, F32 refDistance = 50.0_cm)
	{
		ANKI_ASSERT(luminousIntensity >= 0.0f);
		ANKI_ASSERT(refLuminousIntensity > 0.0f);
		ANKI_ASSERT(refDistance > 0.0f);
		return refDistance * sqrt(luminousIntensity / refLuminousIntensity);
	}

	void setLightComponentType(LightComponentType type);

	LightComponentType getLightComponentType() const
	{
		return m_type;
	}

	// Colour of the light. Only its chromaticity matters, the luminance gets normalized away on upload, so changing the hue doesn't change how bright
	// the light is. Use setLuminousPower() or setIlluminance() for that.
	const Vec3& getColor() const
	{
		return m_color;
	}

	void setColor(Vec3 x)
	{
		x = (ANKI_EXPECT(x >= Vec3(0.0f))) ? x : x.max(Vec3(0.0f));
		if(m_color != x)
		{
			m_color = x;
			m_otherDirty = true;
		}
	}

	// Total light emitted by the bulb, in lumens. Point and spot lights only.
	F32 getLuminousPower() const
	{
		return ANKI_EXPECT(m_type == LightComponentType::kPoint || m_type == LightComponentType::kSpot) ? m_pointAndSpot.m_luminousPower : 0.0f;
	}

	void setLuminousPower(F32 lumens)
	{
		lumens = (ANKI_EXPECT(lumens >= 0.0f)) ? lumens : 0.0f;
		if(ANKI_EXPECT(m_type == LightComponentType::kPoint || m_type == LightComponentType::kSpot) && m_pointAndSpot.m_luminousPower != lumens)
		{
			m_pointAndSpot.m_luminousPower = lumens;
			m_otherDirty = true;
		}
	}

	// Light arriving on a surface facing the light, in lux. Directional lights only, since they're the ones far enough away for it to be constant.
	F32 getIlluminance() const
	{
		return (ANKI_EXPECT(m_type == LightComponentType::kDirectional)) ? m_dir.m_illuminance : 0.0f;
	}

	void setIlluminance(F32 lux)
	{
		lux = (ANKI_EXPECT(lux >= 0.0f)) ? lux : 0.0f;
		if(ANKI_EXPECT(m_type == LightComponentType::kDirectional) && m_dir.m_illuminance != lux)
		{
			m_dir.m_illuminance = lux;
			m_otherDirty = true;
		}
	}

	// Physical size of the emitter. Bounds the inverse-square falloff up close and is the sphere that the ray traced paths sample.
	F32 getSourceRadius() const
	{
		return (ANKI_EXPECT(m_type == LightComponentType::kPoint || m_type == LightComponentType::kSpot)) ? m_pointAndSpot.m_sourceRadius : 0.0f;
	}

	void setSourceRadius(F32 x)
	{
		x = (ANKI_EXPECT(x >= kMinSourceRadius && x <= kMaxSourceRadius)) ? x : clamp(x, kMinSourceRadius, kMaxSourceRadius);
		if(ANKI_EXPECT(m_type == LightComponentType::kPoint || m_type == LightComponentType::kSpot) && m_pointAndSpot.m_sourceRadius != x)
		{
			m_pointAndSpot.m_sourceRadius = x;
			m_otherDirty = true;
		}
	}

	// How far the light reaches. Only feeds culling and the falloff window, it doesn't change how bright the light is.
	F32 getInfluenceRadius() const
	{
		return (ANKI_EXPECT(m_type == LightComponentType::kPoint)) ? m_pointAndSpot.m_influenceRadius : 0.0f;
	}

	void setInfluenceRadius(F32 x)
	{
		x = (ANKI_EXPECT(x >= kMinSourceRadius && x <= kMaxInfluenceRadius)) ? x : clamp(x, kMinSourceRadius, kMaxInfluenceRadius);
		if(ANKI_EXPECT(m_type == LightComponentType::kPoint) && m_pointAndSpot.m_influenceRadius != x)
		{
			m_pointAndSpot.m_influenceRadius = x;
			m_shapeDirty = true;
		}
	}

	// The spot light's equivalent of the influence radius: the length of the cone.
	F32 getInfluenceDistance() const
	{
		return (ANKI_EXPECT(m_type == LightComponentType::kSpot)) ? m_pointAndSpot.m_influenceRadius : 0.0f;
	}

	void setInfluenceDistance(F32 x)
	{
		x = (ANKI_EXPECT(x >= kMinSourceRadius && x <= kMaxInfluenceDistance)) ? x : clamp(x, kMinSourceRadius, kMaxInfluenceDistance);
		if(ANKI_EXPECT(m_type == LightComponentType::kSpot) && m_pointAndSpot.m_influenceRadius != x)
		{
			m_pointAndSpot.m_influenceRadius = x;
			m_shapeDirty = true;
		}
	}

	F32 getInnerAngle() const
	{
		return (ANKI_EXPECT(m_type == LightComponentType::kSpot)) ? m_spot.m_innerAngle : 0.0f;
	}

	void setInnerAngle(F32 ang)
	{
		ang = (ANKI_EXPECT(ang >= 0.0f)) ? ang : 0.0f;
		if(ANKI_EXPECT(m_type == LightComponentType::kSpot) && m_spot.m_innerAngle != ang)
		{
			m_spot.m_innerAngle = ang;
			m_shapeDirty = true;
		}
	}

	F32 getOuterAngle() const
	{
		return (ANKI_EXPECT(m_type == LightComponentType::kSpot)) ? m_spot.m_outerAngle : 0.0f;
	}

	void setOuterAngle(F32 ang)
	{
		ang = (ANKI_EXPECT(ang >= 0.0f)) ? ang : 0.0f;
		if(ANKI_EXPECT(m_type == LightComponentType::kSpot) && m_spot.m_outerAngle != ang)
		{
			m_spot.m_outerAngle = ang;
			m_shapeDirty = true;
		}
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	void setShadowEnabled(Bool x)
	{
		if(x != m_shadow)
		{
			m_shadow = x;
			m_shapeDirty = m_otherDirty = true;
		}
	}

	Vec3 getDirection() const
	{
		return -m_worldTransform.getRotation().getZAxis().xyz;
	}

	Vec3 getWorldPosition() const
	{
		return m_worldTransform.getOrigin().xyz;
	}

	// Set the direction of the directional light by setting the date and hour.
	void setDirectionFromTimeOfDay(I32 month, I32 day, F32 hour)
	{
		if(ANKI_EXPECT(m_type == LightComponentType::kDirectional))
		{
			ANKI_EXPECT(month >= 0 && month < 12);
			ANKI_EXPECT(day >= 0 && day < 31);
			ANKI_EXPECT(hour >= 0.0f && hour <= 24.0f);
			m_dir.m_month = clamp(month, 0, 11);
			m_dir.m_day = clamp(day, 0, 30);
			m_dir.m_hour = clamp(hour, 0.0f, 24.0f);
			m_shapeDirty = true;
		}
	}

	// Get the fields which might or might not have come from the direction of the light
	void getTimeOfDay(I32& month, I32& day, F32& hour) const
	{
		if(ANKI_EXPECT(m_type == LightComponentType::kDirectional))
		{
			month = m_dir.m_month;
			day = m_dir.m_day;
			hour = m_dir.m_hour;
		}
		else
		{
			month = day = 0;
			hour = 0.0f;
		}
	}

	ANKI_INTERNAL const Mat4& getSpotLightViewProjectionMatrix() const
	{
		ANKI_ASSERT(m_type == LightComponentType::kSpot);
		return m_spot.m_viewProjMat;
	}

	ANKI_INTERNAL const Mat3x4& getSpotLightViewMatrix() const
	{
		ANKI_ASSERT(m_type == LightComponentType::kSpot);
		return m_spot.m_viewMat;
	}

	// Calculate some matrices for each cascade. For dir lights.
	// cameraFrustum Who is looking at the light.
	// cascadeDistances The distances of the cascades.
	// cascadeProjMats View projection matrices for each cascade.
	// cascadeViewMats View matrices for each cascade. Optional.
	ANKI_INTERNAL void computeCascadeFrustums(const Frustum& cameraFrustum, ConstWeakArray<F32> cascadeDistances, WeakArray<Mat4> cascadeProjMats,
											  WeakArray<Mat3x4> cascadeViewMats = {},
											  WeakArray<Array<F32, U32(FrustumPlaneType::kCount)>> cascadePlanes = {}) const;

	ANKI_INTERNAL void setShadowAtlasUvViewports(ConstWeakArray<Vec4> viewports);

	ANKI_INTERNAL const GpuSceneArrays::Light::Allocation& getGpuSceneLightAllocation() const
	{
		return m_gpuSceneLight;
	}

private:
	Vec3 m_color = Vec3(1.0f);
	Transform m_worldTransform = Transform::getIdentity();

	class Spot
	{
	public:
		Mat3x4 m_viewMat = Mat3x4::getIdentity();
		Mat4 m_viewProjMat = Mat4::getIdentity();
		F32 m_outerAngle = toRad(30.0f);
		F32 m_innerAngle = toRad(15.0f);
	};

	class PointAndSpot
	{
	public:
		F32 m_sourceRadius = kMinSourceRadius;
		F32 m_influenceRadius = 1.0f;
		F32 m_luminousPower = 1000.0f; // In Lumens. Roughly a 75W incandescent bulb.
	};

	class Dir
	{
	public:
		F32 m_illuminance = 100000.0f; // Lux. Clear sky at noon.
		I32 m_month = -1;
		I32 m_day = -1;
		F32 m_hour = -1.0;
	};

	Spot m_spot;
	PointAndSpot m_pointAndSpot;
	Dir m_dir;

	GpuSceneArrays::Light::Allocation m_gpuSceneLight;
	GpuSceneArrays::LightVisibleRenderablesHash::Allocation m_hash;

	Array<Vec4, 6> m_shadowAtlasUvViewports;

	LightComponentType m_type;

	U8 m_shadow : 1 = false;
	U8 m_shapeDirty : 1 = true;
	U8 m_otherDirty : 1 = true;
	U8 m_shadowAtlasUvViewportCount : 3 = 0;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
