// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_PIPELINE_HANDLE_H
#define ANKI_GR_PIPELINE_HANDLE_H

#include "anki/gr/GrHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Program pipeline handle.
class PipelineHandle: public GrHandle<PipelineImpl>
{
public:
	using Base = GrHandle<PipelineImpl>;

	PipelineHandle();

	~PipelineHandle();

	/// Create a pipeline
	ANKI_USE_RESULT Error create(
		CommandBufferHandle& commands,
		const ShaderHandle* progsBegin, const ShaderHandle* progsEnd)
	{
		return commonConstructor(commands, progsBegin, progsEnd);
	}

	/// Create using initializer list
	ANKI_USE_RESULT Error create(
		CommandBufferHandle& commands,
		std::initializer_list<ShaderHandle> progs);

	/// Bind it to the state
	void bind(CommandBufferHandle& commands);

	/// Get an attached program. It may serialize
	ShaderHandle getAttachedProgram(GLenum type) const;

public:
	ANKI_USE_RESULT Error commonConstructor(CommandBufferHandle& commands,
		const ShaderHandle* progsBegin, const ShaderHandle* progsEnd);
};
/// @}

} // end namespace anki

#endif

