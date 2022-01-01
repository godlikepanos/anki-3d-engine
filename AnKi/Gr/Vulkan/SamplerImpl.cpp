// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/SamplerImpl.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

Error SamplerImpl::init(const SamplerInitInfo& inf)
{
	return getGrManagerImpl().getSamplerFactory().newInstance(inf, m_sampler);
}

} // end namespace anki
