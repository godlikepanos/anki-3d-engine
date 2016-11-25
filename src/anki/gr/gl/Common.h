// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

#if ANKI_GL == ANKI_GL_DESKTOP
#if ANKI_OS == ANKI_OS_WINDOWS && !defined(GLEW_STATIC)
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#if !defined(ANKI_GLEW_H)
#error "Wrong GLEW included"
#endif
#elif ANKI_GL == ANKI_GL_ES
#include <GLES3/gl3.h>
#else
#error "See file"
#endif

namespace anki
{

// Forward
class GlState;
class RenderingThread;

/// @addtogroup opengl
/// @{

// Spec limits
const U MAX_UNIFORM_BLOCK_SIZE = 16384;
const U MAX_STORAGE_BLOCK_SIZE = 2 << 27;

/// Converter.
GLenum convertCompareOperation(CompareOperation in);

GLenum convertStencilOperation(StencilOperation in);

inline GLenum convertFaceMode(FaceSelectionMask in)
{
	if(in == FaceSelectionMask::FRONT)
	{
		return GL_FRONT;
	}
	else if(in == FaceSelectionMask::BACK)
	{
		return GL_BACK;
	}
	else
	{
		return GL_FRONT_AND_BACK;
	}
}

void convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter, GLenum& minFilter, GLenum& magFilter);

void convertVertexFormat(const PixelFormat& fmt, U& compCount, GLenum& type, Bool& normalized);

inline GLenum convertIndexType(IndexType ak)
{
	GLenum out;
	switch(ak)
	{
	case IndexType::U16:
		out = GL_UNSIGNED_SHORT;
		break;
	case IndexType::U32:
		out = GL_UNSIGNED_INT;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

inline GLenum convertFillMode(FillMode mode)
{
	GLenum out;
	switch(mode)
	{
	case FillMode::POINTS:
		out = GL_POINT;
		break;
	case FillMode::WIREFRAME:
		out = GL_LINE;
		break;
	case FillMode::SOLID:
		out = GL_FILL;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

GLenum convertBlendMethod(BlendMethod in);

inline GLenum convertBlendFunction(BlendFunction ak)
{
	GLenum out;

	switch(ak)
	{
	case BlendFunction::ADD:
		out = GL_FUNC_ADD;
		break;
	case BlendFunction::SUBTRACT:
		out = GL_FUNC_SUBTRACT;
		break;
	case BlendFunction::REVERSE_SUBTRACT:
		out = GL_FUNC_REVERSE_SUBTRACT;
		break;
	case BlendFunction::MIN:
		out = GL_MIN;
		break;
	case BlendFunction::MAX:
		out = GL_MAX;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}
/// @}

} // end namespace anki
