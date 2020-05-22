// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

// Screen space reflections uniforms
struct SsgiUniforms
{
	UVec2 m_depthBufferSize;
	UVec2 m_framebufferSize;
};

ANKI_END_NAMESPACE
