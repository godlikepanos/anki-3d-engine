// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/Array.h>

namespace anki {

/// @addtogroup graphics
/// @{
enum class ColorBit : U8
{
	NONE = 0,
	RED = 1 << 0,
	GREEN = 1 << 1,
	BLUE = 1 << 2,
	ALPHA = 1 << 3,
	ALL = RED | GREEN | BLUE | ALPHA
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ColorBit)

enum class PrimitiveTopology : U8
{
	POINTS,
	LINES,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	PATCHES
};

enum class FillMode : U8
{
	POINTS,
	WIREFRAME,
	SOLID,
	COUNT
};

enum class FaceSelectionBit : U8
{
	NONE = 0,
	FRONT = 1 << 0,
	BACK = 1 << 1,
	FRONT_AND_BACK = FRONT | BACK
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FaceSelectionBit)

enum class CompareOperation : U8
{
	ALWAYS,
	LESS,
	EQUAL,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	NOT_EQUAL,
	NEVER,
	COUNT
};

enum class StencilOperation : U8
{
	KEEP,
	ZERO,
	REPLACE,
	INCREMENT_AND_CLAMP,
	DECREMENT_AND_CLAMP,
	INVERT,
	INCREMENT_AND_WRAP,
	DECREMENT_AND_WRAP,
	COUNT
};

enum class BlendFactor : U8
{
	ZERO,
	ONE,
	SRC_COLOR,
	ONE_MINUS_SRC_COLOR,
	DST_COLOR,
	ONE_MINUS_DST_COLOR,
	SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA,
	DST_ALPHA,
	ONE_MINUS_DST_ALPHA,
	CONSTANT_COLOR,
	ONE_MINUS_CONSTANT_COLOR,
	CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA,
	SRC_ALPHA_SATURATE,
	SRC1_COLOR,
	ONE_MINUS_SRC1_COLOR,
	SRC1_ALPHA,
	ONE_MINUS_SRC1_ALPHA,
	COUNT
};

enum class BlendOperation : U8
{
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX,
	COUNT
};

enum class VertexStepRate : U8
{
	VERTEX,
	INSTANCE,
	DRAW,
	COUNT
};

/// A way to distinguish the aspect of a depth stencil texture.
enum class DepthStencilAspectBit : U8
{
	NONE = 0,
	DEPTH = 1 << 0,
	STENCIL = 1 << 1,
	DEPTH_STENCIL = DEPTH | STENCIL
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DepthStencilAspectBit)

/// Pixel or vertex format.
/// WARNING: Keep it the same as vulkan (one conversion less).
enum class Format : U32
{
	NONE = 0,

#define ANKI_FORMAT_DEF(type, id, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, \
						depthStencil) \
	type = id,
#include <AnKi/Gr/Format.defs.h>
#undef ANKI_FORMAT_DEF

	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Format)

/// Contains info for a specific Format.
class FormatInfo
{
public:
	U8 m_componentCount; ///< The number of components.
	U8 m_texelSize; ///< The size of the texel. Only for incompressed, zero for compressed.
	U8 m_blockWidth; ///< The width of the block size of compressed formats. Zero otherwise.
	U8 m_blockHeight; ///< The height of the block size of compressed formats. Zero otherwise.
	U8 m_blockSize; ///< The size of the block of a compressed format. Zero otherwise.
	U8 m_shaderType; ///< It's 0 if the shader sees it as float, 1 if uint and 2 if signed int.
	DepthStencilAspectBit m_depthStencil; ///< Depth/stencil mask.
	const char* m_name;

	Bool isDepthStencil() const
	{
		return m_depthStencil != DepthStencilAspectBit::NONE;
	}

	Bool isDepth() const
	{
		return !!(m_depthStencil & DepthStencilAspectBit::DEPTH);
	}

	Bool isStencil() const
	{
		return !!(m_depthStencil & DepthStencilAspectBit::STENCIL);
	}

	Bool isCompressed() const
	{
		return m_blockSize > 0;
	}
};

/// Get info for a specific Format.
ANKI_PURE const FormatInfo& getFormatInfo(Format fmt);

