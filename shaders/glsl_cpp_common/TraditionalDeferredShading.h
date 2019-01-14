// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

struct DeferredPointLightUniforms
{
	// Use these to get the correct face UVs
	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvOffset;

	Mat4 m_invViewProjMat;

	Vec3 m_camPos;
	F32 m_padding;

	Vec2 m_fbSize;
	Vec2 m_padding1;

	// Light props
	Vec3 m_position;
	F32 m_oneOverSquareRadius; // 1/radius^2

	Vec3 m_diffuseColor;
	F32 m_padding2;
};

struct DeferredSpotLightUniforms
{
	// Use these to get the correct face UVs
	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvOffset;

	Mat4 m_invViewProjMat;

	Vec3 m_camPos;
	F32 m_padding;

	Vec2 m_fbSize;
	Vec2 m_padding1;

	// Light props
	Vec3 m_position;
	F32 m_oneOverSquareRadius; // 1/radius^2

	Vec3 m_diffuseColor;
	F32 m_outerCos;

	Vec3 m_lightDir;
	F32 m_innerCos;
};

struct DeferredDirectionalLightUniforms
{
	// Use these to get the correct face UVs
	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvOffset;

	Mat4 m_invViewProjMat;

	Vec3 m_camPos;
	F32 m_padding;

	Vec2 m_fbSize;
	F32 m_near;
	F32 m_far;

	// Light props
	Vec3 m_diffuseColor;
	F32 m_padding2;

	Vec3 m_lightDir;
	F32 m_effectiveShadowDistance;

	Mat4 m_lightMatrix;
};

struct DeferredVertexUniforms
{
	Mat4 m_mvp;
};

const UVec2 GBUFFER_RT0_BINDING = UVec2(0, 0);
const UVec2 GBUFFER_RT1_BINDING = UVec2(0, 1);
const UVec2 GBUFFER_RT2_BINDING = UVec2(0, 2);
const UVec2 GBUFFER_DEPTH_BINDING = UVec2(0, 3);
const UVec2 GBUFFER_SHADOW_ATLAS_BINDING = UVec2(0, 4);

ANKI_END_NAMESPACE
