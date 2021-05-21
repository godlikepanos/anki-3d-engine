// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

ANKI_BEGIN_NAMESPACE

// Compute the far plane of a shadow cascade. "p" is the power that defines the distance curve.
// "effectiveShadowDistance" is the far plane of the last cascade.
ANKI_SHADER_FUNC_INLINE F32 computeShadowCascadeDistance(U32 cascadeIdx, F32 p, F32 effectiveShadowDistance,
														 U32 shadowCascadeCount)
{
	return pow((F32(cascadeIdx) + 1.0f) / F32(shadowCascadeCount), p) * effectiveShadowDistance;
}

// The reverse of computeShadowCascadeDistance().
ANKI_SHADER_FUNC_INLINE U32 computeShadowCascadeIndex(F32 distance, F32 p, F32 effectiveShadowDistance,
													  U32 shadowCascadeCount)
{
	const F32 shadowCascadeCountf = F32(shadowCascadeCount);
	F32 idx = pow(distance / effectiveShadowDistance, 1.0f / p) * shadowCascadeCountf;
	idx = min(idx, shadowCascadeCountf - 1.0f);
	return U32(idx);
}

ANKI_END_NAMESPACE
