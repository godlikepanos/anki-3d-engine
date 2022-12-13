// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

#if ANKI_HLSL
enum class VertexStreamId : U32
#else
enum class VertexStreamId : U8
#endif
{
	// For regular geometry
	kPosition,
	kNormal,
	kTangent,
	kUv,
	kBoneIds,
	kBoneWeights,

	kMeshRelatedCount,
	kMeshRelatedFirst = 0,

	// For particles
	kParticlePosition = 0,
	kParticleScale,
	kParticleAlpha,
	kParticleLife,
	kParticleStartingLife,
	kParticlePreviousPosition,

	kParticleRelatedCount,
	kParticleRelatedFirst = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexStreamId)

#if ANKI_HLSL
enum class VertexStreamMask : U32
#else
enum class VertexStreamMask : U8
#endif
{
	kNone,

	kPosition = 1 << 0,
	kNormal = 1 << 1,
	kTangent = 1 << 2,
	kUv = 1 << 3,
	kBoneIds = 1 << 4,
	kBoneWeights = 1 << 5,

	kParticlePosition = 1 << 0,
	kParticleScale = 1 << 1,
	kParticleAlpha = 1 << 2,
	kParticleLife = 1 << 3,
	kParticleStartingLife = 1 << 4,
	kParticlePreviousPosition = 1 << 5,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexStreamMask)

#if defined(__cplusplus)
inline constexpr Array<Format, U32(VertexStreamId::kMeshRelatedCount)> kMeshRelatedVertexStreamFormats = {
	Format::kR16G16B16_Unorm, Format::kR8G8B8A8_Snorm, Format::kR8G8B8A8_Snorm,
	Format::kR32G32_Sfloat,   Format::kR8G8B8A8_Uint,  Format::kR8G8B8A8_Snorm};
#endif

ANKI_END_NAMESPACE
