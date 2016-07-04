// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryAllocator.h>
#include <anki/gr/vulkan/Semaphore.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Texture container.
class TextureImpl : public VulkanObject
{
public:
	VkImage m_imageHandle = VK_NULL_HANDLE;
	VkImageView m_viewHandle = VK_NULL_HANDLE;
	SamplerPtr m_sampler;
	U32 m_memIdx = MAX_U32;
	GpuMemoryAllocationHandle m_memHandle;

	U32 m_width = 0;
	U32 m_height = 0;
	TextureType m_type = TextureType::CUBE;
	U8 m_mipCount = 0;
	U32 m_layerCount = 0;
	VkImageAspectFlags m_aspect = 0;
	VkImageLayout m_optimalLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

	TextureImpl(GrManager* manager);

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init, Texture* tex);

private:
	class CreateContext;

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
