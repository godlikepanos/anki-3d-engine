// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Sampler.h>
#include <anki/gr/vulkan/SamplerImpl.h>

namespace anki
{

//==============================================================================
Sampler::Sampler(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
Sampler::~Sampler()
{
}

//==============================================================================
void Sampler::init(const SamplerInitInfo& init)
{
}

} // end namespace anki