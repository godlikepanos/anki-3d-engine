// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_SAMPLER_HANDLE_H
#define ANKI_GR_SAMPLER_HANDLE_H

#include "anki/gr/TextureSamplerCommon.h"
#include "anki/gr/GrPtr.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Sampler.
class SamplerPtr: public GrPtr<SamplerImpl>
{
public:
	using Base = GrPtr<SamplerImpl>;
	using Initializer = SamplerInitializer;

	/// Create husk.
	SamplerPtr();

	~SamplerPtr();

	/// Create the sampler
	void create(CommandBufferPtr& commands, const Initializer& init);

	/// Bind to a unit
	void bind(CommandBufferPtr& commands, U32 unit);

	/// Bind default sampler
	static void bindDefault(CommandBufferPtr& commands, U32 unit);
};
/// @}

} // end namespace anki

#endif

