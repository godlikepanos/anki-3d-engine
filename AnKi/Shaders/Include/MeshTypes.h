// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	kUv,
	kBoneIds,
	kBoneWeights,

	kMeshRelatedCount,
	kMeshRelatedFirst = 0,

	// For particles
	kParticlePosition = 0,
	kParticlePreviousPosition,
	kParticleScale,
	kParticleColor,
	kParticleLife,
	kParticleStartingLife,
	kParticleVelocity,

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
	kUv = 1 << 2,
	kBoneIds = 1 << 3,
	kBoneWeights = 1 << 4,

	kParticlePosition = 1 << 0,
	kParticlePreviousPosition = 1 << 1,
	kParticleScale = 1 << 2,
	kParticleColor = 1 << 3,
	kParticleLife = 1 << 4,
	kParticleStartingLife = 1 << 5,
	kParticleVelocity = 1 << 6,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexStreamMask)

#if defined(__cplusplus)
inline constexpr Array<Format, U32(VertexStreamId::kMeshRelatedCount)> kMeshRelatedVertexStreamFormats = {
	Format::kR16G16B16A16_Unorm, Format::kR8G8B8A8_Snorm, Format::kR32G32_Sfloat, Format::kR8G8B8A8_Uint, Format::kR8G8B8A8_Snorm};

inline constexpr Array<Format, U32(VertexStreamId::kParticleRelatedCount)> kParticleRelatedVertexStreamFormats = {
	Format::kR32G32B32_Sfloat, Format::kR32G32B32_Sfloat, Format::kR32_Sfloat,      Format::kR32G32B32A32_Sfloat,
	Format::kR32_Sfloat,       Format::kR32_Sfloat,       Format::kR32G32B32_Sfloat};

constexpr Format kMeshletPrimitiveFormat = Format::kR8G8B8A8_Uint;
#endif

struct UnpackedMeshVertex
{
	Vec3 m_position;
	RVec3 m_normal;
	Vec2 m_uv;
	UVec4 m_boneIndices;
	RVec4 m_boneWeights;
};

struct Meshlet
{
	U32 m_vertexOffsets[(U32)VertexStreamId::kMeshRelatedCount];
	U32 m_firstPrimitive; // In size of kMeshletPrimitiveFormat
	U32 m_primitiveCount_R16_Uint_vertexCount_R16_Uint;
	U32 m_coneDirection_R8G8B8_Snorm_minusSinAngle_R8_Snorm;

	Vec3 m_sphereCenter;
	F32 m_sphereRadius;

	Vec4 m_padding;
};
// Power of 2 because the sizeof will be used as allocation alignment and allocation alignments need to be power of 2
static_assert(isPowerOfTwo(sizeof(Meshlet)));

ANKI_END_NAMESPACE
