// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SamplerFactory.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

MicroSampler::~MicroSampler()
{
	// TODO
}

GrAllocator<U8> MicroSampler::getAllocator() const
{
	return m_factory->m_gr->getAllocator();
}

Error MicroSampler::init(const SamplerInitInfo& inf)
{
	// TODO
	return Error::NONE;
}

Error SamplerFactory::init(GrManagerImpl* gr)
{
	ANKI_ASSERT(gr);
	// TODO
	return Error::NONE;
}

void SamplerFactory::destroy()
{
	// TODO
}

Error SamplerFactory::newInstance(const SamplerInitInfo& inf, MicroSamplerPtr& psampler)
{
	// TODO
	return Error::NONE;
}

} // end namespace anki
