// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

Error SamplerImpl::init(const SamplerInitInfo& inf_)
{
	SamplerInitInfo inf = inf_;

	// Set a constant name because the name will take part in hashing. If it's unique every time then there is no point
	// in having the SamplerFactory
	inf.setName("SamplerSampler");

	return getGrManagerImpl().getSamplerFactory().newInstance(inf, m_sampler);
}

} // end namespace anki