/// Texture type.
enum class TextureType : U8
{
	_1D,
	_2D,
	_3D,
	_2D_ARRAY,
	CUBE,
	CUBE_ARRAY,
	COUNT
};

inline Bool textureTypeIsCube(const TextureType t)
{
	return t == TextureType::CUBE || t == TextureType::CUBE_ARRAY;
}

/// Texture usage hints. They are very important.
enum class TextureUsageBit : U32
{
	NONE = 0,

	SAMPLED_GEOMETRY = 1 << 0,
	SAMPLED_FRAGMENT = 1 << 1,
	SAMPLED_COMPUTE = 1 << 2,
	SAMPLED_TRACE_RAYS = 1 << 3,

	IMAGE_GEOMETRY_READ = 1 << 4,
	IMAGE_GEOMETRY_WRITE = 1 << 5,
	IMAGE_FRAGMENT_READ = 1 << 6,
	IMAGE_FRAGMENT_WRITE = 1 << 7,
	IMAGE_COMPUTE_READ = 1 << 8,
	IMAGE_COMPUTE_WRITE = 1 << 9,
	IMAGE_TRACE_RAYS_READ = 1 << 10,
	IMAGE_TRACE_RAYS_WRITE = 1 << 11,

	FRAMEBUFFER_ATTACHMENT_READ = 1 << 12,
	FRAMEBUFFER_ATTACHMENT_WRITE = 1 << 13,
	FRAMEBUFFER_SHADING_RATE = 1 << 14,

	TRANSFER_DESTINATION = 1 << 15,
	GENERATE_MIPMAPS = 1 << 16,

	PRESENT = 1 << 17,

	// Derived
	ALL_SAMPLED = SAMPLED_GEOMETRY | SAMPLED_FRAGMENT | SAMPLED_COMPUTE | SAMPLED_TRACE_RAYS,
	ALL_IMAGE = IMAGE_GEOMETRY_READ | IMAGE_GEOMETRY_WRITE | IMAGE_FRAGMENT_READ | IMAGE_FRAGMENT_WRITE
				| IMAGE_COMPUTE_READ | IMAGE_COMPUTE_WRITE | IMAGE_TRACE_RAYS_READ | IMAGE_TRACE_RAYS_WRITE,
	ALL_FRAMEBUFFER_ATTACHMENT = FRAMEBUFFER_ATTACHMENT_READ | FRAMEBUFFER_ATTACHMENT_WRITE,

	ALL_GRAPHICS = SAMPLED_GEOMETRY | SAMPLED_FRAGMENT | IMAGE_GEOMETRY_READ | IMAGE_GEOMETRY_WRITE
				   | IMAGE_FRAGMENT_READ | IMAGE_FRAGMENT_WRITE | FRAMEBUFFER_ATTACHMENT_READ
				   | FRAMEBUFFER_ATTACHMENT_WRITE | FRAMEBUFFER_SHADING_RATE,
	ALL_COMPUTE = SAMPLED_COMPUTE | IMAGE_COMPUTE_READ | IMAGE_COMPUTE_WRITE,
	ALL_TRANSFER = TRANSFER_DESTINATION | GENERATE_MIPMAPS,

	ALL_READ = ALL_SAMPLED | IMAGE_GEOMETRY_READ | IMAGE_FRAGMENT_READ | IMAGE_COMPUTE_READ | IMAGE_TRACE_RAYS_READ
			   | FRAMEBUFFER_ATTACHMENT_READ | FRAMEBUFFER_SHADING_RATE | PRESENT | GENERATE_MIPMAPS,
	ALL_WRITE = IMAGE_GEOMETRY_WRITE | IMAGE_FRAGMENT_WRITE | IMAGE_COMPUTE_WRITE | IMAGE_TRACE_RAYS_WRITE
				| FRAMEBUFFER_ATTACHMENT_WRITE | TRANSFER_DESTINATION | GENERATE_MIPMAPS
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureUsageBit)

enum class SamplingFilter : U8
{
	NEAREST,
	LINEAR,
	MIN, ///< It calculates the min of a 2x2 quad. Only if GpuDeviceCapabilities::m_samplingFilterMinMax is supported.
	MAX, ///< It calculates the max of a 2x2 quad. Only if GpuDeviceCapabilities::m_samplingFilterMinMax is supported.
	BASE ///< Only for mipmaps.
};

