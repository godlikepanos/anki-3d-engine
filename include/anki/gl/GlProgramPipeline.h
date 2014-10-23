// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PROGRAM_PIPELINE_H
#define ANKI_GL_GL_PROGRAM_PIPELINE_H

#include "anki/gl/GlObject.h"
#include "anki/gl/GlProgramHandle.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Program pipeline
class GlProgramPipeline: public GlObject
{
public:
	using Base = GlObject;

	GlProgramPipeline() = default;

	~GlProgramPipeline()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create(
		const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd,
		GlAllocator<U8> alloc);

	GlProgramHandle getAttachedProgram(GLenum type) const;

	/// Bind the pipeline to the state
	void bind();

private:
	Array<GlProgramHandle, 6> m_progs;

	/// Create pipeline object
	void createPpline();

	/// Attach all the programs
	void attachProgramsInternal(const GlProgramHandle* progs, PtrSize count);

	void destroy();
};

/// @}

} // end namespace anki

#endif

