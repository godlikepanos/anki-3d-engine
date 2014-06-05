// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlCommon.h"

namespace anki {

//==============================================================================
U computeShaderTypeIndex(const GLenum glType)
{
	U idx = 0;
	switch(glType)
	{
	case GL_VERTEX_SHADER:
		idx = 0;
		break;
	case GL_TESS_CONTROL_SHADER:
		idx = 1;
		break;
	case GL_TESS_EVALUATION_SHADER:
		idx = 2;
		break;
	case GL_GEOMETRY_SHADER:
		idx = 3;
		break;
	case GL_FRAGMENT_SHADER:
		idx = 4;
		break;
	case GL_COMPUTE_SHADER:
		idx = 5;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return idx;
}

//==============================================================================
GLenum computeGlShaderType(const U idx, GLbitfield* bit)
{
	static const Array<GLenum, 6> gltype = {{GL_VERTEX_SHADER, 
		GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}};

	static const Array<GLuint, 6> glbit = {{
		GL_VERTEX_SHADER_BIT, GL_TESS_CONTROL_SHADER_BIT, 
		GL_TESS_EVALUATION_SHADER_BIT, GL_GEOMETRY_SHADER_BIT,
		GL_FRAGMENT_SHADER_BIT, GL_COMPUTE_SHADER_BIT}};

	if(bit)
	{
		*bit = glbit[idx];
	}

	return gltype[idx];
}

} // end namespace anki
