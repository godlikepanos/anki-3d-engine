// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/gr/common/Misc.h>
#include <anki/gr/vulkan/SamplerFactory.h>
#include <anki/util/HashMap.h>

namespace anki
{

// Forward
class TextureUsageState;

/// @addtogroup vulkan
/// @{

enum class TextureImplWorkaround : U8
{
	NONE,
	R8G8B8_TO_R8G8B8A8 = 1 << 0,
	S8_TO_D24S8 = 1 << 1,
	D24S8_TO_D32S8 = 1 << 2,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureImplWorkaround, inline)

/// Texture container.
class TextureImpl final : public Texture, public VulkanObject<Texture, TextureImpl>
{
public:
	VkImage m_imageHandle = VK_NULL_HANDLE;

	GpuMemoryHandle m_memHandle;

	U32 m_surfaceOrVolumeCount = 0;

	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;

	TextureImplWorkaround m_workarounds = TextureImplWorkaround::NONE;

	VkImageViewCreateInfo m_viewCreateInfoTemplate;

	TextureImpl(GrManager* manager, CString name)
		: Texture(manager, name)
	{
	}

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init);

	Bool aspectValid(DepthStencilAspectBit aspect) const
	{
		return m_aspect == aspect || !!(aspect & m_aspect);
	}

	/// Compute the layer as defined by Vulkan.
	U computeVkArrayLayer(const TextureSurfaceInfo& surf) const
	{
		U layer = 0;
		switch(m_texType)
		{
		case TextureType::_2D:
			layer = 0;
			break;
		case TextureType::CUBE:
			layer = surf.m_face;
			break;
		case TextureType::_2D_ARRAY:
			layer = surf.m_layer;
			break;
		case TextureType::CUBE_ARRAY:
			layer = surf.m_layer * 6 + surf.m_face;
			break;
		default:
			ANKI_ASSERT(0);
		}

		return layer;
	}

	U computeVkArrayLayer(const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_texType == TextureType::_3D);
		return 0;
	}

	Bool usageValid(TextureUsageBit usage) const
	{
		return (usage & m_usage) == usage;
	}

	/// By knowing the previous and new texture usage calculate the relavant info for a ppline barrier.
	void computeBarrierInfo(TextureUsageBit before,
		TextureUsageBit after,
		U level,
		VkPipelineStageFlags& srcStages,
		VkAccessFlags& srcAccesses,
		VkPipelineStageFlags& dstStages,
		VkAccessFlags& dstAccesses) const;

	/// Predict the image layout.
	VkImageLayout computeLayout(TextureUsageBit usage, U level) const;

	void computeVkImageSubresourceRange(const TextureSubresourceInfo& in, VkImageSubresourceRange& range) const
	{
		ANKI_ASSERT(isSubresourceValid(in));

		range.aspectMask = convertImageAspect(in.m_depthStencilAspect);
		range.baseMipLevel = in.m_firstMipmap;
		range.levelCount = in.m_mipmapCount;

		const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
		range.baseArrayLayer = in.m_firstLayer * faceCount + in.m_firstFace;
		range.layerCount = in.m_layerCount * in.m_faceCount;
	}

	void computeVkImageViewCreateInfo(
		const TextureSubresourceInfo& subresource, VkImageViewCreateInfo& viewCi, TextureType& newTextureType) const
	{
		ANKI_ASSERT(isSubresourceValid(subresource));

		viewCi = m_viewCreateInfoTemplate;
		computeVkImageSubresourceRange(subresource, viewCi.subresourceRange);

		// Fixup the image view type
		newTextureType = computeNewTexTypeOfSubresource(subresource);
		if(newTextureType == TextureType::_2D)
		{
			// Change that anyway
			viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}
	}

	VkImageView getOrCreateView(const TextureSubresourceInfo& subresource, TextureType& newTexType) const;

private:
	mutable HashMap<TextureSubresourceInfo, VkImageView> m_viewsMap;
	mutable Mutex m_viewsMapMtx;

	VkDeviceMemory m_dedicatedMem = VK_NULL_HANDLE;

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(const TextureInitInfo& init);

	template<typename TextureInfo>
	void updateUsageState(
		const TextureInfo& surfOrVol, TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const;

	void updateUsageState(TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const;

	/// Compute the new type of a texture view.
	TextureType computeNewTexTypeOfSubresource(const TextureSubresourceInfo& subresource) const
	{
		ANKI_ASSERT(isSubresourceValid(subresource));
		return (textureTypeIsCube(m_texType) && subresource.m_faceCount != 6) ? TextureType::_2D : m_texType;
	}
};
/// @}

} // end namespace anki
