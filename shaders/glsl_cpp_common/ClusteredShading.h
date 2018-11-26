// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

// Consts
const U32 TYPED_OBJECT_COUNT = 5u;
const F32 INVALID_TEXTURE_INDEX = -1.0;
const F32 LIGHT_FRUSTUM_NEAR_PLANE = 0.1 / 4.0; // The near plane on the shadow map frustums.

// See the documentation in the ClustererBin class.
struct ClustererMagicValues
{
	Vec4 m_val0;
	Vec4 m_val1;
};

// Point light
struct PointLight
{
	Vec4 m_posRadius; // xyz: Light pos in world space. w: The 1/(radius^2)
	Vec4 m_diffuseColorTileSize; // xyz: diff color, w: tile size in the shadow atlas
	Vec2 m_radiusPad1; // x: radius
	UVec2 m_atlasTiles; // x: encodes 6 uints with atlas tile indices in the x dir. y: same for y dir.
};
const U32 SIZEOF_POINT_LIGHT = 3 * SIZEOF_VEC4;
ANKI_SHADER_STATIC_ASSERT(sizeof(PointLight) == SIZEOF_POINT_LIGHT)

// Spot light
struct SpotLight
{
	Vec4 m_posRadius; // xyz: Light pos in world space. w: The 1/(radius^2)
	Vec4 m_diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	Vec4 m_lightDirRadius; // xyz: light direction, w: radius
	Vec4 m_outerCosInnerCos;
	Mat4 m_texProjectionMat;
};
const U32 SIZEOF_SPOT_LIGHT = 4 * SIZEOF_VEC4 + SIZEOF_MAT4;
ANKI_SHADER_STATIC_ASSERT(sizeof(SpotLight) == SIZEOF_SPOT_LIGHT)

// Representation of a reflection probe
struct ReflectionProbe
{
	Vec4 m_positionCubemapIndex; // xyz: Position of the prove in view space. w: Slice in u_reflectionsTex vector.
	Vec4 m_aabbMinPad1;
	Vec4 m_aabbMaxPad1;
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
const U32 SIZEOF_FOG_DENSITY_VOLUME = 2 * SIZEOF_VEC4;
ANKI_SHADER_STATIC_ASSERT(sizeof(FogDensityVolume) == SIZEOF_FOG_DENSITY_VOLUME)

// Common uniforms for light shading passes
struct LightingUniforms
{
	Vec4 m_unprojectionParams;
	Vec4 m_rendererSizeTimeNear;
	Vec4 m_cameraPosFar;
	ClustererMagicValues m_clustererMagicValues;
	ClustererMagicValues m_prevClustererMagicValues;
	UVec4 m_clusterCount;
	UVec4 m_lightVolumeLastClusterPad3;
	Mat4 m_viewMat;
	Mat4 m_invViewMat;
	Mat4 m_projMat;
	Mat4 m_invProjMat;
	Mat4 m_viewProjMat;
	Mat4 m_invViewProjMat;
	Mat4 m_prevViewProjMat;
	Mat4 m_prevViewProjMatMulInvViewProjMat; // Used to re-project previous frames
};

ANKI_SHADER_FUNC_INLINE F32 computeClusterKf(ClustererMagicValues magic, Vec3 worldPos)
{
	F32 fz = sqrt(dot(magic.m_val0.xyz(), worldPos) - magic.m_val0.w());
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
	UVec2 xy = UVec2(uv * Vec2(clusterCountX, clusterCountY));
	U32 k = computeClusterK(magic, worldPos);
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
	F32 k = computeClusterKf(magic, worldPos);
	return Vec3(uv, k / F32(clusterCountZ));
}

ANKI_END_NAMESPACE
