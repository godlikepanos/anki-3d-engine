// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GrUpscalerImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Vulkan/CommandBufferImpl.h>

namespace anki {

GrUpscalerImpl::~GrUpscalerImpl()
{
	// TODO:
}

Error GrUpscalerImpl::init(const GrUpscalerInitInfo& init)
{
	return Error::NONE;
}

void GrUpscalerImpl::upscale(CommandBufferPtr cmdb, const TextureViewPtr& srcRt, const TextureViewPtr& dstRt,
							 const TextureViewPtr& mvRt, const TextureViewPtr& depthRt, const TextureViewPtr& exposure,
							 const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& mVScale)
{
	ANKI_VK_LOGE("Trying to use DLSS when not supported");
}

} // end namespace anki
