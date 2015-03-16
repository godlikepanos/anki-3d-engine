// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/PipelineImpl.h"
#include "anki/gr/gl/ShaderImpl.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
Error PipelineImpl::create(
	const ShaderHandle* progsBegin, const ShaderHandle* progsEnd)
{
	ANKI_ASSERT(progsBegin != nullptr && progsEnd != nullptr);
	ANKI_ASSERT(progsBegin != progsEnd);

	Error err = ErrorCode::NONE;

	attachProgramsInternal(progsBegin, progsEnd - progsBegin);

	// Create and attach programs
	glGenProgramPipelines(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	glBindProgramPipeline(m_glName);

	for(U i = 0; i < m_shaders.size(); i++)
	{
		ShaderHandle& prog = m_shaders[i];

		if(prog.isCreated())
		{
			GLbitfield bit;
			GLenum gltype = 
				computeGlShaderType(static_cast<ShaderType>(i), &bit);
			ANKI_ASSERT(prog.getType() == gltype && "Attached wrong shader");
			(void)gltype;
			glUseProgramStages(m_glName, bit, prog.get().getGlName());
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
		err = infoLogTxt.create(alloc, infoLen + 1);

		if(!err)
		{
			glGetProgramPipelineInfoLog(
				m_glName, infoLen, &charsWritten, &infoLogTxt[0]);

			ANKI_LOGE("Ppline error log follows:\n%s", &infoLogTxt[0]);
			err = ErrorCode::USER_DATA;
		}
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
void PipelineImpl::attachProgramsInternal(
	const ShaderHandle* progs, PtrSize count)
{
	U mask = 0;
	while(count-- != 0)
	{
		const ShaderHandle& prog = progs[count];
		ShaderType type = computeShaderTypeIndex(prog.get().getType());
		U idx = enumToType(type);

		ANKI_ASSERT(!m_shaders[idx].isCreated() && "Attaching the same");
		m_shaders[idx] = prog;
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
void PipelineImpl::bind()
{
	ANKI_ASSERT(isCreated());
	glBindProgramPipeline(m_glName);
}

//==============================================================================
ShaderHandle PipelineImpl::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	ShaderType stype = computeShaderTypeIndex(type);
	U idx = enumToType(stype);
	ShaderHandle prog = m_shaders[idx];
	ANKI_ASSERT(prog.isCreated() && "Asking for non-created program");
	return prog;
}

} // end namespace anki

