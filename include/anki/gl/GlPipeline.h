// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PIPELINE_H
#define ANKI_GL_GL_PIPELINE_H

#include "anki/gl/GlObject.h"
#include "anki/gl/GlShaderHandle.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Program pipeline
class GlPipeline: public GlObject
{
public:
	using Base = GlObject;

	GlPipeline() = default;

	~GlPipeline()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create(
		const GlShaderHandle* progsBegin, const GlShaderHandle* progsEnd,
		GlAllocator<U8> alloc);

	GlShaderHandle getAttachedProgram(GLenum type) const;

	/// Bind the pipeline to the state
	void bind();

private:
	Array<GlShaderHandle, 6> m_shaders;

	/// Create pipeline object
	void createPpline();

	/// Attach all the programs
	void attachProgramsInternal(const GlShaderHandle* progs, PtrSize count);

	void destroy();
};

/// @}

} // end namespace anki

#endif

