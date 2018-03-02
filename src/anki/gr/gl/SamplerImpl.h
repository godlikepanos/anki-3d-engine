// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Sampler.h>
#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Sampler GL object.
class SamplerImpl final : public Sampler, public GlObject
{
public:
	SamplerImpl(GrManager* manager, CString name)
		: Sampler(manager, name)
	{
	}

	~SamplerImpl()
	{
		destroyDeferred(getManager(), glDeleteSamplers);
	}

	void init(const SamplerInitInfo& sinit);
};
/// @}

} // end namespace anki
