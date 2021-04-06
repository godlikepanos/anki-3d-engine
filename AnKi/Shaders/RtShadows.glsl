// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/RtShadows.h>
#include <AnKi/Shaders/Pack.glsl>

const F32 RT_SHADOWS_MAX_HISTORY_LENGTH = 31.0;

UVec4 packRtShadows(F32 shadowFactors[MAX_RT_SHADOW_LAYERS])
{
	const U32 a = newPackUnorm4x8(Vec4(shadowFactors[0], shadowFactors[1], shadowFactors[2], shadowFactors[3]));
	const U32 b = newPackUnorm4x8(Vec4(shadowFactors[4], shadowFactors[5], shadowFactors[6], shadowFactors[7]));
	return UVec4(a, b, 0, 0);
}

void unpackRtShadows(UVec4 packed, out F32 shadowFactors[MAX_RT_SHADOW_LAYERS])
{
	const Vec4 a = newUnpackUnorm4x8(packed.x);
	const Vec4 b = newUnpackUnorm4x8(packed.y);
	shadowFactors[0] = a[0];
	shadowFactors[1] = a[1];
	shadowFactors[2] = a[2];
	shadowFactors[3] = a[3];
	shadowFactors[4] = b[0];
	shadowFactors[5] = b[1];
	shadowFactors[6] = b[2];
	shadowFactors[7] = b[3];
}

void zeroRtShadowLayers(out F32 shadowFactors[MAX_RT_SHADOW_LAYERS])
{
	ANKI_UNROLL for(U32 i = 0; i < MAX_RT_SHADOW_LAYERS; ++i)
	{
		shadowFactors[i] = 0.0;
	}
}
