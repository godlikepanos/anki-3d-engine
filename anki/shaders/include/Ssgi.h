// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/include/Common.h>

ANKI_BEGIN_NAMESPACE

// Screen space reflections uniforms
struct SsgiUniforms
{
	UVec2 m_depthBufferSize;
	UVec2 m_framebufferSize;
	U32 m_frameCount;
	U32 m_maxSteps;
	U32 m_firstStepPixels;
	U32 m_padding0;
	Mat4 m_invProjMat;
	Mat4 m_projMat;
	Mat4 m_prevViewProjMatMulInvViewProjMat;
#ifdef __cplusplus
	Mat3x4 m_normalMat;
#else
	Mat3 m_normalMat;
#endif
};

ANKI_END_NAMESPACE
