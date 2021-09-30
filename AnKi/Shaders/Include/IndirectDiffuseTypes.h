// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct IndirectDiffuseUniforms
{
	UVec2 m_depthBufferSize;
	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;
	U32 m_maxSteps;
	U32 m_stepIncrement;
};

struct IndirectDiffuseDenoiseUniforms
{
	Mat4 m_invertedViewProjectionJitterMat;
	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;
	F32 m_minSampleCount;
	F32 m_maxSampleCount;
	F32 m_padding[2u];
};

ANKI_END_NAMESPACE
