// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

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
	F32 m_padding[3u];
};

struct IndirectDiffuseDenoiseUniforms
{
	Mat4 m_invertedViewProjectionJitterMat;
	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;
	F32 m_sampleCountDiv2;
	F32 m_padding[3u];
};

ANKI_END_NAMESPACE
