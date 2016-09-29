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

} // end namespace anki
