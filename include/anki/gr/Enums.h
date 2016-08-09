// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Enum.h>
#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup graphics
/// @{
enum ColorBit : U8
{
	NONE = 0,
	RED = 1 << 0,
	GREEN = 1 << 1,
	BLUE = 1 << 2,
	ALPHA = 1 << 3,
	ALL = RED | GREEN | BLUE | ALPHA
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ColorBit, inline)

enum PrimitiveTopology : U8
{
	POINTS,
	LINES,
	LINE_STIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	PATCHES
};

enum class FillMode : U8
{
	POINTS,
	WIREFRAME,
	SOLID
};

enum class CullMode : U8
{
	FRONT,
	BACK,
	FRONT_AND_BACK
};

enum class CompareOperation : U8
{
	ALWAYS,
	LESS,
	EQUAL,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	NOT_EQUAL,
	NEVER
};

enum class BlendMethod : U8
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
	ONE_MINUS_SRC1_ALPHA
};

enum class BlendFunction : U8
{
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX
};

enum class VertexStepRate : U8
{
	VERTEX,
	INSTANCE,
	DRAW
};

enum class ComponentFormat : U8
{
	NONE,

	R8,
	R8G8,
	R8G8B8,
	R8G8B8A8,
	R16,
	R16G16,
	R16G16B16,
	R16G16B16A16,
	R32,
	R32G32,
	R32G32B32,
	R32G32B32A32,

	// Special
	R10G10B10A2,
	R11G11B10,

	// Compressed
	R8G8B8_S3TC, ///< DXT1
	R8G8B8_ETC2,
	R8G8B8A8_S3TC, ///< DXT5
	R8G8B8A8_ETC2,

	// Depth
	D16,
	D24,
	D32,

	// Limits
	FIRST_COMPRESSED = R8G8B8_S3TC,
	LAST_COMPRESSED = R8G8B8A8_ETC2
};

enum class TransformFormat : U8
{
	NONE,
	UNORM,
	SNORM,
	UINT,
	SINT,
	FLOAT
};

enum class TextureType : U8
{
	_1D,
	_2D,
	_3D,
	_2D_ARRAY,
	CUBE,
	CUBE_ARRAY
};

/// Texture usage hints. They are very important.
enum class TextureUsageBit : U16
{
	NONE,

	/// @name Sampled
	/// @{
	SAMPLED_VERTEX = 1 << 0,
	SAMPLED_TESSELLATION_CONTROL = 1 << 1,
	SAMPLED_TESSELLATION_EVALUATION = 1 << 2,
	SAMPLED_GEOMETRY = 1 << 3,
	SAMPLED_FRAGMENT = 1 << 4,
	SAMPLED_COMPUTE = 1 << 5,
	SAMPLED_ALL_GRAPHICS = SAMPLED_VERTEX | SAMPLED_TESSELLATION_CONTROL
		| SAMPLED_TESSELLATION_EVALUATION
		| SAMPLED_GEOMETRY
		| SAMPLED_FRAGMENT,
	SAMPLED_ALL = SAMPLED_ALL_GRAPHICS | SAMPLED_COMPUTE,
	/// @}

	/// @name Image_load_store
	/// @{
	IMAGE_COMPUTE_READ = 1 << 6,
	IMAGE_COMPUTE_WRITE = 1 << 7,
	IMAGE_COMPUTE_READ_WRITE = IMAGE_COMPUTE_READ | IMAGE_COMPUTE_WRITE,
	IMAGE_ALL = IMAGE_COMPUTE_READ_WRITE,
	/// @}

	/// @name Attachment
	/// @{
	FRAMEBUFFER_ATTACHMENT_READ = 1 << 8,
	FRAMEBUFFER_ATTACHMENT_WRITE = 1 << 9,
	FRAMEBUFFER_ATTACHMENT_READ_WRITE =
		FRAMEBUFFER_ATTACHMENT_READ | FRAMEBUFFER_ATTACHMENT_WRITE,
	/// @}

	/// @name Misc
	/// @{
	GENERATE_MIPMAPS = 1 << 10,
	UPLOAD = 1 << 11,
	CLEAR = 1 << 12 ///< Will be used in CommandBuffer::clearImage.
	/// @}
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureUsageBit, inline)

enum class SamplingFilter : U8
{
	NEAREST,
	LINEAR,
	BASE ///< Only for mipmaps
};

enum class ShaderType : U8
{
	VERTEX,
	TESSELLATION_CONTROL,
	TESSELLATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,

	COUNT,
	FIRST_GRAPHICS = VERTEX,
	LAST_GRAPHICS = FRAGMENT,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderType, inline)

