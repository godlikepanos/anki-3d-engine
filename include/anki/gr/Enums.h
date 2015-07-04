// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/util/StdTypes.h"
#include "anki/util/Enum.h"

namespace anki {

/// @addtogroup graphics
/// @{

enum ColorBit: U8
{
	NONE = 0,
	RED = 1 << 0,
	GREEN = 1 << 1,
	BLUE = 1 << 2,
	ALPHA = 1 << 3,
	ALL = RED | GREEN | BLUE | ALPHA
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ColorBit, inline)

enum PrimitiveTopology: U8
{
	POINTS,
	LINES,
	LINE_STIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	PATCHES
};

enum class FillMode: U8
{
	POINTS,
	WIREFRAME,
	SOLID
};

enum class CullMode: U8
{
	FRONT,
	BACK,
	FRONT_AND_BACK
};

enum class CompareOperation: U8
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

enum class BlendMethod: U8
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

enum class BlendFunction: U8
{
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX
};

enum class VertexStepRate: U8
{
	VERTEX,
	INSTANCE,
	DRAW
};

enum class ComponentFormat: U8
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

enum class TransformFormat: U8
{
	NONE,
	UNORM,
	SNORM,
	UINT,
	SINT,
	FLOAT
};

enum class TextureType: U8
{
	_1D,
	_2D,
	_3D,
	_2D_ARRAY,
	CUBE
};

enum class SamplingFilter: U8
{
	NEAREST,
	LINEAR,
	BASE ///< Only for mipmaps
};

enum class ShaderType: U8
{
	VERTEX,
	TESSELLATION_CONTROL,
	TESSELLATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,

	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderType, inline)

enum class ShaderVariableDataType: U8
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
	:	m_components(cf),
		m_transform(tf),
		m_srgb(srgb)
	{}

	PixelFormat& operator=(const PixelFormat&) = default;

	Bool operator==(const PixelFormat& b) const
	{
		return m_components == b.m_components && m_transform == b.m_transform
			&& m_srgb == b.m_srgb;
	}
};

/// Occlusion query result bit.
enum class OcclusionQueryResultBit: U8
{
	NOT_AVAILABLE = 1 << 0,
	VISIBLE = 1 << 1,
	NOT_VISIBLE = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(OcclusionQueryResultBit, inline)

/// Occlusion query result.
enum class OcclusionQueryResult: U8
{
	NOT_AVAILABLE,
	VISIBLE,
	NOT_VISIBLE
};

/// Attachment load operation.
enum class AttachmentLoadOperation: U8
{
	LOAD,
	CLEAR,
	DONT_CARE
};

/// Attachment store operation.
enum class AttachmentStoreOperation: U8
{
	STORE,
	RESOLVE_MSAA,
	DONT_CARE
};

/// Buffer usage modes.
enum class BufferUsageBit: U8
{
	NONE = 0,
	UNIFORM_BUFFER = 1 << 0,
	STORAGE_BUFFER = 1 << 1,
	INDEX_BUFFER = 1 << 2,
	VERTEX_BUFFER = 1 << 3,
	INDIRECT_BUFFER = 1 << 4
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferUsageBit, inline)

/// Buffer access from client modes.
enum class BufferAccessBit: U8
{
	NONE = 0,
	CLIENT_MAP_READ = 1 << 0,
	CLIENT_MAP_WRITE = 1 << 1,
	CLIENT_WRITE = 1 << 2,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BufferAccessBit, inline)
/// @}

} // end namespace anki

