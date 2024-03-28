// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkSampler.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

namespace anki {

Sampler* Sampler::newInstance(const SamplerInitInfo& init)
{
	SamplerImpl* impl = anki::newInstance<SamplerImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = getGrManagerImpl().getSamplerFactory().newInstance(init, impl->m_sampler);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki
