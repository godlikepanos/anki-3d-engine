// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/ShaderProgramImpl.h>
#include <anki/gr/Shader.h>
#include <anki/gr/gl/ShaderImpl.h>

namespace anki
{

static void deletePrograms(GLsizei n, const GLuint* progs)
{
	ANKI_ASSERT(n == 1);
	ANKI_ASSERT(progs);
	glDeleteProgram(*progs);
}

ShaderProgramImpl::~ShaderProgramImpl()
{
	destroyDeferred(getManager(), deletePrograms);
}

Error ShaderProgramImpl::initGraphics(ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag)
{
	m_glName = glCreateProgram();
	ANKI_ASSERT(m_glName != 0);

	glAttachShader(m_glName, static_cast<const ShaderImpl&>(*vert).getGlName());
	m_shaders[ShaderType::VERTEX] = vert;

	if(tessc)
	{
		glAttachShader(m_glName, static_cast<const ShaderImpl&>(*tessc).getGlName());
		m_shaders[ShaderType::TESSELLATION_CONTROL] = tessc;
	}

	if(tesse)
	{
		glAttachShader(m_glName, static_cast<const ShaderImpl&>(*tesse).getGlName());
		m_shaders[ShaderType::TESSELLATION_EVALUATION] = tesse;
	}

	if(geom)
	{
		glAttachShader(m_glName, static_cast<const ShaderImpl&>(*geom).getGlName());
		m_shaders[ShaderType::GEOMETRY] = geom;
	}

	glAttachShader(m_glName, static_cast<const ShaderImpl&>(*frag).getGlName());
	m_shaders[ShaderType::FRAGMENT] = frag;

	return link(static_cast<const ShaderImpl&>(*vert).getGlName(), static_cast<const ShaderImpl&>(*frag).getGlName());
}

Error ShaderProgramImpl::initCompute(ShaderPtr comp)
{
	m_glName = glCreateProgram();
	ANKI_ASSERT(m_glName != 0);

	glAttachShader(m_glName, static_cast<const ShaderImpl&>(*comp).getGlName());
	m_shaders[ShaderType::COMPUTE] = comp;

	return link(0, 0);
}

Error ShaderProgramImpl::link(GLuint vert, GLuint frag)
{
	Error err = Error::NONE;

	glLinkProgram(m_glName);
	GLint status = 0;
	glGetProgramiv(m_glName, GL_LINK_STATUS, &status);

	if(!status)
	{
		GLint infoLen = 0;
		GLint charsWritten = 0;
		DynamicArrayAuto<char> infoLogTxt(getAllocator());

		glGetProgramiv(m_glName, GL_INFO_LOG_LENGTH, &infoLen);

		infoLogTxt.create(infoLen + 1);

		glGetProgramInfoLog(m_glName, infoLen, &charsWritten, &infoLogTxt[0]);

		ANKI_GL_LOGE("Link error log follows (vs:%u, fs:%u):\n%s", vert, frag, &infoLogTxt[0]);

		err = Error::USER_DATA;
	}

	return err;
}

} // end namespace anki
