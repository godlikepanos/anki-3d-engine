// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct DeferredPointLightUniforms
{
	// Use these to get the correct face UVs
	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvBias;

	Vec2 m_fbUvScale;
	Vec2 m_fbUvBias;

	Mat4 m_invViewProjMat;

	Vec3 m_camPos;
	F32 m_padding;

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
	Vec2 m_inputTexUvBias;

	Vec2 m_fbUvScale;
	Vec2 m_fbUvBias;

	Mat4 m_invViewProjMat;

	Vec3 m_camPos;
	F32 m_padding;

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
	Vec2 m_inputTexUvBias;

	Vec2 m_fbUvScale;
	Vec2 m_fbUvBias;

	Mat4 m_invViewProjMat;

	Vec3 m_camPos;
	F32 m_near;

	// Light props
	Vec3 m_diffuseColor;
	F32 m_far;

	Vec3 m_lightDir;
	F32 m_effectiveShadowDistance;

	Mat4 m_lightMatrix;
};

struct DeferredVertexUniforms
{
	Mat4 m_mvp;
};

struct DeferredSkyboxUniforms
{
#if ANKI_GLSL
	ANKI_RP Vec3 m_solidColor;
#else
	RVec3 m_solidColor;
#endif
	F32 m_padding1;

	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvBias;

	Mat4 m_invertedViewProjectionMat;

	Vec3 m_cameraPos;
	F32 m_padding2;
};

ANKI_END_NAMESPACE
