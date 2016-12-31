// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_RENDERER_COMMON_UNIFORMS_GLSL
#define ANKI_SHADERS_RENDERER_COMMON_UNIFORMS_GLSL

#include "shaders/Common.glsl"

struct RendererCommonUniforms
{
	// Projection params
	vec4 projectionParams;

	// x: near, y: far, zw: depth linearization coefficients
	vec4 nearFarLinearizeDepth;

	mat4 projectionMatrix;
};

#endif
