// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_PIPELINE_H
#define ANKI_GL_GL_PIPELINE_H

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/ShaderHandle.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Program pipeline
class PipelineImpl: public GlObject
{
public:
	using Base = GlObject;

	PipelineImpl() = default;

	~PipelineImpl()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create(
		const ShaderHandle* progsBegin, const ShaderHandle* progsEnd,
		GlAllocator<U8> alloc);

	ShaderHandle getAttachedProgram(GLenum type) const;

	/// Bind the pipeline to the state
	void bind();

private:
	Array<ShaderHandle, 6> m_shaders;

	/// Create pipeline object
	void createPpline();

	/// Attach all the programs
	void attachProgramsInternal(const ShaderHandle* progs, PtrSize count);

	void destroy();
};
/// @}

} // end namespace anki

#endif

