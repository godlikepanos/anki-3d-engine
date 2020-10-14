// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>
#include <anki/gr/utils/Functions.h>
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
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TextureImplWorkaround)

/// A Vulkan image view with some extra data.
class MicroImageView
{
public:
	VkImageView m_handle = {};

	/// Index 0: Sampled image with SHADER_READ_ONLY layout.
	/// Index 1: Storage image with ofcource GENERAL layout.
	mutable Array<U32, 2> m_bindlessIndices = {MAX_U32, MAX_U32};

	/// Protect the m_bindlessIndices.
	mutable SpinLock m_lock;

	/// Because for example a single surface view of a cube texture will be a 2D view.
	TextureType m_derivedTextureType = TextureType::COUNT;

	MicroImageView()
	{
		for(U32 idx : m_bindlessIndices)
		{
			ANKI_ASSERT(idx == MAX_U32 && "Forgot to unbind the bindless");
			(void)idx;
		}
	}

	MicroImageView(const MicroImageView& b)
	{
		*this = std::move(b);
	}

	MicroImageView& operator=(const MicroImageView& b)
	{
		m_handle = b.m_handle;
		m_bindlessIndices = b.m_bindlessIndices;
		m_derivedTextureType = b.m_derivedTextureType;
		return *this;
	}
};

/// Texture container.
class TextureImpl final : public Texture, public VulkanObject<Texture, TextureImpl>
{
public:
	VkImage m_imageHandle = VK_NULL_HANDLE;

	GpuMemoryHandle m_memHandle;

	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;

	TextureImplWorkaround m_workarounds = TextureImplWorkaround::NONE;

	VkImageViewCreateInfo m_viewCreateInfoTemplate;

	TextureImpl(GrManager* manager, CString name)
		: Texture(manager, name)
	{
	}

	~TextureImpl();

	ANKI_USE_RESULT Error init(const TextureInitInfo& init)
	{
		return initInternal(VK_NULL_HANDLE, init);
	}

	ANKI_USE_RESULT Error initExternal(VkImage image, const TextureInitInfo& init)
	{
		return initInternal(image, init);
	}

	Bool aspectValid(DepthStencilAspectBit aspect) const
	{
		return m_aspect == aspect || !!(aspect & m_aspect);
	}

	/// Compute the layer as defined by Vulkan.
	U32 computeVkArrayLayer(const TextureSurfaceInfo& surf) const
	{
		U32 layer = 0;
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

	U32 computeVkArrayLayer(const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_texType == TextureType::_3D);
		return 0;
	}

	Bool usageValid(TextureUsageBit usage) const
	{
#if ANKI_ENABLE_ASSERTS
		LockGuard<SpinLock> lock(m_usedForMtx);
		m_usedFor |= usage;
#endif
		return (usage & m_usage) == usage;
	}

	/// By knowing the previous and new texture usage calculate the relavant info for a ppline barrier.
	void computeBarrierInfo(TextureUsageBit before, TextureUsageBit after, U32 level, VkPipelineStageFlags& srcStages,
							VkAccessFlags& srcAccesses, VkPipelineStageFlags& dstStages,
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

	void computeVkImageViewCreateInfo(const TextureSubresourceInfo& subresource, VkImageViewCreateInfo& viewCi,
									  TextureType& newTextureType) const
	{
		ANKI_ASSERT(isSubresourceValid(subresource));

		viewCi = m_viewCreateInfoTemplate;
		computeVkImageSubresourceRange(subresource, viewCi.subresourceRange);

		// Fixup the image view type
		newTextureType = computeNewTexTypeOfSubresource(subresource);
		viewCi.viewType = convertTextureViewType(newTextureType);
	}

	const MicroImageView& getOrCreateView(const TextureSubresourceInfo& subresource) const;

private:
	mutable HashMap<TextureSubresourceInfo, MicroImageView> m_viewsMap;
	mutable RWMutex m_viewsMapMtx;

	VkDeviceMemory m_dedicatedMem = VK_NULL_HANDLE;

#if ANKI_ENABLE_ASSERTS
	mutable TextureUsageBit m_usedFor = TextureUsageBit::NONE;
	mutable SpinLock m_usedForMtx;
#endif

	ANKI_USE_RESULT static VkFormatFeatureFlags calcFeatures(const TextureInitInfo& init);

	ANKI_USE_RESULT static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	ANKI_USE_RESULT Bool imageSupported(const TextureInitInfo& init);

	ANKI_USE_RESULT Error initImage(const TextureInitInfo& init);

	template<typename TextureInfo>
	void updateUsageState(const TextureInfo& surfOrVol, TextureUsageBit usage, StackAllocator<U8>& alloc,
						  TextureUsageState& state) const;

	void updateUsageState(TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const;

	/// Compute the new type of a texture view.
	TextureType computeNewTexTypeOfSubresource(const TextureSubresourceInfo& subresource) const;

	ANKI_USE_RESULT Error initInternal(VkImage externalImage, const TextureInitInfo& init);

	void computeBarrierInfo(TextureUsageBit usage, Bool src, U32 level, VkPipelineStageFlags& stages,
							VkAccessFlags& accesses) const;
};
/// @}

} // end namespace anki
