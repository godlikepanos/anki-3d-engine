// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_PIPELINE_IMPL_H
#define ANKI_GR_GL_PIPELINE_IMPL_H

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/ShaderHandle.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Program pipeline
class PipelineImpl: public GlObject
{
public:
	using Base = GlObject;

	PipelineImpl(GrManager* manager)
	:	Base(manager)
	{}

	~PipelineImpl()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create(
		const ShaderHandle* progsBegin, const ShaderHandle* progsEnd);

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

