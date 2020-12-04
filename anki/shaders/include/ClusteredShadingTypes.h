// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Mainly contains light related structures. Everything is packed to align with std140

#pragma once

#include <anki/shaders/include/Common.h>

ANKI_BEGIN_NAMESPACE

// Consts
const U32 TYPED_OBJECT_COUNT = 6u; // Point lights, spot lights, refl probes, GI probes, decals and fog volumes
const F32 INVALID_TEXTURE_INDEX = -1.0f;
const F32 LIGHT_FRUSTUM_NEAR_PLANE = 0.1f / 4.0f; // The near plane on the shadow map frustums.
const U32 MAX_SHADOW_CASCADES = 4u;
const F32 SUBSURFACE_MIN = 0.01f;
const U32 MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES = 8u; // Global illumination clipmap count.

// See the documentation in the ClustererBin class.
struct ClustererMagicValues
{
	Vec4 m_val0;
	Vec4 m_val1;
};

// Point light
struct PointLight
{
	Vec3 m_position; // Position in world space
	F32 m_squareRadiusOverOne; // 1/(radius^2)
	Vec3 m_diffuseColor;
	F32 m_shadowAtlasTileScale; // UV scale for all tiles
	Vec2 m_padding;
	F32 m_radius; // Radius
	U32 m_shadowLayer;
	Vec4 m_shadowAtlasTileOffsets[3u]; // It's a Vec4 because of the std140 limitations
};
const U32 _ANKI_SIZEOF_PointLight = 6 * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(PointLight) == _ANKI_SIZEOF_PointLight);

// Spot light
struct SpotLight
{
	Vec3 m_position; // Position in world space
	F32 m_squareRadiusOverOne; // 1/(radius^2)
	Vec3 m_diffuseColor;
	F32 m_shadowmapId; // Shadowmap tex ID
	Vec3 m_dir; // Light direction
	F32 m_radius; // Max distance
	F32 m_outerCos;
	F32 m_innerCos;
	U32 m_shadowLayer;
	U32 m_padding;
	Mat4 m_texProjectionMat;
};
const U32 _ANKI_SIZEOF_SpotLight = 4 * ANKI_SIZEOF(Vec4) + ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLight) == _ANKI_SIZEOF_SpotLight);

// Directional light (sun)
struct DirectionalLight
{
	Vec3 m_diffuseColor;
	U32 m_cascadeCount; // If it's zero then it doesn't case shadow
	Vec3 m_dir;
	U32 m_active;
	F32 m_effectiveShadowDistance;
	F32 m_shadowCascadesDistancePower;
	U32 m_shadowLayer;
	U32 m_padding;
	Mat4 m_textureMatrices[MAX_SHADOW_CASCADES];
};
const U32 _ANKI_SIZEOF_DirectionalLight = 3 * ANKI_SIZEOF(Vec4) + MAX_SHADOW_CASCADES * ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(DirectionalLight) == _ANKI_SIZEOF_DirectionalLight);

// Representation of a reflection probe
struct ReflectionProbe
{
	Vec3 m_position; // Position of the probe in world space
	F32 m_cubemapIndex; // Slice in cubemap array texture
	Vec3 m_aabbMin;
	F32 m_padding0;
	Vec3 m_aabbMax;
	F32 m_padding1;
};
const U32 _ANKI_SIZEOF_ReflectionProbe = 3 * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(ReflectionProbe) == _ANKI_SIZEOF_ReflectionProbe);

// Decal
struct Decal
{
	Vec4 m_diffUv;
	Vec4 m_normRoughnessUv;
	Mat4 m_texProjectionMat;
	Vec4 m_blendFactors;
};
const U32 _ANKI_SIZEOF_Decal = 3 * ANKI_SIZEOF(Vec4) + ANKI_SIZEOF(Mat4);
ANKI_SHADER_STATIC_ASSERT(sizeof(Decal) == _ANKI_SIZEOF_Decal);

// Fog density volume
struct FogDensityVolume
{
	Vec3 m_aabbMinOrSphereCenter;
	U32 m_isBox;
	Vec3 m_aabbMaxOrSphereRadiusSquared;
	F32 m_density;
};
const U32 _ANKI_SIZEOF_FogDensityVolume = 2u * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(FogDensityVolume) == _ANKI_SIZEOF_FogDensityVolume);

// Global illumination probe
struct GlobalIlluminationProbe
{
	Vec3 m_aabbMin;
	U32 m_textureIndex;

	Vec3 m_aabbMax;
	F32 m_halfTexelSizeU; // (1.0 / textureSize(texArr[m_textureIndex]).x) / 2.0

	// Used to calculate a factor that is zero when fragPos is close to AABB bounds and 1.0 at m_fadeDistance and less.
	F32 m_fadeDistance;
	F32 m_padding0;
	F32 m_padding1;
	F32 m_padding2;
};
const U32 _ANKI_SIZEOF_GlobalIlluminationProbe = 3u * ANKI_SIZEOF(Vec4);
ANKI_SHADER_STATIC_ASSERT(sizeof(GlobalIlluminationProbe) == _ANKI_SIZEOF_GlobalIlluminationProbe);

// Common uniforms for light shading passes
struct LightingUniforms
{
	Vec4 m_unprojectionParams;

	Vec2 m_rendererSize;
	F32 m_time;
	F32 m_near;

	Vec3 m_cameraPos;
	F32 m_far;

	ClustererMagicValues m_clustererMagicValues;
	ClustererMagicValues m_prevClustererMagicValues;

	UVec4 m_clusterCount;

	Vec2 m_padding;
	U32 m_frameCount;
	U32 m_lightVolumeLastCluster;

	Mat4 m_viewMat;
	Mat4 m_invViewMat;
	Mat4 m_projMat;
	Mat4 m_invProjMat;
	Mat4 m_viewProjMat;
	Mat4 m_invViewProjMat;
	Mat4 m_prevViewProjMat;
	Mat4 m_prevViewProjMatMulInvViewProjMat; // Used to re-project previous frames

	DirectionalLight m_dirLight;
};
const U32 _ANKI_SIZEOF_LightingUniforms =
	9u * ANKI_SIZEOF(Vec4) + 8u * ANKI_SIZEOF(Mat4) + ANKI_SIZEOF(DirectionalLight);
ANKI_SHADER_STATIC_ASSERT(sizeof(LightingUniforms) == _ANKI_SIZEOF_LightingUniforms);

ANKI_END_NAMESPACE
