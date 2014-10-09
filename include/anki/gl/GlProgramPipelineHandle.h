// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PROGRAM_PIPELINE_HANDLE_H
#define ANKI_GL_GL_PROGRAM_PIPELINE_HANDLE_H

#include "anki/gl/GlContainerHandle.h"

namespace anki {

// Forward
class GlCommandBufferHandle;
class GlProgramPipeline;
class GlProgramHandle;

/// @addtogroup opengl_other
/// @{

/// Program pipeline handle
class GlProgramPipelineHandle: public GlContainerHandle<GlProgramPipeline>
{
public:
	using Base = GlContainerHandle<GlProgramPipeline>;

	GlProgramPipelineHandle();

	~GlProgramPipelineHandle();

	/// Create a pipeline
	ANKI_USE_RESULT Error create(
		GlCommandBufferHandle& commands,
		const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
	{
		return commonConstructor(commands, progsBegin, progsEnd);
	}

	/// Create using initializer list
	ANKI_USE_RESULT Error create(
		GlCommandBufferHandle& commands,
		std::initializer_list<GlProgramHandle> progs);

	/// Bind it to the state
	void bind(GlCommandBufferHandle& commands);

	/// Get an attached program. It may serialize
	GlProgramHandle getAttachedProgram(GLenum type) const;

public:
	ANKI_USE_RESULT Error commonConstructor(GlCommandBufferHandle& commands,
		const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd);
};

/// @}

} // end namespace anki

#endif

