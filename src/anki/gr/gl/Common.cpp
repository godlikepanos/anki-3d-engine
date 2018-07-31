// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/Common.h>

namespace anki
{

GLenum convertCompareOperation(CompareOperation in)
{
	GLenum out = GL_NONE;

	switch(in)
	{
	case CompareOperation::ALWAYS:
		out = GL_ALWAYS;
		break;
	case CompareOperation::LESS:
		out = GL_LESS;
		break;
	case CompareOperation::EQUAL:
		out = GL_EQUAL;
		break;
	case CompareOperation::LESS_EQUAL:
		out = GL_LEQUAL;
		break;
	case CompareOperation::GREATER:
		out = GL_GREATER;
		break;
	case CompareOperation::GREATER_EQUAL:
		out = GL_GEQUAL;
		break;
	case CompareOperation::NOT_EQUAL:
		out = GL_NOTEQUAL;
		break;
	case CompareOperation::NEVER:
		out = GL_NEVER;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

GLenum convertStencilOperation(StencilOperation in)
{
	GLenum out = GL_NONE;

	switch(in)
	{
	case StencilOperation::KEEP:
		out = GL_KEEP;
		break;
	case StencilOperation::ZERO:
		out = GL_ZERO;
		break;
	case StencilOperation::REPLACE:
		out = GL_REPLACE;
		break;
	case StencilOperation::INCREMENT_AND_CLAMP:
		out = GL_INCR;
		break;
	case StencilOperation::DECREMENT_AND_CLAMP:
		out = GL_DECR;
		break;
	case StencilOperation::INVERT:
		out = GL_INVERT;
		break;
	case StencilOperation::INCREMENT_AND_WRAP:
		out = GL_INCR_WRAP;
		break;
	case StencilOperation::DECREMENT_AND_WRAP:
		out = GL_DECR_WRAP;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

void convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter, GLenum& minFilter, GLenum& magFilter)
{
	switch(minMagFilter)
	{
	case SamplingFilter::NEAREST:
		magFilter = GL_NEAREST;
		switch(mipFilter)
		{
		case SamplingFilter::NEAREST:
			minFilter = GL_NEAREST_MIPMAP_NEAREST;
			break;
		case SamplingFilter::LINEAR:
			minFilter = GL_NEAREST_MIPMAP_LINEAR;
			break;
		case SamplingFilter::BASE:
			minFilter = GL_NEAREST;
			break;
		default:
			ANKI_ASSERT(0);
		}
		break;
	case SamplingFilter::LINEAR:
		magFilter = GL_LINEAR;
		switch(mipFilter)
		{
		case SamplingFilter::NEAREST:
			minFilter = GL_LINEAR_MIPMAP_NEAREST;
			break;
		case SamplingFilter::LINEAR:
			minFilter = GL_LINEAR_MIPMAP_LINEAR;
			break;
		case SamplingFilter::BASE:
			minFilter = GL_LINEAR;
			break;
		default:
			ANKI_ASSERT(0);
		}
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

void convertVertexFormat(Format fmt, U& compCount, GLenum& type, Bool& normalized)
{
	switch(fmt)
	{
	case Format::R32_SFLOAT:
		compCount = 1;
		type = GL_FLOAT;
		normalized = false;
		break;
	case Format::R32G32_SFLOAT:
		compCount = 2;
		type = GL_FLOAT;
		normalized = false;
		break;
	case Format::R32G32B32_SFLOAT:
		compCount = 3;
		type = GL_FLOAT;
		normalized = false;
		break;
	case Format::R32G32B32A32_SFLOAT:
		compCount = 4;
		type = GL_FLOAT;
		normalized = false;
		break;
	case Format::R16G16_SFLOAT:
		compCount = 2;
		type = GL_HALF_FLOAT;
		normalized = false;
		break;
	case Format::R16G16_UNORM:
		compCount = 2;
		type = GL_UNSIGNED_SHORT;
		normalized = true;
		break;
	case Format::R16G16B16_SFLOAT:
		compCount = 3;
		type = GL_HALF_FLOAT;
		normalized = false;
		break;
	case Format::R16G16B16_UNORM:
		compCount = 3;
		type = GL_UNSIGNED_SHORT;
		normalized = true;
		break;
	case Format::R16G16B16A16_SFLOAT:
		compCount = 4;
		type = GL_HALF_FLOAT;
		normalized = false;
		break;
	case Format::A2B10G10R10_SNORM_PACK32:
		compCount = 4;
		type = GL_INT_2_10_10_10_REV;
		normalized = true;
		break;
	case Format::R8G8B8A8_UNORM:
		compCount = 4;
		type = GL_UNSIGNED_BYTE;
		normalized = true;
		break;
	case Format::R8G8B8_UNORM:
		compCount = 3;
		type = GL_UNSIGNED_BYTE;
		normalized = true;
		break;
	case Format::R16G16B16A16_UINT:
		compCount = 4;
		type = GL_UNSIGNED_SHORT;
		normalized = false;
		break;
	default:
		ANKI_ASSERT(!"TODO");
	}
}

GLenum convertBlendFactor(BlendFactor in)
{
	GLenum out;

	switch(in)
	{
	case BlendFactor::ZERO:
		out = GL_ZERO;
		break;
	case BlendFactor::ONE:
		out = GL_ONE;
		break;
	case BlendFactor::SRC_COLOR:
		out = GL_SRC_COLOR;
		break;
	case BlendFactor::ONE_MINUS_SRC_COLOR:
		out = GL_ONE_MINUS_SRC_COLOR;
		break;
	case BlendFactor::DST_COLOR:
		out = GL_DST_COLOR;
		break;
	case BlendFactor::ONE_MINUS_DST_COLOR:
		out = GL_ONE_MINUS_DST_COLOR;
		break;
	case BlendFactor::SRC_ALPHA:
		out = GL_SRC_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_SRC_ALPHA:
		out = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BlendFactor::DST_ALPHA:
		out = GL_DST_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_DST_ALPHA:
		out = GL_ONE_MINUS_DST_ALPHA;
		break;
	case BlendFactor::CONSTANT_COLOR:
		out = GL_CONSTANT_COLOR;
		break;
	case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
		out = GL_ONE_MINUS_CONSTANT_COLOR;
		break;
	case BlendFactor::CONSTANT_ALPHA:
		out = GL_CONSTANT_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
		out = GL_ONE_MINUS_CONSTANT_ALPHA;
		break;
	case BlendFactor::SRC_ALPHA_SATURATE:
		out = GL_SRC_ALPHA_SATURATE;
		break;
	case BlendFactor::SRC1_COLOR:
		out = GL_SRC1_COLOR;
		break;
	case BlendFactor::ONE_MINUS_SRC1_COLOR:
		out = GL_ONE_MINUS_SRC1_COLOR;
		break;
	case BlendFactor::SRC1_ALPHA:
		out = GL_SRC1_ALPHA;
		break;
	case BlendFactor::ONE_MINUS_SRC1_ALPHA:
		out = GL_ONE_MINUS_SRC1_ALPHA;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

void convertTextureInformation(
	Format pf, Bool8& compressed, GLenum& format, GLenum& internalFormat, GLenum& type, DepthStencilAspectBit& dsAspect)
{
	compressed = formatIsCompressed(pf);
	dsAspect = computeFormatAspect(pf);
	format = GL_NONE;
	internalFormat = GL_NONE;
	type = GL_NONE;

	switch(pf)
	{
	case Format::BC1_RGB_UNORM_BLOCK:
		format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
	case Format::BC3_UNORM_BLOCK:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
	case Format::BC6H_UFLOAT_BLOCK:
		format = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
		internalFormat = format;
		type = GL_FLOAT;
		break;
	case Format::R8_UNORM:
		format = GL_RED;
		internalFormat = GL_R8;
		type = GL_UNSIGNED_BYTE;
		break;
	case Format::R8_SNORM:
		format = GL_RED;
		internalFormat = GL_R8_SNORM;
		type = GL_BYTE;
		break;
	case Format::R8G8_UNORM:
		format = GL_RG;
		internalFormat = GL_RG8;
		type = GL_UNSIGNED_BYTE;
		break;
	case Format::R8G8_SNORM:
		format = GL_RG;
		internalFormat = GL_RG8_SNORM;
		type = GL_BYTE;
		break;
	case Format::R8G8B8_UNORM:
		format = GL_RGB;
		internalFormat = GL_RGB8;
		type = GL_UNSIGNED_BYTE;
		break;
	case Format::R8G8B8_SNORM:
		format = GL_RGB;
		internalFormat = GL_RGB8_SNORM;
		type = GL_BYTE;
		break;
	case Format::R8G8B8_UINT:
		format = GL_RGB;
		internalFormat = GL_RGB8UI;
		type = GL_RGB_INTEGER;
		break;
	case Format::R8G8B8_SINT:
		format = GL_RGB;
		internalFormat = GL_RGB8I;
		type = GL_RGB_INTEGER;
		break;
	case Format::R8G8B8A8_UNORM:
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
		type = GL_UNSIGNED_BYTE;
		break;
	case Format::R8G8B8A8_SNORM:
		format = GL_RGBA;
		internalFormat = GL_RGBA8_SNORM;
		type = GL_BYTE;
		break;
	case Format::R16_SFLOAT:
		format = GL_R;
		internalFormat = GL_R16F;
		type = GL_FLOAT;
		break;
	case Format::R16_UINT:
		format = GL_R;
		internalFormat = GL_R16UI;
		type = GL_UNSIGNED_SHORT;
		break;
	case Format::R16_UNORM:
		format = GL_R;
		internalFormat = GL_R16;
		type = GL_UNSIGNED_SHORT;
		break;
	case Format::R16G16_UNORM:
		format = GL_RG;
		internalFormat = GL_RG16;
		type = GL_UNSIGNED_SHORT;
		break;
	case Format::R16G16_SNORM:
		format = GL_RG;
		internalFormat = GL_RG16_SNORM;
		type = GL_SHORT;
		break;
	case Format::R16G16B16_SFLOAT:
		format = GL_RGB;
		internalFormat = GL_RGB16F;
		type = GL_FLOAT;
		break;
	case Format::R16G16B16_UINT:
		format = GL_RGB_INTEGER;
		internalFormat = GL_RGB16UI;
		type = GL_UNSIGNED_INT;
		break;
	case Format::R16G16B16A16_SFLOAT:
		format = GL_RGBA;
		internalFormat = GL_RGBA16F;
		type = GL_FLOAT;
		break;
	case Format::R16G16B16A16_UINT:
		format = GL_RGBA;
		internalFormat = GL_RGBA16UI;
		type = GL_UNSIGNED_SHORT;
		break;
	case Format::R32_SFLOAT:
		format = GL_R;
		internalFormat = GL_R32F;
		type = GL_FLOAT;
		break;
	case Format::R32_UINT:
		format = GL_RG_INTEGER;
		internalFormat = GL_R32UI;
		type = GL_UNSIGNED_INT;
		break;
	case Format::R32G32_SFLOAT:
		format = GL_RG;
		internalFormat = GL_RG32F;
		type = GL_FLOAT;
		break;
	case Format::R32G32_UINT:
		format = GL_RG_INTEGER;
		internalFormat = GL_RG32UI;
		type = GL_UNSIGNED_INT;
		break;
	case Format::R32G32B32_SFLOAT:
		format = GL_RGB;
		internalFormat = GL_RGB32F;
		type = GL_FLOAT;
		break;
	case Format::R32G32B32_UINT:
		format = GL_RGB_INTEGER;
		internalFormat = GL_RGB32UI;
		type = GL_UNSIGNED_INT;
		break;
	case Format::R32G32B32A32_SFLOAT:
		format = GL_RGBA;
		internalFormat = GL_RGBA32F;
		type = GL_FLOAT;
		break;
	case Format::R32G32B32A32_UINT:
		format = GL_RGBA_INTEGER;
		internalFormat = GL_RGBA32UI;
		type = GL_UNSIGNED_INT;
		break;
	case Format::B10G11R11_UFLOAT_PACK32:
		format = GL_RGB;
		internalFormat = GL_R11F_G11F_B10F;
		type = GL_FLOAT;
		break;
	case Format::A2B10G10R10_UNORM_PACK32:
		format = GL_RGBA;
		internalFormat = GL_RGB10_A2;
		type = GL_UNSIGNED_INT;
		break;
	case Format::D24_UNORM_S8_UINT:
		format = GL_DEPTH_STENCIL;
		internalFormat = GL_DEPTH24_STENCIL8;
		type = GL_UNSIGNED_INT;
		break;
	case Format::D16_UNORM_S8_UINT:
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT16;
		type = GL_UNSIGNED_SHORT;
		break;
	case Format::D32_SFLOAT:
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT32;
		type = GL_UNSIGNED_INT;
		break;
	case Format::S8_UINT:
		format = GL_STENCIL_INDEX;
		internalFormat = GL_STENCIL_INDEX8;
		type = GL_BYTE;
		break;
	default:
		ANKI_ASSERT(!"TODO");
	}
}

} // end namespace anki
