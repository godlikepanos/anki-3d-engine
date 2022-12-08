// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#define ANKI_SUPPORTS_64BIT_TYPES !ANKI_PLATFORM_MOBILE

//! == C++ =============================================================================================================
#if defined(__cplusplus)

#	include <AnKi/Math.h>

#	define ANKI_HLSL 0
#	define ANKI_GLSL 0
#	define ANKI_CPP 1

#	define ANKI_BEGIN_NAMESPACE namespace anki {
#	define ANKI_END_NAMESPACE }
#	define ANKI_SHADER_FUNC_INLINE inline

#	define ANKI_SHADER_STATIC_ASSERT(cond_) static_assert(cond_)

ANKI_BEGIN_NAMESPACE
using Address = U64;
using ScalarVec4 = Array<F32, 4>;
using ScalarMat3x4 = Array<F32, 12>;
using ScalarMat4 = Array<F32, 16>;

using RF32 = F32;
using RVec2 = Vec2;
using RVec3 = Vec3;
using RVec4 = Vec4;
using RMat3 = Mat3;
ANKI_END_NAMESPACE

#	define ANKI_RP

//! == HLSL ============================================================================================================
#elif defined(__HLSL_VERSION)
#	define ANKI_HLSL 1
#	define ANKI_GLSL 0
#	define ANKI_CPP 0

#	define ANKI_BEGIN_NAMESPACE
#	define ANKI_END_NAMESPACE
#	define ANKI_SHADER_FUNC_INLINE

#	define ANKI_SHADER_STATIC_ASSERT(cond_)
#	define ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enum_)

#	define constexpr static const

#	define ANKI_SUPPORTS_16BIT_TYPES 0

template<typename T>
void maybeUnused(T a)
{
	a = a;
}
#	define ANKI_MAYBE_UNUSED(x) maybeUnused(x)

// Use a define because the [] annoyes clang-format
#	define _ANKI_NUMTHREADS
#	define ANKI_NUMTHREADS(x, y, z) _ANKI_NUMTHREADS[numthreads(x, y, z)]

#	define _ANKI_CONCATENATE(a, b) a##b
#	define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#	define ANKI_BINDLESS_SET(s) \
		[[vk::binding(0, s)]] Texture2D<uint4> g_bindlessTextures2dU32[kMaxBindlessTextures]; \
		[[vk::binding(0, s)]] Texture2D<int4> g_bindlessTextures2dI32[kMaxBindlessTextures]; \
		[[vk::binding(0, s)]] Texture2D<RVec4> g_bindlessTextures2dF32[kMaxBindlessTextures]; \
		[[vk::binding(0, s)]] Texture2DArray<RVec4> g_bindlessTextures2dArrayF32[kMaxBindlessTextures]; \
		[[vk::binding(1, s)]] Buffer<float4> g_bindlessTextureBuffersF32[kMaxBindlessReadonlyTextureBuffers];

#	define _ANKI_SCONST_X(type, n, id) [[vk::constant_id(id)]] const type n = (type)1;

#	define _ANKI_SCONST_X2(type, componentType, n, id) \
		[[vk::constant_id(id + 0u)]] const componentType ANKI_CONCATENATE(_anki_const_0_2_, n) = (componentType)1; \
		[[vk::constant_id(id + 1u)]] const componentType ANKI_CONCATENATE(_anki_const_1_2_, n) = (componentType)1; \
		static const type n = type(ANKI_CONCATENATE(_anki_const_0_2_, n), ANKI_CONCATENATE(_anki_const_1_2_, n))

#	define _ANKI_SCONST_X3(type, componentType, n, id) \
		[[vk::constant_id(id + 0u)]] const componentType ANKI_CONCATENATE(_anki_const_0_3_, n) = (componentType)1; \
		[[vk::constant_id(id + 1u)]] const componentType ANKI_CONCATENATE(_anki_const_1_3_, n) = (componentType)1; \
		[[vk::constant_id(id + 2u)]] const componentType ANKI_CONCATENATE(_anki_const_2_3_, n) = (componentType)1; \
		static const type n = type(ANKI_CONCATENATE(_anki_const_0_3_, n), ANKI_CONCATENATE(_anki_const_1_3_, n), \
								   ANKI_CONCATENATE(_anki_const_2_3_, n))

#	define _ANKI_SCONST_X4(type, componentType, n, id) \
		[[vk::constant_id(id + 0u)]] const componentType ANKI_CONCATENATE(_anki_const_0_4_, n) = (componentType)1; \
		[[vk::constant_id(id + 1u)]] const componentType ANKI_CONCATENATE(_anki_const_1_4_, n) = (componentType)1; \
		[[vk::constant_id(id + 2u)]] const componentType ANKI_CONCATENATE(_anki_const_2_4_, n) = (componentType)1; \
		[[vk::constant_id(id + 3u)]] const componentType ANKI_CONCATENATE(_anki_const_3_4_, n) = (componentType)1; \
		static const type n = type(ANKI_CONCATENATE(_anki_const_0_4_, n), ANKI_CONCATENATE(_anki_const_1_4_, n), \
								   ANKI_CONCATENATE(_anki_const_2_4_, n), ANKI_CONCATENATE(_anki_const_2_4_, n))

#	define ANKI_SPECIALIZATION_CONSTANT_I32(n, id) _ANKI_SCONST_X(I32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_IVEC2(n, id) _ANKI_SCONST_X2(IVec2, I32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_IVEC3(n, id) _ANKI_SCONST_X3(IVec3, I32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_IVEC4(n, id) _ANKI_SCONST_X4(IVec4, I32, n, id)

#	define ANKI_SPECIALIZATION_CONSTANT_U32(n, id) _ANKI_SCONST_X(U32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_UVEC2(n, id) _ANKI_SCONST_X2(UVec2, U32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_UVEC3(n, id) _ANKI_SCONST_X3(UVec3, U32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_UVEC4(n, id) _ANKI_SCONST_X4(UVec4, U32, n, id)

#	define ANKI_SPECIALIZATION_CONSTANT_F32(n, id) _ANKI_SCONST_X(F32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_VEC2(n, id) _ANKI_SCONST_X2(Vec2, F32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_VEC3(n, id) _ANKI_SCONST_X3(Vec3, F32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_VEC4(n, id) _ANKI_SCONST_X4(Vec4, F32, n, id)

#	pragma pack_matrix(row_major)

typedef float F32;
constexpr uint kSizeof_F32 = 4u;
typedef float2 Vec2;
constexpr uint kSizeof_Vec2 = 8u;
typedef float3 Vec3;
constexpr uint kSizeof_Vec3 = 12u;
typedef float4 Vec4;
constexpr uint kSizeof_Vec4 = 16u;

#	if ANKI_SUPPORTS_16BIT_TYPES
typedef float16_t F16;
constexpr uint kSizeof_F16 = 2u;
typedef float16_t2 HVec2;
constexpr uint kSizeof_HVec2 = 4u;
typedef float16_t3 HVec3;
constexpr uint kSizeof_HVec3 = 6u;
typedef float16_t4 HVec4;
constexpr uint kSizeof_HVec4 = 8u;

typedef uint16_t U16;
constexpr uint kSizeof_U16 = 2u;
typedef uint16_t2 U16Vec2;
constexpr uint kSizeof_U16Vec2 = 4u;
typedef uint16_t3 U16Vec3;
constexpr uint kSizeof_U16Vec3 = 6u;
typedef uint16_t4 U16Vec4;
constexpr uint kSizeof_U16Vec4 = 8u;

typedef int16_t I16;
constexpr uint kSizeof_I16 = 2u;
typedef int16_t2 I16Vec2;
constexpr uint kSizeof_I16Vec2 = 4u;
typedef int16_t3 I16Vec3;
constexpr uint kSizeof_I16Vec3 = 6u;
typedef int16_t4 I16Vec4;
constexpr uint kSizeof_I16Vec4 = 8u;
#	endif

typedef uint U32;
constexpr uint kSizeof_U32 = 4u;
typedef uint32_t2 UVec2;
constexpr uint kSizeof_UVec2 = 8u;
typedef uint32_t3 UVec3;
constexpr uint kSizeof_UVec3 = 12u;
typedef uint32_t4 UVec4;
constexpr uint kSizeof_UVec4 = 16u;

