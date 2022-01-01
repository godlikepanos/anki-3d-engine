// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

const U32 MAX_RT_SHADOW_LAYERS = 8u;

struct RtShadowsUniforms
{
	F32 historyRejectFactor[MAX_RT_SHADOW_LAYERS]; // 1.0 means reject, 0.0 not reject
};

struct RtShadowsDenoiseUniforms
{
	Mat4 invViewProjMat;
	F32 time;
	U32 padding[3];
};

ANKI_END_NAMESPACE
