// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE;

	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;

	TextureImplWorkaround m_workarounds = TextureImplWorkaround::NONE;

	VkImageViewCreateInfo m_viewCreateInfoTemplate;

	TextureImpl(GrManager* manager)
		: Texture(manager)
	{
	}

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init);

	Bool aspectValid(DepthStencilAspectBit aspect) const
	{
		return m_aspect == aspect || !!(aspect & m_aspect);
	}

	void checkSurfaceOrVolume(const TextureSurfaceInfo& surf) const
	{
		checkTextureSurface(m_texType, m_depth, m_mipCount, m_layerCount, surf);
	}

	void checkSurfaceOrVolume(const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_texType == TextureType::_3D);
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

	/// That view will be used in descriptor sets.
	VkImageView getOrCreateResourceGroupView(DepthStencilAspectBit aspect) const;

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

	void checkSubresourceRange(const VkImageSubresourceRange& range) const
	{
		ANKI_ASSERT(range.baseArrayLayer < m_layerCount);
		ANKI_ASSERT(range.baseArrayLayer + range.layerCount <= m_layerCount);
		ANKI_ASSERT(range.levelCount < m_mipCount);
		ANKI_ASSERT(range.baseMipLevel + range.levelCount <= m_mipCount);
	}

	void computeSubresourceRange(const TextureSubresourceInfo& in, VkImageSubresourceRange& range) const
	{
		ANKI_ASSERT(isSubresourceValid(in));

		range.aspectMask = convertImageAspect(in.m_depthStencilAspect & m_aspect);
		range.baseMipLevel = in.m_baseMipmap;
		range.levelCount = in.m_mipmapCount;

		const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
		range.baseArrayLayer = in.m_baseLayer * faceCount + in.m_baseFace;
		range.layerCount = in.m_layerCount * in.m_faceCount;
	}

	VkImageView getOrCreateView(const VkImageViewCreateInfo& ci) const;

private:
	class ViewHasher
	{
	public:
		U64 operator()(const VkImageViewCreateInfo& b) const
		{
			return computeHash(&b, sizeof(b));
		}
	};

	mutable HashMap<VkImageViewCreateInfo, VkImageView, ViewHasher> m_viewsMap;
	mutable Mutex m_viewsMapMtx;

	VkDeviceMemory m_dedicatedMem = VK_NULL_HANDLE;

	Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(const TextureInitInfo& init);

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