enum class ShaderTypeBit : U8
{
	NONE = 0,
	VERTEX = 1 << 0,
	TESSELLATION_CONTROL = 1 << 1,
	TESSELLATION_EVALUATION = 1 << 2,
	GEOMETRY = 1 << 3,
	FRAGMENT = 1 << 4,
	COMPUTE = 1 << 5
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderTypeBit, inline)

enum class ShaderVariableDataType : U8
{
	NONE,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	MAT3,
	MAT4,
	SAMPLER_2D,
	SAMPLER_3D,
	SAMPLER_2D_ARRAY,
	SAMPLER_CUBE,

	NUMERICS_FIRST = FLOAT,
	NUMERICS_LAST = MAT4,

	SAMPLERS_FIRST = SAMPLER_2D,
	SAMPLERS_LAST = SAMPLER_CUBE
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderVariableDataType, inline)

/// Format for images and vertex attributes.
class PixelFormat
{
public:
	ComponentFormat m_components = ComponentFormat::NONE;
	TransformFormat m_transform = TransformFormat::NONE;
	Bool8 m_srgb = false;

	PixelFormat() = default;

	PixelFormat(const PixelFormat&) = default;

	PixelFormat(ComponentFormat cf, TransformFormat tf, Bool srgb = false)
		: m_components(cf)
		, m_transform(tf)
		, m_srgb(srgb)
	{
	}

	PixelFormat& operator=(const PixelFormat&) = default;

	Bool operator==(const PixelFormat& b) const
	{
		return m_components == b.m_components && m_transform == b.m_transform
			&& m_srgb == b.m_srgb;
	}

	Bool operator!=(const PixelFormat& b) const
	{
		return !(*this == b);
	}
};

/// Occlusion query result bit.
enum class OcclusionQueryResultBit : U8
{
	NOT_AVAILABLE = 1 << 0,
	VISIBLE = 1 << 1,
	NOT_VISIBLE = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(OcclusionQueryResultBit, inline)

/// Occlusion query result.
enum class OcclusionQueryResult : U8
{
	NOT_AVAILABLE,
	VISIBLE,
	NOT_VISIBLE
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
enum class BufferUsageBit : U32
{
	NONE = 0,

	UNIFORM_VERTEX = 1 << 0,
	UNIFORM_TESSELLATION_EVALUATION = 1 << 1,
	UNIFORM_TESSELLATION_CONTROL = 1 << 2,
	UNIFORM_GEOMETRY = 1 << 3,
	UNIFORM_FRAGMENT = 1 << 4,
	UNIFORM_COMPUTE = 1 << 5,
	UNIFORM_ALL_GRAPHICS = UNIFORM_VERTEX | UNIFORM_TESSELLATION_EVALUATION
		| UNIFORM_TESSELLATION_CONTROL
		| UNIFORM_GEOMETRY
		| UNIFORM_FRAGMENT,
	UNIFORM_ALL = UNIFORM_ALL_GRAPHICS | UNIFORM_COMPUTE,

	STORAGE_VERTEX = 1 << 6,
	STORAGE_TESSELLATION_EVALUATION = 1 << 7,
	STORAGE_TESSELLATION_CONTROL = 1 << 8,
	STORAGE_GEOMETRY = 1 << 9,
	STORAGE_FRAGMENT = 1 << 10,
	STORAGE_COMPUTE_READ = 1 << 11,
	STORAGE_COMPUTE_WRITE = 1 << 12,
	STORAGE_COMPUTE_READ_WRITE = STORAGE_COMPUTE_READ | STORAGE_COMPUTE_WRITE,
	STORAGE_ALL_GRAPHICS = STORAGE_VERTEX | STORAGE_TESSELLATION_EVALUATION
		| STORAGE_TESSELLATION_CONTROL
		| STORAGE_GEOMETRY
		| STORAGE_FRAGMENT,
	STORAGE_ALL =
		STORAGE_ALL_GRAPHICS | STORAGE_COMPUTE_READ | STORAGE_COMPUTE_WRITE,

	INDEX = 1 << 13,
	VERTEX = 1 << 14,
	INDIRECT = 1 << 15,

	TRANSFER_SOURCE = 1 << 16,
	TRANSFER_DESTINATION = 1 << 17,
	TRANSFER_ANY = TRANSFER_SOURCE | TRANSFER_DESTINATION
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferUsageBit, inline)

/// Buffer access when mapped.
enum class BufferMapAccessBit : U8
{
	NONE = 0,
	READ = 1 << 0,
	WRITE = 1 << 1
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferMapAccessBit, inline)
/// @}

} // end namespace anki
