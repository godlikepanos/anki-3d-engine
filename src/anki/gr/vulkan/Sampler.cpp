// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Sampler.h>
#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Sampler* Sampler::newInstance(GrManager* manager, const SamplerInitInfo& init)
{
	SamplerImpl* impl = manager->getAllocator().newInstance<SamplerImpl>(manager, init.getName());
	Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
	}
	return impl;
}

} // end namespace anki
