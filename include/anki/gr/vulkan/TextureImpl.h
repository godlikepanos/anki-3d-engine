// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryAllocator.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Texture container.
class TextureImpl : public VulkanObject
{
public:
	TextureImpl(GrManager* manager);

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init);

	const SamplerPtr& getSampler() const
	{
		return m_sampler;
	}

	VkImage getImageHandle() const
	{
		ANKI_ASSERT(m_imageHandle);
		return m_imageHandle;
	}

private:
	class CreateContext;

	VkImage m_imageHandle = VK_NULL_HANDLE;
	VkImageView m_viewHandle = VK_NULL_HANDLE;
	SamplerPtr m_sampler;
	U32 m_memIdx = MAX_U32;
	GpuMemoryAllocationHandle m_memHandle;

	U8 m_mipCount = 0;
	TextureType m_type = TextureType::CUBE;
	VkImageAspectFlags m_aspect = 0;

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(
		const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageUsageFlags calcUsage(
		const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(
		const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(CreateContext& ctx);
	ANKI_USE_RESULT Error initView(CreateContext& ctx);
};
/// @}

} // end namespace anki
