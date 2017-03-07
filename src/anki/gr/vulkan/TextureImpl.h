// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/gr/common/Misc.h>
#include <anki/util/HashMap.h>

namespace anki
{

// Forward
class TextureUsageTracker;
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
class TextureImpl : public VulkanObject
{
public:
	SamplerPtr m_sampler;

	VkImage m_imageHandle = VK_NULL_HANDLE;

	GpuMemoryHandle m_memHandle;

	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	TextureType m_type = TextureType::CUBE;
	U8 m_mipCount = 0;
	U32 m_layerCount = 0;
	U32 m_surfaceOrVolumeCount = 0;
	VkImageAspectFlags m_aspect = 0;
	DepthStencilAspectBit m_akAspect = DepthStencilAspectBit::NONE;
	TextureUsageBit m_usage = TextureUsageBit::NONE;
	TextureUsageBit m_usageWhenEncountered = TextureUsageBit::NONE;
	PixelFormat m_format;
	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;

	Bool m_depthStencil = false;
	TextureImplWorkaround m_workarounds = TextureImplWorkaround::NONE;

	TextureImpl(GrManager* manager, U64 uuid);

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init, Texture* tex);

	void checkSurfaceOrVolume(const TextureSurfaceInfo& surf) const
	{
		checkTextureSurface(m_type, m_depth, m_mipCount, m_layerCount, surf);
	}

	void checkSurfaceOrVolume(const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_type == TextureType::_3D);
		ANKI_ASSERT(vol.m_level < m_mipCount);
	}

	void computeSubResourceRange(
		const TextureSurfaceInfo& surf, DepthStencilAspectBit aspect, VkImageSubresourceRange& range) const;

	void computeSubResourceRange(
		const TextureVolumeInfo& vol, DepthStencilAspectBit aspect, VkImageSubresourceRange& range) const;

	/// Compute the layer as defined by Vulkan.
	U computeVkArrayLayer(const TextureSurfaceInfo& surf) const;

	U computeVkArrayLayer(const TextureVolumeInfo& vol) const
	{
		return 0;
	}

	Bool usageValid(TextureUsageBit usage) const
	{
		return (usage & m_usage) == usage;
	}

	VkImageView getOrCreateSingleSurfaceView(const TextureSurfaceInfo& surf, DepthStencilAspectBit aspect);

	/// That view will be used in descriptor sets.
	VkImageView getOrCreateResourceGroupView(DepthStencilAspectBit aspect);

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

	VkImageAspectFlags convertAspect(DepthStencilAspectBit ak) const;

	void checkSubresourceRange(const VkImageSubresourceRange& range) const
	{
		ANKI_ASSERT(range.baseArrayLayer < m_layerCount);
		ANKI_ASSERT(range.baseArrayLayer + range.layerCount <= m_layerCount);
		ANKI_ASSERT(range.levelCount < m_mipCount);
		ANKI_ASSERT(range.baseMipLevel + range.levelCount <= m_mipCount);
	}

	template<typename TextureInfo>
	VkImageLayout findLayoutFromTracker(const TextureInfo& surfOrVol, const TextureUsageTracker& tracker) const;

	VkImageLayout findLayoutFromTracker(const TextureUsageTracker& tracker) const;

	template<typename TextureInfo>
	void updateTracker(const TextureInfo& surfOrVol, TextureUsageBit usage, TextureUsageTracker& tracker) const;

	void updateTracker(TextureUsageBit usage, TextureUsageTracker& tracker) const;

private:
	class ViewHasher
	{
	public:
		U64 operator()(const VkImageViewCreateInfo& b) const
		{
			return computeHash(&b, sizeof(b));
		}
	};

	class ViewCompare
	{
	public:
		Bool operator()(const VkImageViewCreateInfo& a, const VkImageViewCreateInfo& b) const
		{
			return memcmp(&a, &b, sizeof(a)) == 0;
		}
	};

	HashMap<VkImageViewCreateInfo, VkImageView, ViewHasher, ViewCompare> m_viewsMap;
	Mutex m_viewsMapMtx;
	VkImageViewCreateInfo m_viewCreateInfoTemplate;
	U64 m_uuid; ///< Steal the UUID from the Texture.

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(const TextureInitInfo& init);

	VkImageView getOrCreateView(const VkImageViewCreateInfo& ci);

	U computeSubresourceIdx(const TextureSurfaceInfo& surf) const;

	U computeSubresourceIdx(const TextureVolumeInfo& vol) const
	{
		checkSurfaceOrVolume(vol);
		return vol.m_level;
	}

	template<typename TextureInfo>
	void updateUsageState(
		const TextureInfo& surfOrVol, TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const;

	void updateUsageState(TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const;
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/TextureImpl.inl.h>
