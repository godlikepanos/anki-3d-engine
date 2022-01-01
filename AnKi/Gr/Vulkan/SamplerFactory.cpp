// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/SamplerFactory.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

MicroSampler::~MicroSampler()
{
	if(m_handle)
	{
		vkDestroySampler(m_factory->m_gr->getDevice(), m_handle, nullptr);
	}
}

Error MicroSampler::init(const SamplerInitInfo& inf)
{
	// Fill the create cio
	VkSamplerCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	if(inf.m_minMagFilter == SamplingFilter::NEAREST)
	{
		ci.minFilter = VK_FILTER_NEAREST;
	}
	else if(inf.m_minMagFilter == SamplingFilter::LINEAR)
	{
		ci.minFilter = VK_FILTER_LINEAR;
	}
	else
	{
		ANKI_ASSERT(inf.m_minMagFilter == SamplingFilter::MAX || inf.m_minMagFilter == SamplingFilter::MIN);
		ANKI_ASSERT(m_factory->m_gr->getDeviceCapabilities().m_samplingFilterMinMax);
		ci.minFilter = VK_FILTER_LINEAR;
	}

	ci.magFilter = ci.minFilter;

	if(inf.m_mipmapFilter == SamplingFilter::BASE || inf.m_mipmapFilter == SamplingFilter::NEAREST)
	{
		ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
	else
	{
		ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	switch(inf.m_addressing)
	{
	case SamplingAddressing::CLAMP:
		ci.addressModeU = ci.addressModeV = ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case SamplingAddressing::REPEAT:
		ci.addressModeU = ci.addressModeV = ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	case SamplingAddressing::BLACK:
		ci.addressModeU = ci.addressModeV = ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		break;
	case SamplingAddressing::WHITE:
		ci.addressModeU = ci.addressModeV = ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		break;
	default:
		ANKI_ASSERT(0);
	}

	ci.mipLodBias = inf.m_lodBias;

	if(inf.m_anisotropyLevel > 0)
	{
		ci.anisotropyEnable = VK_TRUE;
		ci.maxAnisotropy = inf.m_anisotropyLevel;
	}

	ci.compareOp = convertCompareOp(inf.m_compareOperation);
	if(ci.compareOp != VK_COMPARE_OP_ALWAYS)
	{
		ci.compareEnable = VK_TRUE;
	}

	ci.minLod = inf.m_minLod;
	ci.maxLod = inf.m_maxLod;

	ci.unnormalizedCoordinates = VK_FALSE;

	// min/max
	VkSamplerReductionModeCreateInfoEXT reductionCi = {};
	if(inf.m_minMagFilter == SamplingFilter::MAX || inf.m_minMagFilter == SamplingFilter::MIN)
	{
		reductionCi.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
		if(inf.m_minMagFilter == SamplingFilter::MAX)
		{
			reductionCi.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX_EXT;
		}
		else
		{
			ANKI_ASSERT(inf.m_minMagFilter == SamplingFilter::MIN);
			reductionCi.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN_EXT;
		}

		ci.pNext = &reductionCi;
	}

	// Create
	ANKI_VK_CHECK(vkCreateSampler(m_factory->m_gr->getDevice(), &ci, nullptr, &m_handle));
	m_factory->m_gr->trySetVulkanHandleName(inf.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, m_handle);

	return Error::NONE;
}

void SamplerFactory::init(GrManagerImpl* gr)
{
	ANKI_ASSERT(gr);
	ANKI_ASSERT(!m_gr);
	m_gr = gr;
}

void SamplerFactory::destroy()
{
	if(!m_gr)
	{
		return;
	}

	GrAllocator<U8> alloc = m_gr->getAllocator();
	for(auto it : m_map)
	{
		MicroSampler* const sampler = it;
		ANKI_ASSERT(sampler->getRefcount().load() == 0 && "Someone still holds a reference to a sampler");
		alloc.deleteInstance(sampler);
	}

	m_map.destroy(alloc);

	m_gr = nullptr;
}

Error SamplerFactory::newInstance(const SamplerInitInfo& inf, MicroSamplerPtr& psampler)
{
	ANKI_ASSERT(m_gr);

	Error err = Error::NONE;
	MicroSampler* out = nullptr;
	const U64 hash = inf.computeHash();

	LockGuard<Mutex> lock(m_mtx);

	auto it = m_map.find(hash);

	if(it != m_map.getEnd())
	{
		out = *it;
	}
	else
	{
		// Create a new one

		GrAllocator<U8> alloc = m_gr->getAllocator();

		out = alloc.newInstance<MicroSampler>(this);
		err = out->init(inf);

		if(err)
		{
			alloc.deleteInstance(out);
			out = nullptr;
		}
		else
		{
			m_map.emplace(alloc, hash, out);
		}
	}

	if(!err)
	{
		psampler.reset(out);
	}

	return err;
}

} // end namespace anki
