// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

struct DeferredPointLightUniforms
{
	Vec4 m_inputTexUvScaleAndOffset; // Use this to get the correct face UVs
	Mat4 m_invViewProjMat;
	Vec4 m_camPosPad1;
	Vec4 m_fbSizePad2;

	// Light props
	Vec4 m_posRadius; // xyz: Light pos in world space. w: The -1/radius
	Vec4 m_diffuseColorPad1; // xyz: diff color
};

struct DeferredSpotLightUniforms
{
	Vec4 m_inputTexUvScaleAndOffset; // Use this to get the correct face UVs
	Mat4 m_invViewProjMat;
	Vec4 m_camPosPad1;
	Vec4 m_fbSizePad2;

	// Light props
	Vec4 m_posRadius; // xyz: Light pos in world space. w: The -1/radius
	Vec4 m_diffuseColorOuterCos; // xyz: diff color, w: outer cosine of spot
	Vec4 m_lightDirInnerCos; // xyz: light dir, w: inner cosine of spot
};

struct DeferredVertexUniforms
{
	Mat4 m_mvp;
};

const UVec2 GBUFFER_RT0_BINDING = UVec2(0, 0);
const UVec2 GBUFFER_RT1_BINDING = UVec2(0, 1);
const UVec2 GBUFFER_RT2_BINDING = UVec2(0, 2);
const UVec2 GBUFFER_DEPTH_BINDING = UVec2(0, 3);

ANKI_END_NAMESPACE
