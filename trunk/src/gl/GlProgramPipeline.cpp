// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlProgramPipeline.h"
#include "anki/gl/GlProgram.h"

namespace anki {

//==============================================================================
GlProgramPipeline::GlProgramPipeline(
	const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
{
	ANKI_ASSERT(progsBegin != nullptr && progsEnd != nullptr);
	ANKI_ASSERT(progsBegin != progsEnd);

	attachProgramsInternal(progsBegin, progsEnd - progsBegin);

	// Create and attach programs
	glGenProgramPipelines(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	glBindProgramPipeline(m_glName);

	for(U i = 0; i < m_progs.size(); i++)
	{
		GlProgramHandle& prog = m_progs[i];

		if(prog.isCreated())
		{
			GLbitfield bit;
			GLenum gltype = computeGlShaderType(i, &bit);
			ANKI_ASSERT(prog.getType() == gltype && "Attached wrong shader");
			(void)gltype;
			glUseProgramStages(m_glName, bit, prog._get().getGlName());
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
		String infoLogTxt;

		glGetProgramPipelineiv(m_glName, GL_INFO_LOG_LENGTH, &infoLen);

		infoLogTxt.resize(infoLen + 1);
		glGetProgramPipelineInfoLog(
			m_glName, infoLen, &charsWritten, &infoLogTxt[0]);

		infoLogTxt = "Ppline error log follows:\n" + infoLogTxt;

		throw ANKI_EXCEPTION("%s", infoLogTxt.c_str());
	}

	glBindProgramPipeline(0);
}

//==============================================================================
GlProgramPipeline& GlProgramPipeline::operator=(GlProgramPipeline&& b)
{
	destroy();
	Base::operator=(std::forward<Base>(b));

	for(U i = 0; i < m_progs.size(); i++)
	{
		m_progs[i] = b.m_progs[i];
		b.m_progs[i] = GlProgramHandle();
	}

	return *this;
}

//==============================================================================
void GlProgramPipeline::destroy()
{
	if(m_glName)
	{
		glDeleteProgramPipelines(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
void GlProgramPipeline::attachProgramsInternal(
	const GlProgramHandle* progs, PtrSize count)
{
	U mask = 0;
	while(count-- != 0)
	{
		const GlProgramHandle& prog = progs[count];
		U idx = computeShaderTypeIndex(prog._get().getType());

		ANKI_ASSERT(!m_progs[idx].isCreated() && "Attaching the same");
		m_progs[idx] = prog;
		mask |= 1 << idx;
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
void GlProgramPipeline::bind()
{
	ANKI_ASSERT(isCreated());
	glBindProgramPipeline(m_glName);
}

//==============================================================================
GlProgramHandle GlProgramPipeline::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	U idx = computeShaderTypeIndex(type);
	GlProgramHandle prog = m_progs[idx];
	ANKI_ASSERT(prog.isCreated() && "Asking for non-created program");
	return prog;
}

} // end namespace anki

