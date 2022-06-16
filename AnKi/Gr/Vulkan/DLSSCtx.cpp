// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/DLSSCtx.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Vulkan/DLSSCtxImpl.h>

namespace anki {

DLSSCtx* DLSSCtx::newInstance(GrManager* manager, const DLSSCtxInitInfo& init)
{
	DLSSCtxImpl* impl(nullptr);
	impl = manager->getAllocator().newInstance<DLSSCtxImpl>(manager, "");
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

void DLSSCtx::upscale(CommandBufferPtr cmdb, const TextureViewPtr& srcRt, const TextureViewPtr& dstRt,
					  const TextureViewPtr& mvRt, const TextureViewPtr& depthRt, const TextureViewPtr& exposure,
					  const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& mVScale)
{
	ANKI_VK_SELF(DLSSCtxImpl);
	self.upscale(cmdb, srcRt, dstRt, mvRt, depthRt, exposure, resetAccumulation, jitterOffset, mVScale);
}

} // end namespace anki
