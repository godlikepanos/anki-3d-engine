// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/gl/ShaderImpl.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
static GLenum computeGlShaderType(const ShaderType idx, GLbitfield* bit)
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

//==============================================================================
Error PipelineImpl::create(const Initializer& init)
{
	Error err = ErrorCode::NONE;

	attachProgramsInternal(init);

	// Create and attach programs
	glGenProgramPipelines(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	glBindProgramPipeline(m_glName);

	for(U i = 0; i < m_shaders.getSize(); i++)
	{
		ShaderHandle& shader = m_shaders[i];

		if(shader.isCreated())
		{
			GLbitfield bit;
			computeGlShaderType(static_cast<ShaderType>(i), &bit);
			glUseProgramStages(m_glName, bit, shader.get().getGlName());
		}
	}

	// Validate and check error
	glValidateProgramPipeline(m_glName);
	GLint status = 0;
	glGetProgramPipelineiv(m_glName, GL_VALIDATE_STATUS, &status);

	if(!status)
	{
		GLint infoLen = 0;
		GLint charsWritten = 0;
		DArray<char> infoLogTxt;

		glGetProgramPipelineiv(m_glName, GL_INFO_LOG_LENGTH, &infoLen);

		auto alloc = getAllocator();
		infoLogTxt.create(alloc, infoLen + 1);

		glGetProgramPipelineInfoLog(
			m_glName, infoLen, &charsWritten, &infoLogTxt[0]);

		ANKI_LOGE("Ppline error log follows:\n%s", &infoLogTxt[0]);
		err = ErrorCode::USER_DATA;
	}

	glBindProgramPipeline(0);

	return err;
}

//==============================================================================
void PipelineImpl::destroy()
{
	if(m_glName)
	{
		glDeleteProgramPipelines(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
void PipelineImpl::attachProgramsInternal(const Initializer& init)
{
	U mask = 0;
	U count = 6;
	while(count-- != 0)
	{
		const ShaderHandle& shader = init.m_shaders[count];
		if(shader.isCreated())
		{
			ANKI_ASSERT(count == enumToType(shader.get().getType()));
			m_shaders[count] = shader;
			mask |= 1 << count;
		}
	}

	// Check what we attached
	//
	if(mask & (1 << 5))
	{
		// Is compute

		ANKI_ASSERT((mask & (1 << 5)) == (1 << 5) 
			&& "Compute should be alone in the pipeline");
	}
	else
	{
		const U fragVert = (1 << 0) | (1 << 4);
		ANKI_ASSERT((mask & fragVert) && "Should contain vert and frag");
		(void)fragVert;

		const U tess = (1 << 1) | (1 << 2);
		if((mask & tess) != 0)
		{
			ANKI_ASSERT(((mask & tess) == 0x6)
				&& "Should set both the tessellation shaders");
		}
	}
}

//==============================================================================
void PipelineImpl::bind()
{
	ANKI_ASSERT(isCreated());
	glBindProgramPipeline(m_glName);
}

} // end namespace anki

