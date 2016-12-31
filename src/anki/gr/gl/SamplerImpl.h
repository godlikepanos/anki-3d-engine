// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Sampler GL object.
class SamplerImpl : public GlObject
{
public:
	SamplerImpl(GrManager* manager)
		: GlObject(manager)
	{
	}

	~SamplerImpl()
	{
		destroyDeferred(glDeleteSamplers);
	}

	void init(const SamplerInitInfo& sinit);
};
/// @}

} // end namespace anki
