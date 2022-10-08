// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

#if __cplusplus
enum class VertexStreamId : U8
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
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexStreamId)

enum class VertexStreamMask : U8
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

inline constexpr Array<Format, U32(VertexStreamId::kMeshRelatedCount)> kMeshRelatedVertexStreamFormats = {
	Format::kR32G32B32A32_Sfloat, Format::kR8G8B8A8_Snorm, Format::kR8G8B8A8_Snorm,
	Format::kR32G32_Sfloat,       Format::kR8G8B8A8_Uint,  Format::kR8G8B8A8_Snorm};

#else

// For regular geometry
const U32 kVertexStreamPosition = 0u;
const U32 kVertexStreamNormal = 1u;
const U32 kVertexStreamTangent = 2u;
const U32 kVertexStreamUv = 3u;
const U32 kVertexStreamBoneIds = 4u;
const U32 kVertexStreamBoneWeights = 5u;

const U32 kVertexStreamRegularCount = 6u;

// For particles
const U32 kVertexStreamParticlePosition = 0u;
const U32 kVertexStreamParticleScale = 1u;
const U32 kVertexStreamParticleAlpha = 2u;
const U32 kVertexStreamParticleLife = 3u;
const U32 kVertexStreamParticleStartingLife = 4u;
const U32 kVertexStreamParticlePreviousPosition = 5u;
#endif

ANKI_END_NAMESPACE
