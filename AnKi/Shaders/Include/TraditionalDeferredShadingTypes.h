// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct TraditionalDeferredShadingDirectionalLight
{
	Vec3 m_padding;
	F32 m_effectiveShadowDistance;

	Mat4 m_lightMatrix;
};

struct TraditionalDeferredShadingConstants
{
	// Use these to get the correct face UVs
	Vec2 m_inputTexUvScale;
	Vec2 m_inputTexUvBias;

	Mat4 m_invViewProjMat;

	Vec3 m_cameraPos;
	F32 m_padding0;

	TraditionalDeferredShadingDirectionalLight m_dirLight;
};

struct TraditionalDeferredSkyboxConstants
{
	RVec3 m_solidColor;
	F32 m_padding1;

	Mat4 m_invertedViewProjectionMat;

	Vec3 m_cameraPos;
	F32 m_padding2;

	Vec3 m_scale;
	F32 m_padding3;

	Vec3 m_bias;
	F32 m_padding4;
};

ANKI_END_NAMESPACE
