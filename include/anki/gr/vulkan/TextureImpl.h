// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryAllocator.h>
#include <anki/gr/vulkan/Semaphore.h>
#include <anki/gr/common/Misc.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

enum class TextureImplWorkaround : U8
{
	NONE,
	R8G8B8_TO_R8G8B8A8 = 1 << 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureImplWorkaround, inline)

/// Texture container.
class TextureImpl : public VulkanObject
{
public:
	SamplerPtr m_sampler;

	VkImage m_imageHandle = VK_NULL_HANDLE;
	VkImageView m_viewHandle = VK_NULL_HANDLE;

	/// A number of views, one for each level. Used in image load/store.
	DynamicArray<VkImageView> m_viewsEveryLevel;

	U32 m_memIdx = MAX_U32;
	GpuMemoryAllocationHandle m_memHandle;

	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	TextureType m_type = TextureType::CUBE;
	U8 m_mipCount = 0;
	U32 m_layerCount = 0;
	VkImageAspectFlags m_aspect = 0;
	TextureUsageBit m_usage = TextureUsageBit::NONE;
	PixelFormat m_format;

	Bool m_depthStencil = false;
	TextureImplWorkaround m_workarounds = TextureImplWorkaround::NONE;

	TextureImpl(GrManager* manager);

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init, Texture* tex);

	void checkSurface(const TextureSurfaceInfo& surf) const
	{
		checkTextureSurface(m_type, m_depth, m_mipCount, m_layerCount, surf);
	}

	void computeSubResourceRange(
		const TextureSurfaceInfo& surf, VkImageSubresourceRange& range) const;

	Bool usageValid(TextureUsageBit usage) const
	{
		return (usage & m_usage) == usage;
	}

	/// By knowing the previous and new texture usage calculate the relavant
	/// info for a ppline barrier.
	void computeBarrierInfo(TextureUsageBit before,
		TextureUsageBit after,
		U level,
		VkPipelineStageFlags& srcStages,
		VkAccessFlags& srcAccesses,
		VkPipelineStageFlags& dstStages,
		VkAccessFlags& dstAccesses) const;

	/// Predict the image layout.
	VkImageLayout computeLayout(TextureUsageBit usage, U level) const;

private:
	class CreateContext;

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(
		const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(
		const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(CreateContext& ctx);
	ANKI_USE_RESULT Error initView(CreateContext& ctx);
};

//==============================================================================
inline void TextureImpl::computeSubResourceRange(
	const TextureSurfaceInfo& surf, VkImageSubresourceRange& range) const
{
	checkSurface(surf);
	range.aspectMask = m_aspect;
	range.baseMipLevel = surf.m_level;
	range.levelCount = 1;
	switch(m_type)
	{
	case TextureType::_2D:
		range.baseArrayLayer = 0;
		break;
	case TextureType::_3D:
		range.baseArrayLayer = surf.m_depth;
		break;
	case TextureType::CUBE:
		range.baseArrayLayer = surf.m_face;
		break;
	case TextureType::_2D_ARRAY:
		range.baseArrayLayer = surf.m_layer;
		break;
	case TextureType::CUBE_ARRAY:
		range.baseArrayLayer = surf.m_layer * 6 + surf.m_face;
		break;
	default:
		ANKI_ASSERT(0);
	}
	range.layerCount = 1;
}
/// @}

} // end namespace anki
