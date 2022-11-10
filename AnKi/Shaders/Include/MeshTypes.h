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
	Format::kR16G16B16_Unorm, Format::kR8G8B8A8_Snorm, Format::kR8G8B8A8_Snorm,
	Format::kR32G32_Sfloat,   Format::kR8G8B8A8_Uint,  Format::kR8G8B8A8_Snorm};

constexpr U32 kMaxVertexStreamIds = 6u;

#else

// For regular geometry
const U32 kVertexStreamIdPosition = 0u;
const U32 kVertexStreamIdNormal = 1u;
const U32 kVertexStreamIdTangent = 2u;
const U32 kVertexStreamIdUv = 3u;
const U32 kVertexStreamIdBoneIds = 4u;
const U32 kVertexStreamIdBoneWeights = 5u;

// For particles
const U32 kVertexStreamIdParticlePosition = 0u;
const U32 kVertexStreamIdParticleScale = 1u;
const U32 kVertexStreamIdParticleAlpha = 2u;
const U32 kVertexStreamIdParticleLife = 3u;
const U32 kVertexStreamIdParticleStartingLife = 4u;
const U32 kVertexStreamIdParticlePreviousPosition = 5u;

const U32 kMaxVertexStreamIds = 6u;
#endif

ANKI_END_NAMESPACE
