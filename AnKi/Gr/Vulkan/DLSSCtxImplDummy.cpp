// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/DLSSCtxImpl.h>

namespace anki {


DLSSCtxImpl::~DLSSCtxImpl()
{
	// TODO: 
}

Error DLSSCtxImpl::init(const UVec2& srcRes, const UVec2& dstRes, const DLSSQualityMode mode)
{
	return Error::NONE;
}

void DLSSCtxImpl::upscale(CommandBufferPtr cmdb,
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
	ANKI_VK_LOGE("Trying to use DLSS when not supported");
}

} // end namespace anki
