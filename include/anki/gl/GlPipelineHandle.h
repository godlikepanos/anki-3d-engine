// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PIPELINE_HANDLE_H
#define ANKI_GL_GL_PIPELINE_HANDLE_H

#include "anki/gl/GlContainerHandle.h"

namespace anki {

// Forward
class GlCommandBufferHandle;
class GlPipeline;
class GlShaderHandle;

/// @addtogroup opengl_other
/// @{

/// Program pipeline handle
class GlPipelineHandle: public GlContainerHandle<GlPipeline>
{
public:
	using Base = GlContainerHandle<GlPipeline>;

	GlPipelineHandle();

	~GlPipelineHandle();

	/// Create a pipeline
	ANKI_USE_RESULT Error create(
		GlCommandBufferHandle& commands,
		const GlShaderHandle* progsBegin, const GlShaderHandle* progsEnd)
	{
		return commonConstructor(commands, progsBegin, progsEnd);
	}

	/// Create using initializer list
	ANKI_USE_RESULT Error create(
		GlCommandBufferHandle& commands,
		std::initializer_list<GlShaderHandle> progs);

	/// Bind it to the state
	void bind(GlCommandBufferHandle& commands);

	/// Get an attached program. It may serialize
	GlShaderHandle getAttachedProgram(GLenum type) const;

public:
	ANKI_USE_RESULT Error commonConstructor(GlCommandBufferHandle& commands,
		const GlShaderHandle* progsBegin, const GlShaderHandle* progsEnd);
};

/// @}

} // end namespace anki

#endif

