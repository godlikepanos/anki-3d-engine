// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

// For regular geometry
const U32 kVertexStreamPosition = 0u;
const U32 kVertexStreamNormal = 1u;
const U32 kVertexStreamTangent = 2u;
const U32 kVertexStreamUv = 3u;
const U32 kVertexStreamBoneIds = 4u;
const U32 kVertexStreamBoneWeights = 5u;

const U32 kRegularVertexStreamCount = 6u;

#if __cplusplus
inline constexpr Array<Format, kRegularVertexStreamCount> kRegularVertexStreamFormats = {
	Format::kR32R32G32B32_Sfloat, Format::kR8G8B8A8_Snorm, Format::kR8G8B8A8_Snorm,
	Format::kR32G32_Sfloat,       Format::kR8G8B8A8_Uint,  Format::kR8G8B8A8_Snorm};
#endif

// For particles
const U32 kVertexStreamParticlePosition = 0u;
const U32 kVertexStreamParticleScale = 1u;
const U32 kVertexStreamParticleAlpha = 2u;
const U32 kVertexStreamParticleLife = 3u;
const U32 kVertexStreamParticleStartingLife = 4u;
const U32 kVertexStreamParticlePreviousPosition = 5u;

ANKI_END_NAMESPACE