enum class SamplingAddressing : U8
{
	CLAMP,
	REPEAT,
	BLACK,
	WHITE,

	COUNT,
	FIRST = 0,
	LAST = COUNT - 1,
};

enum class ShaderType : U16
{
	VERTEX,
	TESSELLATION_CONTROL,
	TESSELLATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,
	RAY_GEN,
	ANY_HIT,
	CLOSEST_HIT,
	MISS,
	INTERSECTION,
	CALLABLE,

	COUNT,
	FIRST = 0,
	LAST = COUNT - 1,
	FIRST_GRAPHICS = VERTEX,
	LAST_GRAPHICS = FRAGMENT,
	FIRST_RAY_TRACING = RAY_GEN,
	LAST_RAY_TRACING = CALLABLE,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderType)

enum class ShaderTypeBit : U16
{
	VERTEX = 1 << 0,
	TESSELLATION_CONTROL = 1 << 1,
	TESSELLATION_EVALUATION = 1 << 2,
	GEOMETRY = 1 << 3,
	FRAGMENT = 1 << 4,
	COMPUTE = 1 << 5,
	RAY_GEN = 1 << 6,
	ANY_HIT = 1 << 7,
	CLOSEST_HIT = 1 << 8,
	MISS = 1 << 9,
	INTERSECTION = 1 << 10,
	CALLABLE = 1 << 11,

	NONE = 0,
	ALL_GRAPHICS = VERTEX | TESSELLATION_CONTROL | TESSELLATION_EVALUATION | GEOMETRY | FRAGMENT,
	ALL_RAY_TRACING = RAY_GEN | ANY_HIT | CLOSEST_HIT | MISS | INTERSECTION | CALLABLE,
	ALL = ALL_GRAPHICS | COMPUTE | ALL_RAY_TRACING,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderTypeBit)

enum class ShaderVariableDataType : U8
{
	NONE,

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount, isIntagralType) capital,
#define ANKI_SVDT_MACRO_OPAQUE(capital, type) capital,
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
#undef ANKI_SVDT_MACRO_OPAQUE

	// Derived
	COUNT,

	NUMERICS_FIRST = I32,
	NUMERICS_LAST = MAT4,

	NUMERIC_1_COMPONENT_FIRST = I32,
	NUMERIC_1_COMPONENT_LAST = F32,
	NUMERIC_2_COMPONENT_FIRST = IVEC2,
	NUMERIC_2_COMPONENT_LAST = VEC2,
	NUMERIC_3_COMPONENT_FIRST = IVEC3,
	NUMERIC_3_COMPONENT_LAST = VEC3,
	NUMERIC_4_COMPONENT_FIRST = IVEC4,
	NUMERIC_4_COMPONENT_LAST = VEC4,

	MATRIX_FIRST = MAT3,
	MATRIX_LAST = MAT4,

	TEXTURE_FIRST = TEXTURE_1D,
	TEXTURE_LAST = TEXTURE_CUBE_ARRAY,

	IMAGE_FIRST = IMAGE_1D,
	IMAGE_LAST = IMAGE_CUBE_ARRAY,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderVariableDataType)

class ShaderVariableDataTypeInfo
{
public:
	const Char* m_name;
	U32 m_size; ///< Size of the type.
	Bool m_opaque;
	Bool m_isIntegral; ///< If true is integral type. Else it's float.
};

ANKI_PURE const ShaderVariableDataTypeInfo& getShaderVariableDataTypeInfo(ShaderVariableDataType type);

/// Occlusion query result bit.
enum class OcclusionQueryResultBit : U8
{
	NOT_AVAILABLE = 1 << 0,
	VISIBLE = 1 << 1,
	NOT_VISIBLE = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(OcclusionQueryResultBit)

/// Occlusion query result.
enum class OcclusionQueryResult : U8
{
	NOT_AVAILABLE,
	VISIBLE,
	NOT_VISIBLE
};

/// Timestamp query result.
enum class TimestampQueryResult : U8
{
	NOT_AVAILABLE,
	AVAILABLE
};

/// Attachment load operation.
enum class AttachmentLoadOperation : U8
{
	LOAD,
	CLEAR,
	DONT_CARE
};

/// Attachment store operation.
enum class AttachmentStoreOperation : U8
{
	STORE,
	DONT_CARE
};

/// Buffer usage modes.
/// The graphics work consists of the following pipes: INDIRECT, GEOMETRY (all programmable and fixed function geometry
/// stages) and finaly FRAGMENT.
/// The compute from the consists of the following: INDIRECT and COMPUTE.
/// The trace rays from the: INDIRECT and TRACE_RAYS
/// !!WARNING!! If you change this remember to change PrivateBufferUsageBit.
enum class BufferUsageBit : U64
{
	NONE = 0,

