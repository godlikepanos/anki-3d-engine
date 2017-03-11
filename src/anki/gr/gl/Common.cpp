// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

void convertVertexFormat(const PixelFormat& fmt, U& compCount, GLenum& type, Bool& normalized)
{
	if(fmt == PixelFormat(ComponentFormat::R32, TransformFormat::FLOAT))
	{
		compCount = 1;
		type = GL_FLOAT;
		normalized = false;
	}
	else if(fmt == PixelFormat(ComponentFormat::R32G32, TransformFormat::FLOAT))
	{
		compCount = 2;
		type = GL_FLOAT;
		normalized = false;
	}
	else if(fmt == PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT))
	{
		compCount = 3;
		type = GL_FLOAT;
		normalized = false;
	}
	else if(fmt == PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT))
	{
		compCount = 4;
		type = GL_FLOAT;
		normalized = false;
	}
	else if(fmt == PixelFormat(ComponentFormat::R16G16, TransformFormat::FLOAT))
	{
		compCount = 2;
		type = GL_HALF_FLOAT;
		normalized = false;
	}
	else if(fmt == PixelFormat(ComponentFormat::R16G16, TransformFormat::UNORM))
	{
		compCount = 2;
		type = GL_UNSIGNED_SHORT;
		normalized = true;
	}
	else if(fmt == PixelFormat(ComponentFormat::R10G10B10A2, TransformFormat::SNORM))
	{
		compCount = 4;
		type = GL_INT_2_10_10_10_REV;
		normalized = true;
	}
	else if(fmt == PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM))
	{
		compCount = 4;
		type = GL_UNSIGNED_BYTE;
		normalized = true;
	}
	else if(fmt == PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM))
	{
		compCount = 3;
		type = GL_UNSIGNED_BYTE;
		normalized = true;
	}
	else
	{
		ANKI_ASSERT(0 && "TODO");
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

void convertTextureInformation(const PixelFormat& pf,
	Bool8& compressed,
	GLenum& format,
	GLenum& internalFormat,
	GLenum& type,
	DepthStencilAspectBit& dsAspect)
{
	compressed =
		pf.m_components >= ComponentFormat::FIRST_COMPRESSED && pf.m_components <= ComponentFormat::LAST_COMPRESSED;

	switch(pf.m_components)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
	case ComponentFormat::R8G8B8_S3TC:
		format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R8G8B8A8_S3TC:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
#else
	case ComponentFormat::R8G8B8_ETC2:
		format = GL_COMPRESSED_RGB8_ETC2;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R8G8B8A8_ETC2:
		format = GL_COMPRESSED_RGBA8_ETC2_EAC;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
#endif
	case ComponentFormat::R8:
		format = GL_RED;

		if(pf.m_transform == TransformFormat::UNORM)
		{
			internalFormat = GL_R8;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			ANKI_ASSERT(pf.m_transform == TransformFormat::SNORM);
			internalFormat = GL_R8_SNORM;
			type = GL_BYTE;
		}
		break;
	case ComponentFormat::R8G8:
		format = GL_RG;

		if(pf.m_transform == TransformFormat::UNORM)
		{
			internalFormat = GL_RG8;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			ANKI_ASSERT(pf.m_transform == TransformFormat::SNORM);
			internalFormat = GL_RG8_SNORM;
			type = GL_BYTE;
		}
		break;
	case ComponentFormat::R8G8B8:
		format = GL_RGB;

		if(pf.m_transform == TransformFormat::UNORM)
		{
			internalFormat = GL_RGB8;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			ANKI_ASSERT(pf.m_transform == TransformFormat::SNORM);
			internalFormat = GL_RGB8_SNORM;
			type = GL_BYTE;
		}
		break;
	case ComponentFormat::R8G8B8A8:
		format = GL_RGBA;

		if(pf.m_transform == TransformFormat::UNORM)
		{
			internalFormat = GL_RGBA8;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			ANKI_ASSERT(pf.m_transform == TransformFormat::SNORM);
			internalFormat = GL_RGBA8_SNORM;
			type = GL_BYTE;
		}
		break;
	case ComponentFormat::R16:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_R;
			internalFormat = GL_R16F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_R;
			internalFormat = GL_R16UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(!"TODO");
		}
		break;
	case ComponentFormat::R16G16B16:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGB;
			internalFormat = GL_RGB16F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RGB_INTEGER;
			internalFormat = GL_RGB16UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R16G16B16A16:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGBA;
			internalFormat = GL_RGBA16F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			ANKI_ASSERT(!"TODO");
		}
		else
		{
			ANKI_ASSERT(!"TODO");
		}
		break;
	case ComponentFormat::R32:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_R;
			internalFormat = GL_R32F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RG_INTEGER;
			internalFormat = GL_R32UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R32G32:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RG;
			internalFormat = GL_RG32F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RG_INTEGER;
			internalFormat = GL_RG32UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R32G32B32:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGB;
			internalFormat = GL_RGB32F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RGB_INTEGER;
			internalFormat = GL_RGB32UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(!"TODO");
		}
		break;
	case ComponentFormat::R32G32B32A32:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGBA;
			internalFormat = GL_RGBA32F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RGBA_INTEGER;
			internalFormat = GL_RGBA32UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(!"TODO");
		}
		break;
	case ComponentFormat::R11G11B10:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGB;
			internalFormat = GL_R11F_G11F_B10F;
			type = GL_FLOAT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R10G10B10A2:
		if(pf.m_transform == TransformFormat::UNORM)
		{
			format = GL_RGBA;
			internalFormat = GL_RGB10_A2;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::D24S8:
		format = GL_DEPTH_STENCIL;
		internalFormat = GL_DEPTH24_STENCIL8;
		type = GL_UNSIGNED_INT;
		dsAspect = DepthStencilAspectBit::DEPTH_STENCIL;
		break;
	case ComponentFormat::D16:
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT16;
		type = GL_UNSIGNED_SHORT;
		dsAspect = DepthStencilAspectBit::DEPTH;
		break;
	case ComponentFormat::D32:
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT32;
		type = GL_UNSIGNED_INT;
		dsAspect = DepthStencilAspectBit::DEPTH;
		break;
	case ComponentFormat::S8:
		format = GL_STENCIL_INDEX;
		internalFormat = GL_STENCIL_INDEX8;
		type = GL_BYTE;
		dsAspect = DepthStencilAspectBit::STENCIL;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

} // end namespace anki
