// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

} // end namespace anki
