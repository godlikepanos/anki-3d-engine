// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/ShaderProgramImpl.h>
#include <anki/gr/Shader.h>
#include <anki/gr/gl/ShaderImpl.h>

namespace anki
{

Error ShaderProgramImpl::initGraphics(ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag)
{
	m_glName = glCreateProgram();
	ANKI_ASSERT(m_glName != 0);

	glAttachShader(m_glName, vert->m_impl->getGlName());

	if(tessc)
	{
		glAttachShader(m_glName, tessc->m_impl->getGlName());
	}

	if(tesse)
	{
		glAttachShader(m_glName, tesse->m_impl->getGlName());
	}

	if(geom)
	{
		glAttachShader(m_glName, geom->m_impl->getGlName());
	}

	glAttachShader(m_glName, frag->m_impl->getGlName());

	return link(vert->m_impl->getGlName(), frag->m_impl->getGlName());
}

Error ShaderProgramImpl::initCompute(ShaderPtr comp)
{
	m_glName = glCreateProgram();
	ANKI_ASSERT(m_glName != 0);

	glAttachShader(m_glName, comp->m_impl->getGlName());

	return link(0, 0);
}

Error ShaderProgramImpl::link(GLuint vert, GLuint frag)
{
	Error err = ErrorCode::NONE;

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

		err = ErrorCode::USER_DATA;
	}

	return err;
}

} // end namespace anki
