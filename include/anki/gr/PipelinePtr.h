// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_PIPELINE_HANDLE_H
#define ANKI_GR_PIPELINE_HANDLE_H

#include "anki/gr/GrPtr.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Program pipeline.
class PipelinePtr: public GrPtr<PipelineImpl>
{
public:
	using Base = GrPtr<PipelineImpl>;
	using Initializer = PipelineInitializer;

	PipelinePtr();

	~PipelinePtr();

	/// Create a pipeline
	ANKI_USE_RESULT Error create(GrManager* manager, const Initializer& init);

	/// Bind it to the state
	void bind(CommandBufferPtr& commands);
};
/// @}

} // end namespace anki

#endif

