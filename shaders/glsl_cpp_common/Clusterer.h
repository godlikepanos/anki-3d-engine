// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GLSL_CPP_COMMON_CLUSTERER_H
#define ANKI_SHADERS_GLSL_CPP_COMMON_CLUSTERER_H

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

// See the documentation in the Clusterer class.
struct ClustererMagicValues
{
	Vec4 m_val0;
	Vec4 m_val1;
};

ANKI_SHADER_FUNC_INLINE U32 computeClusterK(ClustererMagicValues magic, Vec3 worldPos)
{
	F32 fz = sqrt(dot(magic.m_val0.xyz(), worldPos) - magic.m_val0.w());
	U32 z = U32(fz);
	return z;
}

// Compute cluster index
ANKI_SHADER_FUNC_INLINE U32 computeClusterIndex(
	ClustererMagicValues magic, Vec2 uv, Vec3 worldPos, U32 clusterCountX, U32 clusterCountY)
{
	UVec2 xy = UVec2(uv * Vec2(clusterCountX, clusterCountY));

	return computeClusterK(magic, worldPos) * (clusterCountX * clusterCountY) + xy.y() * clusterCountX + xy.x();
}

// Compute the Z of the near plane given a cluster idx
ANKI_SHADER_FUNC_INLINE F32 computeClusterNear(ClustererMagicValues magic, U32 k)
{
	F32 fk = F32(k);
	return magic.m_val1.x() * fk * fk + magic.m_val1.y();
}

ANKI_SHADER_FUNC_INLINE F32 computeClusterFar(ClustererMagicValues magic, U32 k)
{
	return computeClusterNear(magic, k + 1u);
}

ANKI_END_NAMESPACE

#endif