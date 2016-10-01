// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/gr/vulkan/Semaphore.h>
#include <anki/gr/common/Misc.h>
#include <anki/util/HashMap.h>

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

	GpuMemoryHandle m_memHandle;

	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	TextureType m_type = TextureType::CUBE;
	U8 m_mipCount = 0;
	U32 m_layerCount = 0;
	VkImageAspectFlags m_aspect = 0;
	DepthStencilAspectMask m_akAspect = DepthStencilAspectMask::NONE;
	TextureUsageBit m_usage = TextureUsageBit::NONE;
	PixelFormat m_format;
	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;

	Bool m_depthStencil = false;
	TextureImplWorkaround m_workarounds = TextureImplWorkaround::NONE;

	TextureImpl(GrManager* manager);

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init, Texture* tex);

	void checkSurface(const TextureSurfaceInfo& surf) const
	{
		checkTextureSurface(m_type, m_depth, m_mipCount, m_layerCount, surf);
	}

	void checkVolume(const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_type == TextureType::_3D);
		ANKI_ASSERT(vol.m_level < m_mipCount);
	}

	void computeSubResourceRange(
		const TextureSurfaceInfo& surf, DepthStencilAspectMask aspect, VkImageSubresourceRange& range) const;

	void computeSubResourceRange(
		const TextureVolumeInfo& vol, DepthStencilAspectMask aspect, VkImageSubresourceRange& range) const;

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

	VkImageView getOrCreateSingleSurfaceView(const TextureSurfaceInfo& surf, DepthStencilAspectMask aspect);

	VkImageView getOrCreateSingleLevelView(U level, DepthStencilAspectMask aspect);

	/// That view will be used in descriptor sets.
	VkImageView getOrCreateResourceGroupView(DepthStencilAspectMask aspect);

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

	VkImageAspectFlags convertAspect(DepthStencilAspectMask ak) const;

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

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(const TextureInitInfo& init);

	VkImageView getOrCreateView(const VkImageViewCreateInfo& ci);
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/TextureImpl.inl.h>
