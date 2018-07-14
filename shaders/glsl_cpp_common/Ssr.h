// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

// Screen space reflections uniforms
struct SsrUniforms
{
	Vec4 m_nearPad3;
	Mat4 m_prevViewProjMatMulInvViewProjMat;
	Mat4 m_projMat;
	Mat4 m_invProjMat;
#ifdef __cplusplus
	Mat3x4 m_normalMat;
#else
	Mat3 m_normalMat;
#endif
};

ANKI_END_NAMESPACE
