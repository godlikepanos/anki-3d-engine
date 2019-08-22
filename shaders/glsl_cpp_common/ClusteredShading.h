// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Mainly contains light related structures. Everything is packed to align with std140

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

// Consts
const U32 TYPED_OBJECT_COUNT = 6u; // Point lights, spot lights, refl probes, GI probes, decals and fog volumes
const F32 INVALID_TEXTURE_INDEX = -1.0f;
const F32 LIGHT_FRUSTUM_NEAR_PLANE = 0.1f / 4.0f; // The near plane on the shadow map frustums.
const U32 MAX_SHADOW_CASCADES = 4u;
const F32 SUBSURFACE_MIN = 0.05f;
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
	Vec3 m_padding;
	F32 m_radius; // Radius
	Vec4 m_shadowAtlasTileOffsets[3u]; // It's a Vec4 because of the std140 limitations
};
const U32 SIZEOF_POINT_LIGHT = 6 * SIZEOF_VEC4;
ANKI_SHADER_STATIC_ASSERT(sizeof(PointLight) == SIZEOF_POINT_LIGHT)

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
	F32 m_padding0;
	F32 m_padding1;
	Mat4 m_texProjectionMat;
};
const U32 SIZEOF_SPOT_LIGHT = 4 * SIZEOF_VEC4 + SIZEOF_MAT4;
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLight) == SIZEOF_SPOT_LIGHT)

// Directional light (sun)
struct DirectionalLight
{
	Vec3 m_diffuseColor;
	U32 m_cascadeCount; // If it's zero then it doesn't case shadow
	Vec3 m_dir;
	U32 m_active;
	Mat4 m_textureMatrices[MAX_SHADOW_CASCADES];
};
const U32 SIZEOF_DIR_LIGHT = 2 * SIZEOF_VEC4 + MAX_SHADOW_CASCADES * SIZEOF_MAT4;
ANKI_SHADER_STATIC_ASSERT(sizeof(DirectionalLight) == SIZEOF_DIR_LIGHT)

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
const U32 SIZEOF_REFLECTION_PROBE = 3 * SIZEOF_VEC4;
ANKI_SHADER_STATIC_ASSERT(sizeof(ReflectionProbe) == SIZEOF_REFLECTION_PROBE)

// Decal
struct Decal
{
	Vec4 m_diffUv;
	Vec4 m_normRoughnessUv;
	Mat4 m_texProjectionMat;
	Vec4 m_blendFactors;
};
const U32 SIZEOF_DECAL = 3 * SIZEOF_VEC4 + SIZEOF_MAT4;
ANKI_SHADER_STATIC_ASSERT(sizeof(Decal) == SIZEOF_DECAL)

// Fog density volume
struct FogDensityVolume
{
	Vec3 m_aabbMinOrSphereCenter;
	U32 m_isBox;
	Vec3 m_aabbMaxOrSphereRadiusSquared;
	F32 m_density;
};
const U32 SIZEOF_FOG_DENSITY_VOLUME = 2u * SIZEOF_VEC4;
ANKI_SHADER_STATIC_ASSERT(sizeof(FogDensityVolume) == SIZEOF_FOG_DENSITY_VOLUME)

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
const U32 SIZEOF_GLOBAL_ILLUMINATION_PROBE = 3u * SIZEOF_VEC4;
ANKI_SHADER_STATIC_ASSERT(sizeof(GlobalIlluminationProbe) == SIZEOF_GLOBAL_ILLUMINATION_PROBE)

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

	Vec3 m_padding;
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
const U32 SIZEOF_LIGHTING_UNIFORMS = 9u * SIZEOF_VEC4 + 8u * SIZEOF_MAT4 + SIZEOF_DIR_LIGHT;
ANKI_SHADER_STATIC_ASSERT(sizeof(LightingUniforms) == SIZEOF_LIGHTING_UNIFORMS)

ANKI_SHADER_FUNC_INLINE F32 computeClusterKf(ClustererMagicValues magic, Vec3 worldPos)
{
	const F32 fz = sqrt(dot(magic.m_val0.xyz(), worldPos) - magic.m_val0.w());
	return fz;
}

ANKI_SHADER_FUNC_INLINE U32 computeClusterK(ClustererMagicValues magic, Vec3 worldPos)
{
	return U32(computeClusterKf(magic, worldPos));
}

// Compute cluster index
ANKI_SHADER_FUNC_INLINE U32 computeClusterIndex(
	ClustererMagicValues magic, Vec2 uv, Vec3 worldPos, U32 clusterCountX, U32 clusterCountY)
{
	const UVec2 xy = UVec2(uv * Vec2(F32(clusterCountX), F32(clusterCountY)));
	const U32 k = computeClusterK(magic, worldPos);
	return k * (clusterCountX * clusterCountY) + xy.y() * clusterCountX + xy.x();
}

// Compute the Z of the near plane given a cluster idx
ANKI_SHADER_FUNC_INLINE F32 computeClusterNearf(ClustererMagicValues magic, F32 fk)
{
	return magic.m_val1.x() * fk * fk + magic.m_val1.y();
}

// Compute the Z of the near plane given a cluster idx
ANKI_SHADER_FUNC_INLINE F32 computeClusterNear(ClustererMagicValues magic, U32 k)
{
	return computeClusterNearf(magic, F32(k));
}

// Compute the UV coordinates of a volume texture that encloses the clusterer
ANKI_SHADER_FUNC_INLINE Vec3 computeClustererVolumeTextureUvs(
	ClustererMagicValues magic, Vec2 uv, Vec3 worldPos, U32 clusterCountZ)
{
	const F32 k = computeClusterKf(magic, worldPos);
	return Vec3(uv, k / F32(clusterCountZ));
}

ANKI_END_NAMESPACE