typedef int I32;
constexpr uint kSizeof_I32 = 4u;
typedef int32_t2 IVec2;
constexpr uint kSizeof_IVec2 = 8u;
typedef int32_t3 IVec3;
constexpr uint kSizeof_IVec3 = 12u;
typedef int32_t4 IVec4;
constexpr uint kSizeof_IVec4 = 16u;

#	if ANKI_SUPPORTS_64BIT_TYPES
typedef uint64_t U64;
constexpr uint kSizeof_U64 = 8u;
typedef uint64_t2 U64Vec2;
constexpr uint kSizeof_U64Vec2 = 16u;
typedef uint64_t3 U64Vec3;
constexpr uint kSizeof_U64Vec3 = 24u;
typedef uint64_t4 U64Vec4;
constexpr uint kSizeof_U64Vec4 = 32u;

typedef int64_t I64;
constexpr uint kSizeof_I64 = 8u;
typedef int64_t2 I64Vec2;
constexpr uint kSizeof_I64Vec2 = 16u;
typedef int64_t3 I64Vec3;
constexpr uint kSizeof_I64Vec3 = 24u;
typedef int64_t4 I64Vec4;
constexpr uint kSizeof_I64Vec4 = 32u;
#	endif

typedef bool Bool;

#	define _ANKI_DEFINE_OPERATOR_F32_ROWS3(mat, fl, op) \
		mat operator op(fl f) \
		{ \
			mat o; \
			o.m_row0 = m_row0 op f; \
			o.m_row1 = m_row1 op f; \
			o.m_row2 = m_row2 op f; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_F32_ROWS4(mat, fl, op) \
		mat operator op(fl f) \
		{ \
			mat o; \
			o.m_row0 = m_row0 op f; \
			o.m_row1 = m_row1 op f; \
			o.m_row2 = m_row2 op f; \
			o.m_row3 = m_row3 op f; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_SELF_ROWS3(mat, op) \
		mat operator op(mat b) \
		{ \
			mat o; \
			o.m_row0 = m_row0 op b.m_row0; \
			o.m_row1 = m_row1 op b.m_row1; \
			o.m_row2 = m_row2 op b.m_row2; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_SELF_ROWS4(mat, op) \
		mat operator op(mat b) \
		{ \
			mat o; \
			o.m_row0 = m_row0 op b.m_row0; \
			o.m_row1 = m_row1 op b.m_row1; \
			o.m_row2 = m_row2 op b.m_row2; \
			o.m_row3 = m_row3 op b.m_row3; \
			return o; \
		}

#	define _ANKI_DEFINE_ALL_OPERATORS_ROWS3(mat, fl) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS3(mat, fl, +) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS3(mat, fl, -) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS3(mat, fl, *) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS3(mat, fl, /) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS3(mat, +) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS3(mat, -)

#	define _ANKI_DEFINE_ALL_OPERATORS_ROWS4(mat, fl) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS4(mat, fl, +) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS4(mat, fl, -) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS4(mat, fl, *) \
		_ANKI_DEFINE_OPERATOR_F32_ROWS4(mat, fl, /) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS4(mat, +) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS4(mat, -)

struct Mat3
{
	Vec3 m_row0;
	Vec3 m_row1;
	Vec3 m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(Mat3, F32)
};

struct Mat4
{
	Vec4 m_row0;
	Vec4 m_row1;
	Vec4 m_row2;
	Vec4 m_row3;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS4(Mat4, F32)
};

struct Mat3x4
{
	Vec4 m_row0;
	Vec4 m_row1;
	Vec4 m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(Mat3x4, F32)
};

#	if ANKI_FORCE_FULL_FP_PRECISION
typedef float RF32;
typedef float2 RVec2;
typedef float3 RVec3;
typedef float4 RVec4;
typedef Mat3 RMat3;
#	else
typedef min16float RF32;
typedef min16float2 RVec2;
typedef min16float3 RVec3;
typedef min16float4 RVec4;

struct RMat3
{
	RVec3 m_row0;
	RVec3 m_row1;
	RVec3 m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(RMat3, RF32)
};
#	endif

// Matrix functions
Mat3 constructMatrixColumns(Vec3 c0, Vec3 c1, Vec3 c2)
{
	Mat3 m;
	m.m_row0 = Vec3(c0.x, c1.x, c2.x);
	m.m_row1 = Vec3(c0.y, c1.y, c2.y);
	m.m_row2 = Vec3(c0.z, c1.z, c2.z);
	return m;
}

RMat3 constructMatrixColumns(RVec3 c0, RVec3 c1, RVec3 c2)
{
	RMat3 m;
	m.m_row0 = RVec3(c0.x, c1.x, c2.x);
	m.m_row1 = RVec3(c0.y, c1.y, c2.y);
	m.m_row2 = RVec3(c0.z, c1.z, c2.z);
	return m;
}

Vec3 mul(Mat3 m, Vec3 v)
{
	const F32 a = dot(m.m_row0, v);
	const F32 b = dot(m.m_row1, v);
	const F32 c = dot(m.m_row2, v);
	return Vec3(a, b, c);
}

RVec3 mul(RMat3 m, RVec3 v)
{
	const RF32 a = dot(m.m_row0, v);
	const RF32 b = dot(m.m_row1, v);
	const RF32 c = dot(m.m_row2, v);
	return RVec3(a, b, c);
}

Vec4 mul(Mat4 m, Vec4 v)
{
	const F32 a = dot(m.m_row0, v);
	const F32 b = dot(m.m_row1, v);
	const F32 c = dot(m.m_row2, v);
	const F32 d = dot(m.m_row3, v);
	return Vec4(a, b, c, d);
}

Vec3 mul(Mat3x4 m, Vec4 v)
{
	const F32 a = dot(m.m_row0, v);
	const F32 b = dot(m.m_row1, v);
	const F32 c = dot(m.m_row2, v);
	return Vec3(a, b, c);
}

Mat3 transpose(Mat3 m)
{
	return constructMatrixColumns(m.m_row0, m.m_row1, m.m_row2);
}

// Common constants
constexpr F32 kEpsilonF32 = 0.000001f;
#	if ANKI_SUPPORTS_16BIT_TYPES
constexpr F16 kEpsilonhF16 = (F16)0.0001f; // Divisions by this should be OK according to http://weitz.de/ieee
#	endif
constexpr RF32 kEpsilonRF32 = 0.0001f;

constexpr U32 kMaxU32 = 0xFFFFFFFFu;
constexpr F32 kMaxF32 = 3.402823e+38;
#	if ANKI_SUPPORTS_16BIT_TYPES
constexpr F16 kMaxF16 = (F16)65504.0;
constexpr F16 kMinF16 = (F16)0.00006104;
#	endif

constexpr F32 kPi = 3.14159265358979323846f;

//! == GLSL ============================================================================================================
#else
#	define ANKI_HLSL 0
#	define ANKI_GLSL 1
#	define ANKI_CPP 0

#	define ANKI_BEGIN_NAMESPACE
#	define ANKI_END_NAMESPACE
#	define ANKI_SHADER_FUNC_INLINE

#	define ANKI_SHADER_STATIC_ASSERT(cond_)

#	define ScalarVec4 Vec4
#	define ScalarMat3x4 Mat3x4
#	define ScalarMat4 Mat4

#	define constexpr const

#	define ANKI_SUPPORTS_64BIT_TYPES !ANKI_PLATFORM_MOBILE

#	extension GL_EXT_control_flow_attributes : require
#	extension GL_KHR_shader_subgroup_vote : require
#	extension GL_KHR_shader_subgroup_ballot : require
#	extension GL_KHR_shader_subgroup_shuffle : require
#	extension GL_KHR_shader_subgroup_arithmetic : require

#	extension GL_EXT_samplerless_texture_functions : require
#	extension GL_EXT_shader_image_load_formatted : require
#	extension GL_EXT_nonuniform_qualifier : enable

#	extension GL_EXT_buffer_reference : enable
#	extension GL_EXT_buffer_reference2 : enable

#	extension GL_EXT_shader_explicit_arithmetic_types : enable
#	extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#	extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable
#	extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#	extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#	extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable

#	if ANKI_SUPPORTS_64BIT_TYPES
#		extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#		extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
#		extension GL_EXT_shader_atomic_int64 : enable
#		extension GL_EXT_shader_subgroup_extended_types_int64 : enable
#	endif

#	extension GL_EXT_nonuniform_qualifier : enable
#	extension GL_EXT_scalar_block_layout : enable

#	if defined(ANKI_RAY_GEN_SHADER) || defined(ANKI_ANY_HIT_SHADER) || defined(ANKI_CLOSEST_HIT_SHADER) \
		|| defined(ANKI_MISS_SHADER) || defined(ANKI_INTERSECTION_SHADER) || defined(ANKI_CALLABLE_SHADER)
#		extension GL_EXT_ray_tracing : enable
#	endif

#	define F32 float
const uint kSizeof_float = 4u;
#	define Vec2 vec2
const uint kSizeof_vec2 = 8u;
#	define Vec3 vec3
const uint kSizeof_vec3 = 12u;
#	define Vec4 vec4
const uint kSizeof_vec4 = 16u;

#	define F16 float16_t
const uint kSizeof_float16_t = 2u;
#	define HVec2 f16vec2
const uint kSizeof_f16vec2 = 4u;
#	define HVec3 f16vec3
const uint kSizeof_f16vec3 = 6u;
#	define HVec4 f16vec4
const uint kSizeof_f16vec4 = 8u;

#	define U8 uint8_t
const uint kSizeof_uint8_t = 1u;
#	define U8Vec2 u8vec2
const uint kSizeof_u8vec2 = 2u;
#	define U8Vec3 u8vec3
const uint kSizeof_u8vec3 = 3u;
#	define U8Vec4 u8vec4
const uint kSizeof_u8vec4 = 4u;

#	define I8 int8_t
const uint kSizeof_int8_t = 1u;
#	define I8Vec2 i8vec2
const uint kSizeof_i8vec2 = 2u;
#	define I8Vec3 i8vec3
const uint kSizeof_i8vec3 = 3u;
#	define I8Vec4 i8vec4
const uint kSizeof_i8vec4 = 4u;

#	define U16 uint16_t
const uint kSizeof_uint16_t = 2u;
#	define U16Vec2 u16vec2
const uint kSizeof_u16vec2 = 4u;
#	define U16Vec3 u16vec3
const uint kSizeof_u16vec3 = 6u;
#	define U16Vec4 u16vec4
const uint kSizeof_u16vec4 = 8u;

#	define I16 int16_t
const uint kSizeof_int16_t = 2u;
#	define I16Vec2 i16vec2
const uint kSizeof_i16vec2 = 4u;
#	define I16Vec3 i16vec3
const uint kSizeof_i16vec3 = 6u;
#	define i16Vec4 i16vec4
const uint kSizeof_i16vec4 = 8u;

#	define U32 uint
const uint kSizeof_uint = 4u;
#	define UVec2 uvec2
const uint kSizeof_uvec2 = 8u;
#	define UVec3 uvec3
const uint kSizeof_uvec3 = 12u;
#	define UVec4 uvec4
const uint kSizeof_uvec4 = 16u;

#	define I32 int
const uint kSizeof_int = 4u;
#	define IVec2 ivec2
const uint kSizeof_ivec2 = 8u;
#	define IVec3 ivec3
const uint kSizeof_ivec3 = 12u;
#	define IVec4 ivec4
const uint kSizeof_ivec4 = 16u;

#	if ANKI_SUPPORTS_64BIT_TYPES
#		define U64 uint64_t
const uint kSizeof_uint64_t = 8u;
#		define U64Vec2 u64vec2
const uint kSizeof_u64vec2 = 16u;
#		define U64Vec3 u64vec3
const uint kSizeof_u64vec3 = 24u;
#		define U64Vec4 u64vec4
const uint kSizeof_u64vec4 = 32u;

#		define I64 int64_t
const uint kSizeof_int64_t = 8u;
#		define I64Vec2 i64vec2
const uint kSizeof_i64vec2 = 16u;
#		define I64Vec3 i64vec3
const uint kSizeof_i64vec3 = 24u;
#		define I64Vec4 i64vec4
const uint kSizeof_i64vec4 = 32u;
#	endif

#	define Mat3 mat3
const uint kSizeof_mat3 = 36u;

#	define Mat4 mat4
const uint kSizeof_mat4 = 64u;

#	define Mat3x4 mat4x3 // GLSL has the column number first and then the rows
const uint kSizeof_mat4x3 = 48u;

#	define Bool bool

#	if ANKI_SUPPORTS_64BIT_TYPES
#		define Address U64
#	else
#		define Address UVec2
#	endif

#	if ANKI_FORCE_FULL_FP_PRECISION
#		define RF32 F32
#		define RVec2 Vec2
#		define RVec3 Vec3
#		define RVec4 Vec4
#		define RMat3 Mat3
#	else
#		define RF32 mediump F32
#		define RVec2 mediump Vec2
#		define RVec3 mediump Vec3
#		define RVec4 mediump Vec4
#		define RMat3 mediump Mat3
#	endif

#	define _ANKI_CONCATENATE(a, b) a##b
#	define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#	define sizeof(type) _ANKI_CONCATENATE(kSizeof_, type)
#	define alignof(type) _ANKI_CONCATENATE(kAlignof_, type)

#	define _ANKI_SCONST_X(type, n, id) layout(constant_id = id) const type n = type(1)

#	define _ANKI_SCONST_X2(type, componentType, n, id, constWorkaround) \
		layout(constant_id = id + 0u) const componentType ANKI_CONCATENATE(_anki_const_0_2_, n) = componentType(1); \
		layout(constant_id = id + 1u) const componentType ANKI_CONCATENATE(_anki_const_1_2_, n) = componentType(1); \
		constWorkaround type n = type(ANKI_CONCATENATE(_anki_const_0_2_, n), ANKI_CONCATENATE(_anki_const_1_2_, n))

#	define _ANKI_SCONST_X3(type, componentType, n, id, constWorkaround) \
		layout(constant_id = id + 0u) const componentType ANKI_CONCATENATE(_anki_const_0_3_, n) = componentType(1); \
		layout(constant_id = id + 1u) const componentType ANKI_CONCATENATE(_anki_const_1_3_, n) = componentType(1); \
		layout(constant_id = id + 2u) const componentType ANKI_CONCATENATE(_anki_const_2_3_, n) = componentType(1); \
		constWorkaround type n = type(ANKI_CONCATENATE(_anki_const_0_3_, n), ANKI_CONCATENATE(_anki_const_1_3_, n), \
									  ANKI_CONCATENATE(_anki_const_2_3_, n))

#	define _ANKI_SCONST_X4(type, componentType, n, id, constWorkaround) \
		layout(constant_id = id + 0u) const componentType ANKI_CONCATENATE(_anki_const_0_4_, n) = componentType(1); \
		layout(constant_id = id + 1u) const componentType ANKI_CONCATENATE(_anki_const_1_4_, n) = componentType(1); \
		layout(constant_id = id + 2u) const componentType ANKI_CONCATENATE(_anki_const_2_4_, n) = componentType(1); \
		layout(constant_id = id + 3u) const componentType ANKI_CONCATENATE(_anki_const_3_4_, n) = componentType(1); \
		constWorkaround type n = type(ANKI_CONCATENATE(_anki_const_0_4_, n), ANKI_CONCATENATE(_anki_const_1_4_, n), \
									  ANKI_CONCATENATE(_anki_const_2_4_, n), ANKI_CONCATENATE(_anki_const_2_4_, n))

#	define ANKI_SPECIALIZATION_CONSTANT_I32(n, id) _ANKI_SCONST_X(I32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_IVEC2(n, id) _ANKI_SCONST_X2(IVec2, I32, n, id, const)
#	define ANKI_SPECIALIZATION_CONSTANT_IVEC3(n, id) _ANKI_SCONST_X3(IVec3, I32, n, id, const)
#	define ANKI_SPECIALIZATION_CONSTANT_IVEC4(n, id) _ANKI_SCONST_X4(IVec4, I32, n, id, const)

#	define ANKI_SPECIALIZATION_CONSTANT_U32(n, id) _ANKI_SCONST_X(U32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_UVEC2(n, id) _ANKI_SCONST_X2(UVec2, U32, n, id, const)
#	define ANKI_SPECIALIZATION_CONSTANT_UVEC3(n, id) _ANKI_SCONST_X3(UVec3, U32, n, id, const)
#	define ANKI_SPECIALIZATION_CONSTANT_UVEC4(n, id) _ANKI_SCONST_X4(UVec4, U32, n, id, const)

#	define ANKI_SPECIALIZATION_CONSTANT_F32(n, id) _ANKI_SCONST_X(F32, n, id)
#	define ANKI_SPECIALIZATION_CONSTANT_VEC2(n, id) _ANKI_SCONST_X2(Vec2, F32, n, id, )
#	define ANKI_SPECIALIZATION_CONSTANT_VEC3(n, id) _ANKI_SCONST_X3(Vec3, F32, n, id, )
#	define ANKI_SPECIALIZATION_CONSTANT_VEC4(n, id) _ANKI_SCONST_X4(Vec4, F32, n, id, )

#	define ANKI_DEFINE_LOAD_STORE(type, alignment) \
		layout(buffer_reference, scalar, buffer_reference_align = (alignment)) buffer _Ref##type \
		{ \
			type m_value; \
		}; \
		void load(U64 address, out type o) \
		{ \
			o = _Ref##type(address).m_value; \
		} \
		void store(U64 address, type i) \
		{ \
			_Ref##type(address).m_value = i; \
		}

layout(std140, row_major) uniform;
layout(std140, row_major) buffer;

#	if ANKI_FORCE_FULL_FP_PRECISION
#		define ANKI_RP
#	else
#		define ANKI_RP mediump
#	endif

#	define ANKI_FP highp

precision highp int;
precision highp float;

#	define ANKI_BINDLESS_SET(s) \
		layout(set = s, binding = 0) uniform utexture2D u_bindlessTextures2dU32[kMaxBindlessTextures]; \
		layout(set = s, binding = 0) uniform itexture2D u_bindlessTextures2dI32[kMaxBindlessTextures]; \
		layout(set = s, binding = 0) uniform texture2D u_bindlessTextures2dF32[kMaxBindlessTextures]; \
		layout(set = s, binding = 0) uniform texture2DArray u_bindlessTextures2dArrayF32[kMaxBindlessTextures]; \
		layout(set = s, binding = 1) uniform textureBuffer u_bindlessTextureBuffers[kMaxBindlessReadonlyTextureBuffers];

Vec2 pow(Vec2 a, F32 b)
{
	return pow(a, Vec2(b));
}

Vec3 pow(Vec3 a, F32 b)
{
	return pow(a, Vec3(b));
}

Vec4 pow(Vec4 a, F32 b)
{
	return pow(a, Vec4(b));
}

Bool all(Bool b)
{
	return b;
}

Bool any(Bool b)
{
	return b;
}

#	define saturate(x_) clamp((x_), 0.0, 1.0)
#	define saturateRp(x) min(x, F32(kMaxF16))
#	define mad(a_, b_, c_) fma((a_), (b_), (c_))
#	define frac(x) fract(x)
#	define lerp(a, b, t) mix(a, b, t)
#	define atan2(x, y) atan(x, y)

float asfloat(uint u)
{
	return uintBitsToFloat(u);
}

constexpr F32 kEpsilonf = 0.000001f;
constexpr F16 kEpsilonhf = 0.0001hf; // Divisions by this should be OK according to http://weitz.de/ieee/
constexpr ANKI_RP F32 kEpsilonRp = F32(kEpsilonhf);

constexpr U32 kMaxU32 = 0xFFFFFFFFu;
constexpr F32 kMaxF32 = 3.402823e+38;
constexpr F16 kMaxF16 = 65504.0hf;
constexpr F16 kMinF16 = 0.00006104hf;

constexpr F32 kPi = 3.14159265358979323846f;
#endif

//! == Consts ==========================================================================================================
ANKI_BEGIN_NAMESPACE

/// The renderer will group drawcalls into instances up to this number.
constexpr U32 kMaxInstanceCount = 64u;

constexpr U32 kMaxLodCount = 3u;
constexpr U32 kMaxShadowCascades = 4u;

constexpr F32 kShadowsPolygonOffsetFactor = 1.25f;
constexpr F32 kShadowsPolygonOffsetUnits = 2.75f;

ANKI_END_NAMESPACE
