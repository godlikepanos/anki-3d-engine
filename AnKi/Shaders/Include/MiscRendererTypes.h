// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

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

	Vec2 m_prevJitterUv;
	RF32 m_ssaoPower;
	U32 m_frameCount;

	Mat3x4 m_viewMat;
};

ANKI_END_NAMESPACE
