// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_PIPELINE_HANDLE_H
#define ANKI_GR_PIPELINE_HANDLE_H

#include "anki/gr/GrHandle.h"
#include "anki/gr/PipelineCommon.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Program pipeline handle.
class PipelineHandle: public GrHandle<PipelineImpl>
{
public:
	using Base = GrHandle<PipelineImpl>;
	using Initializer = PipelineInitializer;

	PipelineHandle();

	~PipelineHandle();

	/// Create a pipeline
	ANKI_USE_RESULT Error create(
		CommandBufferHandle& commands,
		const Initializer& init);

	/// Bind it to the state
	void bind(CommandBufferHandle& commands);
};
/// @}

} // end namespace anki

#endif

