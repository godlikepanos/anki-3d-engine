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

#	pragma pack_matrix(row_major)

typedef float F32;
typedef float2 Vec2;
typedef float3 Vec3;
typedef float4 Vec4;

typedef float16_t F16;
typedef float16_t2 HVec2;
typedef float16_t3 HVec3;
typedef float16_t4 HVec4;

typedef uint16_t U16;
typedef uint16_t2 U16Vec2;
typedef uint16_t3 U16Vec3;
typedef uint16_t4 U16Vec4;

typedef int16_t I16;
typedef int16_t2 I16Vec2;
typedef int16_t3 I16Vec3;
typedef int16_t4 I16Vec4;

typedef uint U32;
typedef uint32_t2 UVec2;
typedef uint32_t3 UVec3;
typedef uint32_t4 UVec4;

typedef int I32;
typedef int32_t2 IVec2;
typedef int32_t3 IVec3;
typedef int32_t4 IVec4;

typedef uint64_t U64;
typedef uint64_t2 U64Vec2;
typedef uint64_t3 U64Vec3;
typedef uint64_t4 U64Vec4;

typedef int64_t I64;
typedef int64_t2 I64Vec2;
typedef int64_t3 I64Vec3;
typedef int64_t4 I64Vec4;

typedef bool Bool;

#	define _ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, op) \
		matType operator op(scalarType f) \
		{ \
			matType o; \
			o.m_row0 = m_row0 op f; \
			o.m_row1 = m_row1 op f; \
			o.m_row2 = m_row2 op f; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, op) \
		matType operator op(scalarType f) \
		{ \
			matType o; \
			o.m_row0 = m_row0 op f; \
			o.m_row1 = m_row1 op f; \
			o.m_row2 = m_row2 op f; \
			o.m_row3 = m_row3 op f; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_SELF_ROWS3(matType, op) \
		matType operator op(matType b) \
		{ \
			matType o; \
			o.m_row0 = m_row0 op b.m_row0; \
			o.m_row1 = m_row1 op b.m_row1; \
			o.m_row2 = m_row2 op b.m_row2; \
			return o; \
		}

#	define _ANKI_DEFINE_OPERATOR_SELF_ROWS4(matType, op) \
		matType operator op(matType b) \
		{ \
			matType o; \
			o.m_row0 = m_row0 op b.m_row0; \
			o.m_row1 = m_row1 op b.m_row1; \
			o.m_row2 = m_row2 op b.m_row2; \
			o.m_row3 = m_row3 op b.m_row3; \
			return o; \
		}

#	define _ANKI_DEFINE_ALL_OPERATORS_ROWS3(matType, scalarType) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, +) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, -) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, *) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, /) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS3(matType, +) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS3(matType, -)

#	define _ANKI_DEFINE_ALL_OPERATORS_ROWS4(matType, scalarType) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, +) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, -) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, *) \
		_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, /) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS4(matType, +) \
		_ANKI_DEFINE_OPERATOR_SELF_ROWS4(matType, -)

struct Mat3
{
	Vec3 m_row0;
	Vec3 m_row1;
	Vec3 m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(Mat3, F32)

	void setColumns(Vec3 c0, Vec3 c1, Vec3 c2)
	{
		m_row0 = Vec3(c0.x, c1.x, c2.x);
		m_row1 = Vec3(c0.y, c1.y, c2.y);
		m_row2 = Vec3(c0.z, c1.z, c2.z);
	}
};

Vec3 mul(Mat3 m, Vec3 v)
{
	const F32 a = dot(m.m_row0, v);
	const F32 b = dot(m.m_row1, v);
	const F32 c = dot(m.m_row2, v);
	return Vec3(a, b, c);
}

Mat3 transpose(Mat3 m)
{
	Mat3 o;
	o.setColumns(m.m_row0, m.m_row1, m.m_row2);
	return o;
}

struct Mat4
{
	Vec4 m_row0;
	Vec4 m_row1;
	Vec4 m_row2;
	Vec4 m_row3;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS4(Mat4, F32)

	void setColumns(Vec4 c0, Vec4 c1, Vec4 c2, Vec4 c3)
	{
		m_row0 = Vec4(c0.x, c1.x, c2.x, c3.x);
		m_row1 = Vec4(c0.y, c1.y, c2.y, c3.y);
		m_row2 = Vec4(c0.z, c1.z, c2.z, c3.z);
		m_row3 = Vec4(c0.w, c1.w, c2.w, c3.w);
	}

