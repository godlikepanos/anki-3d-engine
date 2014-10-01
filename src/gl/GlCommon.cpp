// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlCommon.h"

namespace anki {

//==============================================================================
ShaderType computeShaderTypeIndex(const GLenum glType)
{
	ShaderType idx = ShaderType::VERTEX;
	switch(glType)
	{
	case GL_VERTEX_SHADER:
		idx = ShaderType::VERTEX;
		break;
	case GL_TESS_CONTROL_SHADER:
		idx = ShaderType::TESSELLATION_CONTROL;
		break;
	case GL_TESS_EVALUATION_SHADER:
		idx = ShaderType::TESSELLATION_EVALUATION;
		break;
	case GL_GEOMETRY_SHADER:
		idx = ShaderType::GEOMETRY;
		break;
	case GL_FRAGMENT_SHADER:
		idx = ShaderType::FRAGMENT;
		break;
	case GL_COMPUTE_SHADER:
		idx = ShaderType::COMPUTE;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return idx;
}

//==============================================================================
GLenum computeGlShaderType(const ShaderType idx, GLbitfield* bit)
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
		*bit = glbit[enumToType(idx)];
	}

	return gltype[enumToType(idx)];
}

} // end namespace anki
