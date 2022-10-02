// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

// EVSM
#define ANKI_EVSM4 0 // 2 component EVSM or 4 component EVSM

const F32 kEvsmPositiveConstant = 40.0f; // EVSM positive constant
const F32 kEvsmNegativeConstant = 5.0f; // EVSM negative constant
const F32 kEvsmBias = 0.01f;
const F32 kEvsmLightBleedingReduction = 0.05f;

struct EvsmResolveUniforms
{
	IVec2 m_viewportXY;
	Vec2 m_viewportZW;

	Vec2 m_uvScale;
	Vec2 m_uvTranslation;

	Vec2 m_uvMin;
	Vec2 m_uvMax;

	U32 m_blur;
	U32 m_padding0;
	U32 m_padding1;
	U32 m_padding2;
};

// RT shadows
const U32 kMaxRtShadowLayers = 8u;

struct RtShadowsUniforms
{
	F32 historyRejectFactor[kMaxRtShadowLayers]; // 1.0 means reject, 0.0 not reject
};

struct RtShadowsDenoiseUniforms
{
	Mat4 invViewProjMat;

	F32 time;
	F32 padding0;
	F32 padding1;
	F32 padding2;
};

// Indirect diffuse
struct IndirectDiffuseUniforms
{
	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;

	Vec4 m_projectionMat;

	ANKI_RP F32 m_radius; ///< In meters.
	U32 m_sampleCount;
	ANKI_RP F32 m_sampleCountf;
	ANKI_RP F32 m_ssaoBias;

	ANKI_RP F32 m_ssaoStrength;
	F32 m_padding0;
	F32 m_padding1;
	F32 m_padding2;
};

struct IndirectDiffuseDenoiseUniforms
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
	ANKI_RP Vec4 m_color;
	Vec4 m_depthPad3;
};

// Depth downscale
struct DepthDownscaleUniforms
{
	Vec2 m_srcTexSizeOverOne;
	U32 m_workgroupCount;
	U32 m_mipmapCount;

	U32 m_lastMipWidth;
	F32 m_padding0;
	F32 m_padding1;
	F32 m_padding2;
};

// Screen space reflections uniforms
struct SsrUniforms
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
struct VolumetricFogUniforms
{
	ANKI_RP Vec3 m_fogDiffuse;
	ANKI_RP F32 m_fogScatteringCoeff;

	ANKI_RP F32 m_fogAbsorptionCoeff;
	ANKI_RP F32 m_near;
	ANKI_RP F32 m_far;
	F32 m_padding;
};

// Vol lighting
struct VolumetricLightingUniforms
{
	ANKI_RP F32 m_densityAtMinHeight;
	ANKI_RP F32 m_densityAtMaxHeight;
	F32 m_minHeight;
	F32 m_oneOverMaxMinusMinHeight; // 1 / (maxHeight / minHeight)
};

ANKI_END_NAMESPACE
