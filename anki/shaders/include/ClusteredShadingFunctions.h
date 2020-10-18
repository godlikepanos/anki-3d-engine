// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/include/ClusteredShadingTypes.h>

ANKI_BEGIN_NAMESPACE

ANKI_SHADER_FUNC_INLINE F32 computeClusterKf(ClustererMagicValues magic, Vec3 worldPos)
{
	const F32 fz = sqrt(dot(magic.m_val0.xyz(), worldPos) - magic.m_val0.w());
	return fz;
}

ANKI_SHADER_FUNC_INLINE U32 computeClusterK(ClustererMagicValues magic, Vec3 worldPos)
{
	return U32(computeClusterKf(magic, worldPos));
}

// Compute cluster index
ANKI_SHADER_FUNC_INLINE U32 computeClusterIndex(ClustererMagicValues magic, Vec2 uv, Vec3 worldPos, U32 clusterCountX,
												U32 clusterCountY)
{
	const UVec2 xy = UVec2(uv * Vec2(F32(clusterCountX), F32(clusterCountY)));
	const U32 k = computeClusterK(magic, worldPos);
	return k * (clusterCountX * clusterCountY) + xy.y() * clusterCountX + xy.x();
}

// Compute the Z of the near plane given a cluster idx
ANKI_SHADER_FUNC_INLINE F32 computeClusterNearf(ClustererMagicValues magic, F32 fk)
{
	return magic.m_val1.x() * fk * fk + magic.m_val1.y();
}

// Compute the Z of the near plane given a cluster idx
ANKI_SHADER_FUNC_INLINE F32 computeClusterNear(ClustererMagicValues magic, U32 k)
{
	return computeClusterNearf(magic, F32(k));
}

// Compute the UV coordinates of a volume texture that encloses the clusterer
ANKI_SHADER_FUNC_INLINE Vec3 computeClustererVolumeTextureUvs(ClustererMagicValues magic, Vec2 uv, Vec3 worldPos,
															  U32 clusterCountZ)
{
	const F32 k = computeClusterKf(magic, worldPos);
	return Vec3(uv, k / F32(clusterCountZ));
}

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
