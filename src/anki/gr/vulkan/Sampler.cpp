// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Sampler.h>
#include <anki/gr/vulkan/SamplerImpl.h>

namespace anki
{

Sampler::Sampler(GrManager* manager)
	: GrObject(manager, CLASS_TYPE)
{
}

Sampler::~Sampler()
{
}

void Sampler::init(const SamplerInitInfo& init)
{
	m_impl.reset(getAllocator().newInstance<SamplerImpl>(&getManager()));

	if(m_impl->init(init))
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

} // end namespace anki