	UNIFORM_GEOMETRY = 1ull << 0ull,
	UNIFORM_FRAGMENT = 1ull << 1ull,
	UNIFORM_COMPUTE = 1ull << 2ull,
	UNIFORM_TRACE_RAYS = 1ull << 3ull,

	STORAGE_GEOMETRY_READ = 1ull << 4ull,
	STORAGE_GEOMETRY_WRITE = 1ull << 5ull,
	STORAGE_FRAGMENT_READ = 1ull << 6ull,
	STORAGE_FRAGMENT_WRITE = 1ull << 7ull,
	STORAGE_COMPUTE_READ = 1ull << 8ull,
	STORAGE_COMPUTE_WRITE = 1ull << 9ull,
	STORAGE_TRACE_RAYS_READ = 1ull << 10ull,
	STORAGE_TRACE_RAYS_WRITE = 1ull << 11ull,

	TEXTURE_GEOMETRY_READ = 1ull << 12ull,
	TEXTURE_GEOMETRY_WRITE = 1ull << 13ull,
	TEXTURE_FRAGMENT_READ = 1ull << 14ull,
	TEXTURE_FRAGMENT_WRITE = 1ull << 15ull,
	TEXTURE_COMPUTE_READ = 1ull << 16ull,
	TEXTURE_COMPUTE_WRITE = 1ull << 17ull,
	TEXTURE_TRACE_RAYS_READ = 1ull << 18ull,
	TEXTURE_TRACE_RAYS_WRITE = 1ull << 19ull,

	INDEX = 1ull << 20ull,
	VERTEX = 1ull << 21ull,

	INDIRECT_COMPUTE = 1ull << 22ll,
	INDIRECT_DRAW = 1ull << 23ull,
	INDIRECT_TRACE_RAYS = 1ull << 24ull,

	TRANSFER_SOURCE = 1ull << 25ull,
	TRANSFER_DESTINATION = 1ull << 26ull,

	ACCELERATION_STRUCTURE_BUILD = 1ull << 27ull, ///< Will be used as a position or index buffer in a BLAS build.
	SBT = 1ull << 28ull, ///< Will be used as SBT in a traceRays() command.

	// Derived
	ALL_UNIFORM = UNIFORM_GEOMETRY | UNIFORM_FRAGMENT | UNIFORM_COMPUTE | UNIFORM_TRACE_RAYS,
	ALL_STORAGE = STORAGE_GEOMETRY_READ | STORAGE_GEOMETRY_WRITE | STORAGE_FRAGMENT_READ | STORAGE_FRAGMENT_WRITE
				  | STORAGE_COMPUTE_READ | STORAGE_COMPUTE_WRITE | STORAGE_TRACE_RAYS_READ | STORAGE_TRACE_RAYS_WRITE,
	ALL_TEXTURE = TEXTURE_GEOMETRY_READ | TEXTURE_GEOMETRY_WRITE | TEXTURE_FRAGMENT_READ | TEXTURE_FRAGMENT_WRITE
				  | TEXTURE_COMPUTE_READ | TEXTURE_COMPUTE_WRITE | TEXTURE_TRACE_RAYS_READ | TEXTURE_TRACE_RAYS_WRITE,
	ALL_INDIRECT = INDIRECT_COMPUTE | INDIRECT_DRAW | INDIRECT_TRACE_RAYS,
	ALL_TRANSFER = TRANSFER_SOURCE | TRANSFER_DESTINATION,

