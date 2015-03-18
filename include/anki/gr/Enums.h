// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_ENUMS_H
#define ANKI_GR_ENUMS_H

#include "anki/util/StdTypes.h"
#include "anki/util/Enum.h"

namespace anki {

/// @addtogroup graphics
/// @{

// Enums
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
	NONE,
	FRONT,
	BACK,
	FRONT_AND_BACK
};

enum class DepthCompareFunction: U8
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
    DEST_COLOR,
    ONE_MINUS_DEST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DEST_ALPHA,
    ONE_MINUS_DEST_ALPHA,
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

	// Compressed
	R8G8B8_S3TC, ///< DXT1
	R8G8B8_ETC2,
	R8G8B8A8_S3TC, ///< DXT5
	R8G8B8A8_ETC2
};

enum class TransformFormat: U8
{
	UNORM,
	SNORM,
	UINT,
	SINT,
	FLOAT
};

enum class ImageType: U8
{
	_1D,
	_2D,
	_3D,
	_2D_ARRAY,
	CUBE
};

enum class TextureFilter: U8
{
	NEAREST,
	LINEAR,
	TRILINEAR
};

enum class ShaderType: U8
{
	VERTEX,
	TESSELLATION_CONTROL,
	TESSELLATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ShaderType, inline)

enum class ShaderVariableDataType: U8
{
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
struct PixelFormat
{
	ComponentFormat m_components = ComponentFormat::R8;
	TransformFormat m_transform = TransformFormat::UNORM;
	Bool8 m_srgb = false;
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
/// @}

} // end namespace anki

#endif

