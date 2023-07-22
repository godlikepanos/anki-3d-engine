// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct TraditionalDeferredShadingDirectionalLight
{
	Vec3 m_diffuseColor;
	U32 m_active;

	Vec3 m_direction;
	F32 m_effectiveShadowDistance;

	Mat4 m_lightMatrix;
};

struct TraditionalDeferredShadingUniforms
{
	// Use these to get the correct face UVs
	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvBias;

	Vec2 m_fbUvScale;
	Vec2 m_fbUvBias;

	Mat4 m_invViewProjMat;

	Vec3 m_cameraPos;
	F32 m_padding0;

	TraditionalDeferredShadingDirectionalLight m_dirLight;
};

struct TraditionalDeferredSkyboxUniforms
{
	RVec3 m_solidColor;
	F32 m_padding1;

	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvBias;

	Mat4 m_invertedViewProjectionMat;

	Vec3 m_cameraPos;
	F32 m_padding2;
};

ANKI_END_NAMESPACE
