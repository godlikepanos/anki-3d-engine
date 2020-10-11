// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

Error SamplerImpl::init(const SamplerInitInfo& inf)
{
	return getGrManagerImpl().getSamplerFactory().newInstance(inf, m_sampler);
}

} // end namespace anki
