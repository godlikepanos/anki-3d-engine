// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrUpscaler.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/CommandBuffer.h>
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

void GrUpscaler::upscale(CommandBufferPtr cmdb, const TextureViewPtr& srcRt, const TextureViewPtr& dstRt,
						 const TextureViewPtr& mvRt, const TextureViewPtr& depthRt, const TextureViewPtr& exposure,
						 const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& mVScale)
{
	ANKI_VK_SELF(GrUpscalerImpl);
	self.upscale(cmdb, srcRt, dstRt, mvRt, depthRt, exposure, resetAccumulation, jitterOffset, mVScale);
}

} // end namespace anki
