// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

//! == C++ =============================================================================================================
#if defined(__cplusplus)

#	include <AnKi/Math.h>

#	define ANKI_HLSL 0
#	define ANKI_GLSL 0
#	define ANKI_CPP 1

#	define ANKI_BEGIN_NAMESPACE namespace anki {
#	define ANKI_END_NAMESPACE }

#	define ANKI_ARRAY(type, size, name) Array<type, U32(size)> name

#	define ANKI_CPP_CODE(...) __VA_ARGS__

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

#	if defined(__spirv__)
#		define ANKI_GR_BACKEND_VULKAN 1
#		define ANKI_GR_BACKEND_DIRECT3D 0
#	else
#		define ANKI_GR_BACKEND_VULKAN 0
#		define ANKI_GR_BACKEND_DIRECT3D 1
#	endif

#	define ANKI_BEGIN_NAMESPACE
#	define ANKI_END_NAMESPACE
#	define inline

#	define ANKI_ARRAY(type, size, name) type name[(U32)size]

#	define ANKI_CPP_CODE(...)

#	define ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enum_)

#	define constexpr static const

#	if defined(ANKI_ASSERTIONS_ENABLED) && ANKI_ASSERTIONS_ENABLED == 1 && ANKI_GR_BACKEND_VULKAN
#		define ANKI_ASSERT(x) \
			if(!(x)) \
			printf("Assertion failed. (" __FILE__ ":%i)", __LINE__)
#	else
#		define ANKI_ASSERT(x)
#	endif

template<typename T>
void maybeUnused(T a)
{
	a = a;
}
#	define ANKI_MAYBE_UNUSED(x) maybeUnused(x)

#	define _ANKI_CONCATENATE(a, b) a##b
#	define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#	define static_assert(x)

#	define _ANKI_SCONST_X(type, n, id) [[vk::constant_id(id)]] const type n = (type)1;

#	define _ANKI_SCONST_X2(type, componentType, n, id) \
		[[vk::constant_id(id + 0u)]] const componentType ANKI_CONCATENATE(_anki_const_0_2_, n) = (componentType)1; \
		[[vk::constant_id(id + 1u)]] const componentType ANKI_CONCATENATE(_anki_const_1_2_, n) = (componentType)1; \
		static const type n = type(ANKI_CONCATENATE(_anki_const_0_2_, n), ANKI_CONCATENATE(_anki_const_1_2_, n))

#	define _ANKI_SCONST_X3(type, componentType, n, id) \
		[[vk::constant_id(id + 0u)]] const componentType ANKI_CONCATENATE(_anki_const_0_3_, n) = (componentType)1; \
		[[vk::constant_id(id + 1u)]] const componentType ANKI_CONCATENATE(_anki_const_1_3_, n) = (componentType)1; \
		[[vk::constant_id(id + 2u)]] const componentType ANKI_CONCATENATE(_anki_const_2_3_, n) = (componentType)1; \
		static const type n = \
			type(ANKI_CONCATENATE(_anki_const_0_3_, n), ANKI_CONCATENATE(_anki_const_1_3_, n), ANKI_CONCATENATE(_anki_const_2_3_, n))

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

typedef bool Bool;

