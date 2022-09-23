// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>

#if ANKI_GL == ANKI_GL_DESKTOP
#	if ANKI_OS == ANKI_OS_WINDOWS && !defined(GLEW_STATIC)
#		define GLEW_STATIC
#	endif
#	include <GL/glew.h>
#	if !defined(ANKI_GLEW_H)
#		error "Wrong GLEW included"
#	endif
#elif ANKI_GL == ANKI_GL_ES
#	include <GLES3/gl3.h>
#else
#	error "See file"
#endif

namespace anki {

// Forward
class GlState;
class RenderingThread;

/// @addtogroup opengl
/// @{

#define ANKI_GL_LOGI(...) ANKI_LOG("GL  ", kNormal, __VA_ARGS__)
#define ANKI_GL_LOGE(...) ANKI_LOG("GL  ", kError, __VA_ARGS__)
#define ANKI_GL_LOGW(...) ANKI_LOG("GL  ", kWarning, __VA_ARGS__)
#define ANKI_GL_LOGF(...) ANKI_LOG("GL  ", kFatal, __VA_ARGS__)

#define ANKI_GL_SELF(class_) class_& self = *static_cast<class_*>(this)
#define ANKI_GL_SELF_CONST(class_) const class_& self = *static_cast<const class_*>(this)

enum class GlExtensions : U16
{
	NONE = 0,
	ARB_SHADER_BALLOT = 1 << 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GlExtensions, inline)

// Spec limits
const U MAX_UNIFORM_BLOCK_SIZE = 16384;
const U MAX_STORAGE_BLOCK_SIZE = 2 << 27;

/// Converter.
GLenum convertCompareOperation(CompareOperation in);

GLenum convertStencilOperation(StencilOperation in);

inline GLenum convertFaceMode(FaceSelectionBit in)
{
	if(in == FaceSelectionBit::FRONT)
	{
		return GL_FRONT;
	}
	else if(in == FaceSelectionBit::BACK)
	{
		return GL_BACK;
	}
	else
	{
		return GL_FRONT_AND_BACK;
	}
}

void convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter, GLenum& minFilter, GLenum& magFilter);

void convertVertexFormat(Format fmt, U& compCount, GLenum& type, Bool& normalized);

inline GLenum convertIndexType(IndexType ak)
{
	GLenum out;
	switch(ak)
	{
	case IndexType::kU16:
		out = GL_UNSIGNED_SHORT;
		break;
	case IndexType::kU32:
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

GLenum convertBlendFactor(BlendFactor in);

inline GLenum convertBlendOperation(BlendOperation ak)
{
	GLenum out;

	switch(ak)
	{
	case BlendOperation::ADD:
		out = GL_FUNC_ADD;
		break;
	case BlendOperation::SUBTRACT:
		out = GL_FUNC_SUBTRACT;
		break;
	case BlendOperation::REVERSE_SUBTRACT:
		out = GL_FUNC_REVERSE_SUBTRACT;
		break;
	case BlendOperation::MIN:
		out = GL_MIN;
		break;
	case BlendOperation::MAX:
		out = GL_MAX;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

inline GLenum convertPrimitiveTopology(PrimitiveTopology ak)
{
	GLenum out;
	switch(ak)
	{
	case PrimitiveTopology::POINTS:
		out = GL_POINTS;
		break;
	case PrimitiveTopology::LINES:
		out = GL_LINES;
		break;
	case PrimitiveTopology::LINE_STRIP:
		out = GL_LINE_STRIP;
		break;
	case PrimitiveTopology::kTriangles:
		out = GL_TRIANGLES;
		break;
	case PrimitiveTopology::TRIANGLE_STRIP:
		out = GL_TRIANGLE_STRIP;
		break;
	case PrimitiveTopology::PATCHES:
		out = GL_PATCHES;
		break;
	default:
		ANKI_ASSERT(0);
		out = 0;
	}

	return out;
}

void convertTextureInformation(Format pf, Bool& compressed, GLenum& format, GLenum& internalFormat, GLenum& type,
							   DepthStencilAspectBit& dsAspect);
/// @}

} // end namespace anki
