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

// Indirect diffuse
struct IndirectDiffuseConstants
{
	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;

	Vec4 m_projectionMat;

	RF32 m_radius; ///< In meters.
	U32 m_sampleCount;
	RF32 m_sampleCountf;
	RF32 m_ssaoBias;

	RF32 m_ssaoStrength;
	F32 m_padding0;
	F32 m_padding1;
	F32 m_padding2;
};

struct IndirectDiffuseDenoiseConstants
{
	Mat4 m_invertedViewProjectionJitterMat;

	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;

	F32 m_sampleCountDiv2;
	F32 m_padding0;
	F32 m_padding1;
	F32 m_padding2;
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
	UVec2 m_depthBufferSize;
	UVec2 m_framebufferSize;

	U32 m_frameCount;
	U32 m_depthMipCount;
	U32 m_maxSteps;
	U32 m_lightBufferMipCount;

	UVec2 m_padding0;
	F32 m_roughnessCutoff;
	U32 m_firstStepPixels;

	Mat4 m_prevViewProjMatMulInvViewProjMat;
	Mat4 m_projMat;
	Mat4 m_invProjMat;
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

ANKI_END_NAMESPACE
