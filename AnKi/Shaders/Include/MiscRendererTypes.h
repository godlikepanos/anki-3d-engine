// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

/// Directional light (sun).
struct DirectionalLight
{
	RVec3 m_diffuseColor;
	F32 m_power;

	RVec3 m_direction;
	U32 m_shadowCascadeCount_31bit_active_1bit; ///< If shadowCascadeCount is zero then it doesn't cast shadow.

	Vec4 m_shadowCascadeDistances;

	Mat4 m_textureMatrices[kMaxShadowCascades];

	Vec4 m_cascadeFarPlanes;

	Vec4 m_cascadePcfTexelRadius; ///< The radius of the PCF. In UV space.
};
static_assert(kMaxShadowCascades == 4u); // Because m_shadowCascadeDistances and m_cascadeFarPlanes is a Vec4

/// Common matrices and stuff.
struct CommonMatrices
{
	Mat3x4 m_cameraTransform;
	Mat3x4 m_view;
	Mat4 m_projection;
	Mat4 m_viewProjection;

	Mat4 m_jitter;
	Mat4 m_projectionJitter;
	Mat4 m_viewProjectionJitter;

	Mat4 m_invertedViewProjectionJitter; ///< To unproject in world space.
	Mat4 m_invertedViewProjection;
	Mat4 m_invertedProjectionJitter; ///< To unproject in view space.

	/// It's being used to reproject a clip space position of the current frame to the previous frame. Its value should be m_jitter *
	/// m_prevFrame.m_viewProjection * m_invertedViewProjectionJitter. At first it unprojects the current position to world space, all fine here. Then
	/// it projects to the previous frame as if the previous frame was using the current frame's jitter matrix.
	Mat4 m_reprojection;

	/// To unproject to view space. Jitter not considered.
	/// @code
	/// F32 z = m_unprojectionParameters.z / (m_unprojectionParameters.w + depth);
	/// Vec2 xy = ndc * m_unprojectionParameters.xy * z;
	/// pos = Vec3(xy, z);
	/// @endcode
	Vec4 m_unprojectionParameters;

	Vec2 m_jitterOffsetNdc;
	Vec2 m_padding;
};

/// Common constants for all passes.
struct GlobalRendererConstants
{
	Vec2 m_renderingSize;
	F32 m_time;
	U32 m_frame;

	Vec4 m_nearPlaneWSpace;

	Vec3 m_cameraPosition;
	F32 m_reflectionProbesMipCount;

	UVec2 m_tileCounts;
	U32 m_zSplitCount;
	F32 m_zSplitCountOverFrustumLength; ///< m_zSplitCount/(far-near)

	Vec2 m_zSplitMagic; ///< It's the "a" and "b" of computeZSplitClusterIndex(). See there for details.
	U32 m_lightVolumeLastZSplit;
	U32 m_padding1;

	UVec2 m_padding0;
	F32 m_near;
	F32 m_far;

	DirectionalLight m_directionalLight;

	CommonMatrices m_matrices;
	CommonMatrices m_previousMatrices;
};

// RT shadows
struct RtShadowsDenoiseConstants
{
	Mat4 m_invViewProjMat;

	F32 m_time;
	U32 m_minSampleCount;
	U32 m_maxSampleCount;
	F32 m_padding2;
};

struct RtShadowsSbtBuildConstants
{
	U32 m_shaderHandleDwordSize;
	U32 m_sbtRecordDwordSize;
	U32 m_padding0;
	U32 m_padding1;
};

// Lens flare
struct LensFlareSprite
{
	Vec4 m_posScale; // xy: Position, zw: Scale
	RVec4 m_color;
	Vec4 m_depthPad3;
};

// Depth downscale
struct DepthDownscaleConstants
{
	Vec2 m_srcTexSizeOverOne;
	U32 m_threadgroupCount;
	U32 m_mipmapCount;
};

// Screen space reflections uniforms
struct SsrConstants
{
	Vec2 m_viewportSizef;
	U32 m_frameCount;
	U32 m_maxIterations;

	UVec2 m_padding;
	F32 m_roughnessCutoff;
	U32 m_stepIncrement;

	Vec4 m_projMat00_11_22_23;

	Vec4 m_unprojectionParameters;

	Mat4 m_prevViewProjMatMulInvViewProjMat;
	Mat3x4 m_normalMat;
};

// Vol fog
struct VolumetricFogConstants
{
	RVec3 m_fogDiffuse;
	RF32 m_fogScatteringCoeff;

	RF32 m_fogAbsorptionCoeff;
	RF32 m_near;
	RF32 m_far;
	F32 m_zSplitCountf;

	UVec3 m_volumeSize;
	F32 m_maxZSplitsToProcessf;
};

// Vol lighting
struct VolumetricLightingConstants
{
	RF32 m_densityAtMinHeight;
	RF32 m_densityAtMaxHeight;
	F32 m_minHeight;
	F32 m_oneOverMaxMinusMinHeight; // 1 / (maxHeight / minHeight)

	UVec3 m_volumeSize;
	F32 m_maxZSplitsToProcessf;
};

// SSAO
struct SsaoConstants
{
	RF32 m_radius; ///< In meters.
	U32 m_sampleCount;
	Vec2 m_viewportSizef;

	Vec4 m_unprojectionParameters;

	F32 m_projectionMat00;
	F32 m_projectionMat11;
	F32 m_projectionMat22;
	F32 m_projectionMat23;

	Vec2 m_padding;
	RF32 m_ssaoPower;
	U32 m_frameCount;

	Mat3x4 m_viewMat;
};

struct SsaoSpatialDenoiseConstants
{
	Mat3x4 m_viewToWorldMat;

	Vec2 m_linearizeDepthParams;
	Vec2 m_padding;
};

struct LodAndRenderableIndex
{
	U32 m_lod_2bit_renderableIndex_30bit;
};

ANKI_END_NAMESPACE
