// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrUpscaler.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Vulkan/GrUpscalerImpl.h>

namespace anki {

GrUpscaler* GrUpscaler::newInstance(GrManager* manager, const GrUpscalerInitInfo& init)
{
	GrUpscalerImpl* impl(nullptr);
	impl = manager->getAllocator().newInstance<GrUpscalerImpl>(manager, "");
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

Error GrUpscaler::init(const GrUpscalerInitInfo& init)
{
	m_initInfo = init;
	ANKI_VK_SELF(GrUpscalerImpl);
	return self.initInternal();
}

} // end namespace anki
