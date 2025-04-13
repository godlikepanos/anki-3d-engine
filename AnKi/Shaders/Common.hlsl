// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's recomended to include it

#pragma once

#if defined(__INTELLISENSE__)
#	include <AnKi/Shaders/Intellisense.hlsl>
#else
#	include <AnKi/Shaders/Include/Common.h>
#endif

// Common constants
constexpr F32 kEpsilonF32 = 0.000001f;
#if ANKI_SUPPORTS_16BIT_TYPES
constexpr F16 kEpsilonF16 = (F16)0.0001f; // Divisions by this should be OK according to http://weitz.de/ieee
#else
constexpr RF32 kEpsilonRF32 = 0.0001f;
#endif

template<typename T>
T getEpsilon();

template<>
F32 getEpsilon()
{
	return kEpsilonF32;
}

#if ANKI_SUPPORTS_16BIT_TYPES
template<>
F16 getEpsilon()
{
	return kEpsilonF16;
}
#endif

#if !ANKI_FORCE_FULL_FP_PRECISION && !ANKI_SUPPORTS_16BIT_TYPES
template<>
RF32 getEpsilon()
{
	return kEpsilonRF32;
}
#endif

constexpr U32 kMaxU32 = 0xFFFFFFFFu;
constexpr F32 kMaxF32 = 3.402823e+38;
#if !ANKI_SUPPORTS_16BIT_TYPES
constexpr RF32 kMaxRF32 = 65504.0f; // Max half float value according to wikipedia
#endif
#if ANKI_SUPPORTS_16BIT_TYPES
constexpr F16 kMaxF16 = (F16)65504.0;
#endif

template<typename T>
T getMaxNumericLimit();

template<>
F32 getMaxNumericLimit()
{
	return kMaxF32;
}

#if !ANKI_FORCE_FULL_FP_PRECISION && !ANKI_SUPPORTS_16BIT_TYPES
template<>
RF32 getMaxNumericLimit()
{
	return kMaxRF32;
}
#endif

#if ANKI_SUPPORTS_16BIT_TYPES
template<>
F16 getMaxNumericLimit()
{
	return kMaxF16;
}
#endif

template<>
U32 getMaxNumericLimit()
{
	return kMaxU32;
}

constexpr F32 kPi = 3.14159265358979323846;
constexpr F32 k2Pi = 2.0 * kPi;
constexpr F32 kHalfPi = kPi / 2.0;
constexpr F32 kNaN = 0.0 / 0.0;

struct Barycentrics
{
	Vec2 m_value;
};

#if ANKI_GR_BACKEND_VULKAN
#	define ANKI_FAST_CONSTANTS(type, var) [[vk::push_constant]] ConstantBuffer<type> var;
#else
#	define ANKI_FAST_CONSTANTS(type, var) ConstantBuffer<type> var : register(b0, space3000);
#endif

#if ANKI_GR_BACKEND_VULKAN
#	define ANKI_BINDLESS(texType, compType) \
		[[vk::binding(0, 1000000)]] Texture##texType<compType> g_bindlessTextures##texType##compType[]; \
		Texture##texType<compType> getBindlessTexture##texType##compType(U32 idx) \
		{ \
			return g_bindlessTextures##texType##compType[idx]; \
		} \
		Texture##texType<compType> getBindlessTextureNonUniformIndex##texType##compType(U32 idx) \
		{ \
			return g_bindlessTextures##texType##compType[NonUniformResourceIndex(idx)]; \
		}
#else
#	define ANKI_BINDLESS(texType, compType) \
		Texture##texType<compType> getBindlessTexture##texType##compType(U32 idx) \
		{ \
			Texture##texType<compType> tex = ResourceDescriptorHeap[idx]; \
			return tex; \
		} \
		Texture##texType<compType> getBindlessTextureNonUniformIndex##texType##compType(U32 idx) \
		{ \
			Texture##texType<compType> tex = ResourceDescriptorHeap[NonUniformResourceIndex(idx)]; \
			return tex; \
		}
#endif

#define ANKI_BINDLESS2(texType) \
	ANKI_BINDLESS(texType, UVec4) \
	ANKI_BINDLESS(texType, IVec4) \
	ANKI_BINDLESS(texType, Vec4)

#define ANKI_BINDLESS3() \
	ANKI_BINDLESS2(2D) \
	ANKI_BINDLESS2(Cube) \
	ANKI_BINDLESS2(2DArray) \
	ANKI_BINDLESS2(3D)

ANKI_BINDLESS3()

template<typename T>
U32 getStructuredBufferElementCount(T x)
{
	U32 size, stride;
	x.GetDimensions(size, stride);
	return size;
}

template<typename T>
U32 checkStructuredBuffer(T buff, U32 idx)
{
	ANKI_ASSERT(idx < getStructuredBufferElementCount(buff));
	return idx;
}

// Safely access a structured buffer. Throw an assertion if it's out of bounds
#define SBUFF(buff, idx) buff[checkStructuredBuffer(buff, idx)]

UVec3 checkUavTexture(RWTexture3D<Vec4> tex, UVec3 coords)
{
	UVec3 size;
	tex.GetDimensions(size.x, size.y, size.z);
	ANKI_ASSERT(coords.x < size.x && coords.y < size.y && coords.z < size.z);
	return coords;
}

UVec2 checkUavTexture(RWTexture2D<Vec4> tex, UVec2 coords)
{
	UVec2 size;
	tex.GetDimensions(size.x, size.y);
	ANKI_ASSERT(coords.x < size.x && coords.y < size.y);
	return coords;
}

/// Safely access a UAV texture. Throw an assertion if it's out of bounds
#define UAV_TEXTURE(tex, coords) tex[checkUavTexture(tex, coords)]

UVec3 checkSrvTexture(Texture3D<Vec4> tex, UVec3 coords)
{
	UVec3 size;
	tex.GetDimensions(size.x, size.y, size.z);
	ANKI_ASSERT(coords.x < size.x && coords.y < size.y && coords.z < size.z);
	return coords;
}

UVec2 checkSrvTexture(Texture2D<Vec4> tex, UVec2 coords)
{
	UVec2 size;
	tex.GetDimensions(size.x, size.y);
	ANKI_ASSERT(coords.x < size.x && coords.y < size.y);
	return coords;
}

/// Safely access a SRV texture. Throw an assertion if it's out of bounds
#define SRV_TEXTURE(tex, coords) tex[checkSrvTexture(tex, coords)]

// Need extra decoration for per-primitive stuff in Vulkan. Remove when https://github.com/microsoft/DirectXShaderCompiler/issues/6862 is fixed
#if ANKI_GR_BACKEND_VULKAN
#	define SpvCapabilityMeshShadingEXT 5283
#	define SpvDecorationPerPrimitiveEXT 5271

#	define ANKI_PER_PRIMITIVE_VAR [[vk::ext_extension("SPV_EXT_mesh_shader")]] [[vk::ext_capability(SpvCapabilityMeshShadingEXT)]]
#	define ANKI_PER_PRIMITIVE_MEMBER \
		[[vk::ext_extension("SPV_EXT_mesh_shader")]] [[vk::ext_decorate(SpvDecorationPerPrimitiveEXT)]] [[vk::ext_capability( \
			SpvCapabilityMeshShadingEXT)]]
#else
#	define ANKI_PER_PRIMITIVE_VAR
#	define ANKI_PER_PRIMITIVE_MEMBER
#endif

#if ANKI_GR_BACKEND_VULKAN && ANKI_CLOSEST_HIT_SHADER
#	define SpvBuiltInHitTriangleVertexPositionsKHR 5335
#	define SpvCapabilityRayTracingPositionFetchKHR 5336

[[vk::ext_extension("SPV_KHR_ray_tracing_position_fetch")]] [[vk::ext_capability(SpvCapabilityRayTracingPositionFetchKHR)]] [[vk::ext_builtin_input(
	SpvBuiltInHitTriangleVertexPositionsKHR)]] const static Vec3 gl_HitTriangleVertexPositions[3];
#endif

#if ANKI_GR_BACKEND_VULKAN
#	define SpvDecorationRelaxedPrecision 0
#	define ANKI_RELAXED_PRECISION [[vk::ext_decorate(SpvDecorationRelaxedPrecision)]]
#else
#	define ANKI_RELAXED_PRECISION
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

template<typename T>
T uvToNdc(T uv)
{
	T ndc = uv * 2.0f - 1.0f;
	ndc.y *= -1.0f;
	return ndc;
}

template<typename T>
T ndcToUv(T ndc)
{
	T uv = ndc * 0.5f + 0.5f;
	uv.y = 1.0f - uv.y;
	return uv;
}

// Define min3, max3, min4, max4 functions

#define DEFINE_COMPARISON(func, scalarType, vectorType) \
	scalarType func##4(vectorType##4 v) \
	{ \
		return func(v.x, func(v.y, func(v.z, v.w))); \
	} \
	scalarType func##4(scalarType x, scalarType y, scalarType z, scalarType w) \
	{ \
		return func(x, func(y, func(z, w))); \
	} \
	scalarType func##3(vectorType##3 v) \
	{ \
		return func(v.x, func(v.y, v.z)); \
	} \
	scalarType func##3(scalarType x, scalarType y, scalarType z) \
	{ \
		return func(x, func(y, z)); \
	} \
	scalarType func##2(vectorType##2 v) \
	{ \
		return func(v.x, v.y); \
	}

#define DEFINE_COMPARISON2(func) \
	DEFINE_COMPARISON(func, F32, Vec) \
	DEFINE_COMPARISON(func, I32, IVec) \
	DEFINE_COMPARISON(func, U32, UVec)

DEFINE_COMPARISON2(min)
DEFINE_COMPARISON2(max)

#undef DEFINE_COMPARISON2
#undef DEFINE_COMPARISON

// Trick intellisense
#if defined(__INTELLISENSE__)
#	define NOT_ZERO(exr) (1)
#else
#	define NOT_ZERO(exr) ((exr) != 0)
#endif

template<typename T>
T pow2(T x)
{
	return x * x;
}
