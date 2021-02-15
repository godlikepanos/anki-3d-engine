// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/Vulkan/SamplerImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki
{

Sampler* Sampler::newInstance(GrManager* manager, const SamplerInitInfo& init)
{
	SamplerImpl* impl = manager->getAllocator().newInstance<SamplerImpl>(manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki
