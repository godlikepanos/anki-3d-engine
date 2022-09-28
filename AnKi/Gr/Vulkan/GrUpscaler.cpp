// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrUpscaler.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Vulkan/GrUpscalerImpl.h>

namespace anki {

GrUpscaler* GrUpscaler::newInstance(GrManager* manager, const GrUpscalerInitInfo& initInfo)
{
	GrUpscalerImpl* impl = anki::newInstance<GrUpscalerImpl>(manager->getMemoryPool(), manager, initInfo.getName());
	const Error err = impl->initInternal(initInfo);
	if(err)
	{
		deleteInstance(manager->getMemoryPool(), impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki
