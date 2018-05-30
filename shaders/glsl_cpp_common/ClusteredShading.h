// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GLSL_CPP_COMMON_CLUSTERED_SHADING_H
#define ANKI_SHADERS_GLSL_CPP_COMMON_CLUSTERED_SHADING_H

#include <shaders/glsl_cpp_common/Clusterer.h>

ANKI_BEGIN_NAMESPACE

// Common uniforms between lights
struct LightingUniforms
{
	Vec4 m_unprojectionParams;
	Vec4 m_rendererSizeTimeNear;
	Vec4 m_cameraPosFar;
	ClustererMagicValues m_clustererMagicValues;
	UVec4 m_tileCount;
	Mat4 m_viewMat;
	Mat4 m_invViewMat;
	Mat4 m_projMat;
	Mat4 m_invProjMat;
	Mat4 m_viewProjMat;
	Mat4 m_invViewProjMat;
	Mat4 m_prevViewProjMat;
	Mat4 m_prevViewProjMatMulInvViewProjMat; // Used to re-project previous frames
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

// Representation of a reflection probe
struct ReflectionProbe
{
	Vec4 m_positionCubemapIndex; // xyz: Position of the prove in view space. w: Slice in u_reflectionsTex vector.
	Vec4 m_aabbMinPad1;
	Vec4 m_aabbMaxPad1;
};
const U32 SIZEOF_REFLECTION_PROBE = 2 * SIZEOF_VEC4;

// Decal
struct Decal
{
	Vec4 m_diffUv;
	Vec4 m_normRoughnessUv;
	Mat4 m_texProjectionMat;
	Vec4 m_blendFactors;
};
const U32 SIZEOF_DECAL = 3 * SIZEOF_VEC4 + SIZEOF_MAT4;

ANKI_END_NAMESPACE

#endif
