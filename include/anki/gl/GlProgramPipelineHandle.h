// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PROGRAM_PIPELINE_HANDLE_H
#define ANKI_GL_GL_PROGRAM_PIPELINE_HANDLE_H

#include "anki/gl/GlContainerHandle.h"

namespace anki {

// Forward
class GlJobChainHandle;
class GlProgramPipeline;
class GlProgramHandle;

/// @addtogroup opengl_other
/// @{

/// Program pipeline handle
class GlProgramPipelineHandle: public GlContainerHandle<GlProgramPipeline>
{
public:
	using Base = GlContainerHandle<GlProgramPipeline>;

	/// @name Contructors/Destructor
	/// @{
	GlProgramPipelineHandle();

	/// Create a pipeline
	explicit GlProgramPipelineHandle(
		GlJobChainHandle& jobs,
		const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
	{
		commonConstructor(jobs, progsBegin, progsEnd);
	}

	/// Create using initializer list
	explicit GlProgramPipelineHandle(
		GlJobChainHandle& jobs,
		std::initializer_list<GlProgramHandle> progs);

	~GlProgramPipelineHandle();
	/// @}

	/// Bind it to the state
	void bind(GlJobChainHandle& jobs);

	/// Get an attached program. It may serialize
	GlProgramHandle getAttachedProgram(GLenum type) const;

public:
	void commonConstructor(GlJobChainHandle& jobs,
		const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd);
};

/// @}

} // end namespace anki

#endif

