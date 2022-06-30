// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrUpscaler.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Vulkan/GrUpscalerImpl.h>

namespace anki {

GrUpscaler* GrUpscaler::newInstance(GrManager* manager, const GrUpscalerInitInfo& initInfo)
{
	GrUpscalerImpl* impl(nullptr);
	impl = manager->getAllocator().newInstance<GrUpscalerImpl>(manager, "");
	ANKI_ASSERT(impl);
	impl->m_upscalerType = initInfo.m_upscalerType;
	const Error err = impl->initInternal(initInfo);

	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki
