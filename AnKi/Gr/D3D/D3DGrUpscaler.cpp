// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DGrUpscaler.h>

namespace anki {

GrUpscaler* GrUpscaler::newInstance(const GrUpscalerInitInfo& initInfo)
{
	GrUpscalerImpl* impl = anki::newInstance<GrUpscalerImpl>(GrMemoryPool::getSingleton(), initInfo.getName());
	const Error err = impl->initInternal(initInfo);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

GrUpscalerImpl::~GrUpscalerImpl()
{
}

Error GrUpscalerImpl::initInternal([[maybe_unused]] const GrUpscalerInitInfo& initInfo)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki
