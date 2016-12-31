// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/gr/Texture.h>
#include <cstring>

namespace anki
{

SamplerImpl::~SamplerImpl()
{
	if(m_handle)
	{
		vkDestroySampler(getDevice(), m_handle, nullptr);
	}
}

Error SamplerImpl::init(const SamplerInitInfo& ii)
{
	// Fill the create cio
	VkSamplerCreateInfo ci;
	memset(&ci, 0, sizeof(ci));

	ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	if(ii.m_minMagFilter == SamplingFilter::NEAREST)
	{
		ci.magFilter = ci.minFilter = VK_FILTER_NEAREST;
	}
	else
	{
		ANKI_ASSERT(ii.m_minMagFilter == SamplingFilter::LINEAR);
		ci.magFilter = ci.minFilter = VK_FILTER_LINEAR;
	}

	if(ii.m_mipmapFilter == SamplingFilter::BASE || ii.m_mipmapFilter == SamplingFilter::NEAREST)
	{
		ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
	else
	{
		ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	if(ii.m_repeat)
	{
		ci.addressModeU = ci.addressModeV = ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
	else
	{
		ci.addressModeU = ci.addressModeV = ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}

	ci.mipLodBias = 0.0;

	if(ii.m_anisotropyLevel > 0)
	{
		ci.anisotropyEnable = VK_TRUE;
		ci.maxAnisotropy = ii.m_anisotropyLevel;
	}

	ci.compareOp = convertCompareOp(ii.m_compareOperation);
	if(ci.compareOp != VK_COMPARE_OP_ALWAYS)
	{
		ci.compareEnable = VK_TRUE;
	}

	ci.minLod = ii.m_minLod;
	ci.maxLod = ii.m_maxLod;

	ci.unnormalizedCoordinates = VK_FALSE;

	// Create
	ANKI_VK_CHECK(vkCreateSampler(getDevice(), &ci, nullptr, &m_handle));

	return ErrorCode::NONE;
}

} // end namespace anki