	Vec4 getTranslationPart()
	{
		return Vec4(m_row0.w, m_row1.w, m_row2.w, m_row3.w);
	}
};

Vec4 mul(Mat4 m, Vec4 v)
{
	const F32 a = dot(m.m_row0, v);
	const F32 b = dot(m.m_row1, v);
	const F32 c = dot(m.m_row2, v);
	const F32 d = dot(m.m_row3, v);
	return Vec4(a, b, c, d);
}

Mat4 mul(Mat4 a_, Mat4 b_)
{
	const Vec4 a[4] = {a_.m_row0, a_.m_row1, a_.m_row2, a_.m_row3};
	const Vec4 b[4] = {b_.m_row0, b_.m_row1, b_.m_row2, b_.m_row3};
	Vec4 c[4];
	[unroll] for(U32 i = 0; i < 4; i++)
	{
		Vec4 t1, t2;
		t1 = a[i][0];
		t2 = b[0] * t1;
		t1 = a[i][1];
		t2 += b[1] * t1;
		t1 = a[i][2];
		t2 += b[2] * t1;
		t1 = a[i][3];
		t2 += b[3] * t1;
		c[i] = t2;
	}
	Mat4 o;
	o.m_row0 = c[0];
	o.m_row1 = c[1];
	o.m_row2 = c[2];
	o.m_row3 = c[3];
	return o;
}

struct Mat3x4
{
	Vec4 m_row0;
	Vec4 m_row1;
	Vec4 m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(Mat3x4, F32)

	Vec3 getTranslationPart()
	{
		return Vec3(m_row0.w, m_row1.w, m_row2.w);
	}

	void setColumns(Vec3 c0, Vec3 c1, Vec3 c2, Vec3 c3)
	{
		m_row0 = Vec4(c0.x, c1.x, c2.x, c3.x);
		m_row1 = Vec4(c0.y, c1.y, c2.y, c3.y);
		m_row2 = Vec4(c0.z, c1.z, c2.z, c3.z);
	}

	void setColumn(U32 i, Vec3 c)
	{
		m_row0[i] = c.x;
		m_row1[i] = c.y;
		m_row2[i] = c.z;
	}

	Vec3 getColumn(U32 i)
	{
		return Vec3(m_row0[i], m_row1[i], m_row2[i]);
	}
};

Vec3 mul(Mat3x4 m, Vec4 v)
{
	const F32 a = dot(m.m_row0, v);
	const F32 b = dot(m.m_row1, v);
	const F32 c = dot(m.m_row2, v);
	return Vec3(a, b, c);
}

Mat3x4 combineTransformations(Mat3x4 a_, Mat3x4 b_)
{
	const Vec4 a[3] = {a_.m_row0, a_.m_row1, a_.m_row2};
	const Vec4 b[3] = {b_.m_row0, b_.m_row1, b_.m_row2};
	Vec4 c[3];
	[unroll] for(U32 i = 0; i < 3; i++)
	{
		Vec4 t2;
		t2 = b[0] * a[i][0];
		t2 += b[1] * a[i][1];
		t2 += b[2] * a[i][2];
		const Vec4 v4 = Vec4(0.0f, 0.0f, 0.0f, a[i][3]);
		t2 += v4;
		c[i] = t2;
	}
	Mat3x4 o;
	o.m_row0 = c[0];
	o.m_row1 = c[1];
	o.m_row2 = c[2];
	return o;
}

template<typename TMat>
Vec3 extractScale(TMat trf)
{
	Vec3 scale;
	[unroll] for(U32 i = 0; i < 3; ++i)
	{
		const Vec3 axis = Vec3(trf.m_row0[i], trf.m_row1[i], trf.m_row2[i]);
		scale[i] = length(axis);
	}

	return scale;
}

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

// Some special spaces or sets for reflection to identify special shader input
#define ANKI_D3D_FAST_CONSTANTS_SPACE 3000
#define ANKI_D3D_SHADER_RECORD_CONSTANTS_SPACE 3001
#define ANKI_D3D_DRAW_ID_CONSTANT_SPACE 3002
#define ANKI_VK_BINDLESS_TEXTURES_DESCRIPTOR_SET 1000000

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
