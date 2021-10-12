// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
	F32 m_radius; ///< In meters.
	U32 m_sampleCount;
	F32 m_sampleCountf;
	F32 m_ssaoBias;
	F32 m_ssaoStrength;
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