	ALL_GEOMETRY = UNIFORM_GEOMETRY | STORAGE_GEOMETRY_READ | STORAGE_GEOMETRY_WRITE | TEXTURE_GEOMETRY_READ
				   | TEXTURE_GEOMETRY_WRITE | INDEX | VERTEX,
	ALL_FRAGMENT = UNIFORM_FRAGMENT | STORAGE_FRAGMENT_READ | STORAGE_FRAGMENT_WRITE | TEXTURE_FRAGMENT_READ
				   | TEXTURE_FRAGMENT_WRITE,
	ALL_GRAPHICS = ALL_GEOMETRY | ALL_FRAGMENT | INDIRECT_DRAW,
	ALL_COMPUTE = UNIFORM_COMPUTE | STORAGE_COMPUTE_READ | STORAGE_COMPUTE_WRITE | TEXTURE_COMPUTE_READ
				  | TEXTURE_COMPUTE_WRITE | INDIRECT_COMPUTE,
	ALL_TRACE_RAYS = UNIFORM_TRACE_RAYS | STORAGE_TRACE_RAYS_READ | STORAGE_TRACE_RAYS_WRITE | TEXTURE_TRACE_RAYS_READ
					 | TEXTURE_TRACE_RAYS_WRITE | INDIRECT_TRACE_RAYS | SBT,

	ALL_RAY_TRACING = ALL_TRACE_RAYS | ACCELERATION_STRUCTURE_BUILD,
	ALL_READ = ALL_UNIFORM | STORAGE_GEOMETRY_READ | STORAGE_FRAGMENT_READ | STORAGE_COMPUTE_READ
			   | STORAGE_TRACE_RAYS_READ | TEXTURE_GEOMETRY_READ | TEXTURE_FRAGMENT_READ | TEXTURE_COMPUTE_READ
			   | TEXTURE_TRACE_RAYS_READ | INDEX | VERTEX | INDIRECT_COMPUTE | INDIRECT_DRAW | INDIRECT_TRACE_RAYS
			   | TRANSFER_SOURCE | ACCELERATION_STRUCTURE_BUILD | SBT,
	ALL_WRITE = STORAGE_GEOMETRY_WRITE | STORAGE_FRAGMENT_WRITE | STORAGE_COMPUTE_WRITE | STORAGE_TRACE_RAYS_WRITE
				| TEXTURE_GEOMETRY_WRITE | TEXTURE_FRAGMENT_WRITE | TEXTURE_COMPUTE_WRITE | TEXTURE_TRACE_RAYS_WRITE
				| TRANSFER_DESTINATION,
	ALL = ALL_READ | ALL_WRITE,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferUsageBit)

/// Buffer access when mapped.
enum class BufferMapAccessBit : U8
{
	NONE = 0,
	READ = 1 << 0,
	WRITE = 1 << 1
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferMapAccessBit)

/// Index buffer's index type.
enum class IndexType : U8
{
	U16,
	U32,
	COUNT
};

/// Rasterization order.
enum class RasterizationOrder : U8
{
	ORDERED,
	RELAXED,
	COUNT
};

/// Acceleration structure type.
enum class AccelerationStructureType : U8
{
	TOP_LEVEL,
	BOTTOM_LEVEL,
	COUNT
};

enum class AccelerationStructureUsageBit : U8
{
	NONE = 0,
	BUILD = 1 << 0,
	ATTACH = 1 << 1, ///< Attached to a TLAS. Only for BLAS.
	GEOMETRY_READ = 1 << 2,
	FRAGMENT_READ = 1 << 3,
	COMPUTE_READ = 1 << 4,
	TRACE_RAYS_READ = 1 << 5,

	// Derived
	ALL_GRAPHICS = GEOMETRY_READ | FRAGMENT_READ,
	ALL_READ = ATTACH | GEOMETRY_READ | FRAGMENT_READ | COMPUTE_READ | TRACE_RAYS_READ,
	ALL_WRITE = BUILD
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AccelerationStructureUsageBit)

/// VRS rates.
enum class VrsRate : U8
{
	_1x1, ///< Disable VRS. Always supported.
	_2x1, ///< Always supported.
	_1x2,
	_2x2, ///< Always supported.
	_4x2,
	_2x4,
	_4x4,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VrsRate)
/// @}

} // end namespace anki
