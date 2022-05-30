// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/DLSSCtx.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/CommandBuffer.h>
#if DLSS_SUPPORT
#include <AnKi/Gr/Vulkan/DLSSCtxImpl.h>
#endif

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

void DLSSCtx::upscale(CommandBufferPtr cmdb,
	const TexturePtr srcRt,
	const TexturePtr dstRt,
	const TexturePtr mvRt,
	const TexturePtr depthRt,
	const TexturePtr exposure,
	const Bool resetAccumulation,
	const F32 sharpness,
	const Vec2& jitterOffset,
	const Vec2& mVScale) 
{
	ANKI_VK_SELF(DLSSCtxImpl);
	self.upscale(cmdb, srcRt, dstRt, mvRt, depthRt, exposure, resetAccumulation, sharpness, jitterOffset, mVScale);
}

} // end namespace anki