#	define _ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(mat, fl, op) \
		mat operator op(fl f) \
		{ \
			mat o; \
			o.m_row0 = m_row0 op f; \
			o.m_row1 = m_row1 op f; \
			o.m_row2 = m_row2 op f; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(mat, fl, op) \
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
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(mat, fl, +) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(mat, fl, -) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(mat, fl, *) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(mat, fl, /) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS3(mat, +) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS3(mat, -)

#	define _ANKI_DEFINE_ALL_OPERATORS_ROWS4(mat, fl) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(mat, fl, +) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(mat, fl, -) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(mat, fl, *) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(mat, fl, /) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS4(mat, +) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS4(mat, -)

// Mat3 "template". Not an actual template because of bugs
#	define _ANKI_MAT3(mat, vec, scalar) \
		struct mat \
		{ \
			vec m_row0; \
			vec m_row1; \
			vec m_row2; \
			_ANKI_DEFINE_ALL_OPERATORS_ROWS3(mat, scalar) \
			void setColumns(vec c0, vec c1, vec c2) \
			{ \
				m_row0 = vec(c0.x, c1.x, c2.x); \
				m_row1 = vec(c0.y, c1.y, c2.y); \
				m_row2 = vec(c0.z, c1.z, c2.z); \
			} \
		}; \
		vec mul(mat m, vec v) \
		{ \
			const scalar a = dot(m.m_row0, v); \
			const scalar b = dot(m.m_row1, v); \
			const scalar c = dot(m.m_row2, v); \
			return vec(a, b, c); \
		} \
		mat transpose(mat m) \
		{ \
			mat o; \
			o.setColumns(m.m_row0, m.m_row1, m.m_row2); \
			return o; \
		}

// Mat4 "template". Not an actual template because of bugs
#	define _ANKI_MAT4(mat, vec, scalar) \
		struct mat \
		{ \
			vec m_row0; \
			vec m_row1; \
			vec m_row2; \
			vec m_row3; \
			_ANKI_DEFINE_ALL_OPERATORS_ROWS4(mat, scalar) \
			vec getTranslationPart() \
			{ \
				return vec(m_row0.w, m_row1.w, m_row2.w, m_row3.w); \
			} \
			void setColumns(vec c0, vec c1, vec c2, vec c3) \
			{ \
				m_row0 = vec(c0.x, c1.x, c2.x, c3.x); \
				m_row1 = vec(c0.y, c1.y, c2.y, c3.y); \
				m_row2 = vec(c0.z, c1.z, c2.z, c3.z); \
				m_row3 = vec(c0.w, c1.w, c2.w, c3.w); \
			} \
		}; \
		vec mul(mat m, vec v) \
		{ \
			const scalar a = dot(m.m_row0, v); \
			const scalar b = dot(m.m_row1, v); \
			const scalar c = dot(m.m_row2, v); \
			const scalar d = dot(m.m_row3, v); \
			return vec(a, b, c, d); \
		} \
		mat mul(mat a_, mat b_) \
		{ \
			const vec a[4] = {a_.m_row0, a_.m_row1, a_.m_row2, a_.m_row3}; \
			const vec b[4] = {b_.m_row0, b_.m_row1, b_.m_row2, b_.m_row3}; \
			vec c[4]; \
			[unroll] for(U32 i = 0; i < 4; i++) \
			{ \
				vec t1, t2; \
				t1 = a[i][0]; \
				t2 = b[0] * t1; \
				t1 = a[i][1]; \
				t2 += b[1] * t1; \
				t1 = a[i][2]; \
				t2 += b[2] * t1; \
				t1 = a[i][3]; \
				t2 += b[3] * t1; \
				c[i] = t2; \
			} \
			mat o; \
			o.m_row0 = c[0]; \
			o.m_row1 = c[1]; \
			o.m_row2 = c[2]; \
			o.m_row3 = c[3]; \
			return o; \
		}

// Mat3x4 "template". Not an actual template because of bugs
#	define _ANKI_MAT3x4(mat, row, column, scalar) \
		struct mat \
		{ \
			row m_row0; \
			row m_row1; \
			row m_row2; \
			_ANKI_DEFINE_ALL_OPERATORS_ROWS3(mat, scalar) \
			column getTranslationPart() \
			{ \
				return column(m_row0.w, m_row1.w, m_row2.w); \
			} \
			void setColumns(column c0, column c1, column c2, column c3) \
			{ \
				m_row0 = row(c0.x, c1.x, c2.x, c3.x); \
				m_row1 = row(c0.y, c1.y, c2.y, c3.y); \
				m_row2 = row(c0.z, c1.z, c2.z, c3.z); \
			} \
			void setColumn(U32 i, column c) \
			{ \
				m_row0[i] = c.x; \
				m_row1[i] = c.y; \
				m_row2[i] = c.z; \
			} \
		}; \
		column mul(mat m, row v) \
		{ \
			const scalar a = dot(m.m_row0, v); \
			const scalar b = dot(m.m_row1, v); \
			const scalar c = dot(m.m_row2, v); \
			return column(a, b, c); \
		} \
		mat combineTransformations(mat a_, mat b_) \
		{ \
			const row a[3] = {a_.m_row0, a_.m_row1, a_.m_row2}; \
			const row b[3] = {b_.m_row0, b_.m_row1, b_.m_row2}; \
			row c[3]; \
			[unroll] for(U32 i = 0; i < 3; i++) \
			{ \
				row t2; \
				t2 = b[0] * a[i][0]; \
				t2 += b[1] * a[i][1]; \
				t2 += b[2] * a[i][2]; \
				const row v4 = row(0.0f, 0.0f, 0.0f, a[i][3]); \
				t2 += v4; \
				c[i] = t2; \
			} \
			mat o; \
			o.m_row0 = c[0]; \
			o.m_row1 = c[1]; \
			o.m_row2 = c[2]; \
			return o; \
		}

_ANKI_MAT3(Mat3, Vec3, F32)
_ANKI_MAT4(Mat4, Vec4, F32)
_ANKI_MAT3x4(Mat3x4, Vec4, Vec3, F32)

#	if ANKI_SUPPORTS_16BIT_TYPES == 0
#		if ANKI_FORCE_FULL_FP_PRECISION
	typedef float RF32;
typedef float2 RVec2;
typedef float3 RVec3;
typedef float4 RVec4;
_ANKI_MAT3(RMat3, Vec3, F32)
#		else
	typedef min16float RF32;
typedef min16float2 RVec2;
typedef min16float3 RVec3;
typedef min16float4 RVec4;
_ANKI_MAT3(RMat3, RVec3, RF32)
#		endif
#	else // ANKI_SUPPORTS_16BIT_TYPES == 0
	_ANKI_MAT3(HMat3, HVec3, F16)
#	endif // ANKI_SUPPORTS_16BIT_TYPES == 0

#endif // defined(__HLSL_VERSION)

//! == Common ==========================================================================================================
ANKI_BEGIN_NAMESPACE

constexpr U32 kMaxLodCount = 3u;
constexpr U32 kMaxShadowCascades = 4u;

constexpr U32 kIndirectDiffuseClipmapCount = 3u;
/// Bias applied to the camera to push the clipmaps further into the looking direction.
constexpr Vec3 kIndirectDiffuseClipmapForwardBias = Vec3(20.0f, 2.0f, 20.0f);

constexpr F32 kShadowsPolygonOffsetFactor = 1.25f;
constexpr F32 kShadowsPolygonOffsetUnits = 2.75f;

#if defined(__HLSL_VERSION)
constexpr U32 kMaxMipsSinglePassDownsamplerCanProduce = 12u;
#else
constexpr U8 kMaxMipsSinglePassDownsamplerCanProduce = 12u;
#endif

constexpr U32 kMaxPrimitivesPerMeshlet = 124; ///< nVidia prefers 126 but meshoptimizer choks with that value.
constexpr U32 kMaxVerticesPerMeshlet = 128;
#define ANKI_TASK_SHADER_THREADGROUP_SIZE 64u
constexpr U32 kMeshletGroupSize = ANKI_TASK_SHADER_THREADGROUP_SIZE;

#define ANKI_MESH_SHADER_THREADGROUP_SIZE 32u
static_assert(kMaxVerticesPerMeshlet % ANKI_MESH_SHADER_THREADGROUP_SIZE == 0);

constexpr F32 kPcfTexelRadius = 4.0f;
constexpr F32 kPcssSearchTexelRadius = 12.0;
constexpr F32 kPcssTexelRadius = 12.0;
constexpr F32 kPcssDirLightMaxPenumbraMeters = 6.0; // If the occluder and the reciever have more than this value then do full penumbra

struct DrawIndirectArgs
{
	U32 m_vertexCount;
	U32 m_instanceCount;
	U32 m_firstVertex;
	U32 m_firstInstance;

#if defined(__cplusplus)
	DrawIndirectArgs()
		: m_vertexCount(kMaxU32)
		, m_instanceCount(1)
		, m_firstVertex(0)
		, m_firstInstance(0)
	{
	}

	DrawIndirectArgs(const DrawIndirectArgs&) = default;

	DrawIndirectArgs(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
		: m_vertexCount(count)
		, m_instanceCount(instanceCount)
		, m_firstVertex(first)
		, m_firstInstance(baseInstance)
	{
	}

	Bool operator==(const DrawIndirectArgs& b) const
	{
		return m_vertexCount == b.m_vertexCount && m_instanceCount == b.m_instanceCount && m_firstVertex == b.m_firstVertex
			   && m_firstInstance == b.m_firstInstance;
	}

	Bool operator!=(const DrawIndirectArgs& b) const
	{
		return !(operator==(b));
	}
#endif
};

struct DrawIndexedIndirectArgs
{
	U32 m_indexCount;
	U32 m_instanceCount;
	U32 m_firstIndex;
	I32 m_vertexOffset;
	U32 m_firstInstance;

#if defined(__cplusplus)
	DrawIndexedIndirectArgs()
		: m_indexCount(kMaxU32)
		, m_instanceCount(1)
		, m_firstIndex(0)
		, m_vertexOffset(0)
		, m_firstInstance(0)
	{
	}

	DrawIndexedIndirectArgs(const DrawIndexedIndirectArgs&) = default;

	DrawIndexedIndirectArgs(U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
		: m_indexCount(count)
		, m_instanceCount(instanceCount)
		, m_firstIndex(firstIndex)
		, m_vertexOffset(baseVertex)
		, m_firstInstance(baseInstance)
	{
	}

	Bool operator==(const DrawIndexedIndirectArgs& b) const
	{
		return m_indexCount == b.m_indexCount && m_instanceCount == b.m_instanceCount && m_firstIndex == b.m_firstIndex
			   && m_vertexOffset == b.m_vertexOffset && m_firstInstance == b.m_firstInstance;
	}

	Bool operator!=(const DrawIndexedIndirectArgs& b) const
	{
		return !(operator==(b));
	}
#endif
};

struct DispatchIndirectArgs
{
	U32 m_threadGroupCountX;
	U32 m_threadGroupCountY;
	U32 m_threadGroupCountZ;
};

/// Mirrors VkGeometryInstanceFlagBitsKHR
enum AccellerationStructureFlag : U32
{
	kAccellerationStructureFlagTriangleFacingCullDisable = 1 << 0,
	kAccellerationStructureFlagFlipFacing = 1 << 1,
	kAccellerationStructureFlagForceOpaque = 1 << 2,
	kAccellerationStructureFlagForceNoOpaque = 1 << 3,
	kAccellerationStructureFlagTriangleFrontCounterlockwise = kAccellerationStructureFlagFlipFacing
};

/// Mirrors VkAccelerationStructureInstanceKHR and D3D12_RAYTRACING_INSTANCE_DESC.
struct AccelerationStructureInstance
{
	Mat3x4 m_transform;
	U32 m_instanceCustomIndex : 24; ///< Custom value that can be accessed in the shaders.
	U32 m_mask : 8;
	U32 m_instanceShaderBindingTableRecordOffset : 24;
	U32 m_flags : 8; ///< It's AccellerationStructureFlag.
#if defined(__cplusplus)
	U64 m_accelerationStructureAddress;
#else
	UVec2 m_accelerationStructureAddress;
#endif
};

ANKI_END_NAMESPACE
