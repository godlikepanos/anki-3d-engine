// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/GpuMemoryManager.h>
#include <AnKi/Gr/Utils/Functions.h>
#include <AnKi/Gr/Vulkan/SamplerFactory.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

// Forward
class TextureUsageState;

/// @addtogroup vulkan
/// @{

/// A Vulkan image view with some extra data.
class MicroImageView
{
	friend class TextureImpl;

public:
	MicroImageView() = default;

	MicroImageView(MicroImageView&& b)
	{
		*this = std::move(b);
	}

	~MicroImageView()
	{
		ANKI_ASSERT(m_bindlessIndex == kMaxU32 && "Forgot to unbind the bindless");
		ANKI_ASSERT(m_handle == VK_NULL_HANDLE);
	}

	MicroImageView& operator=(MicroImageView&& b)
	{
		m_handle = b.m_handle;
		b.m_handle = VK_NULL_HANDLE;
		m_bindlessIndex = b.m_bindlessIndex;
		b.m_bindlessIndex = kMaxU32;
		m_derivedTextureType = b.m_derivedTextureType;
		b.m_derivedTextureType = TextureType::kCount;
		return *this;
	}

	VkImageView getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	/// @note It's thread-safe.
	U32 getOrCreateBindlessIndex(GrManagerImpl& gr) const;

	TextureType getDerivedTextureType() const
	{
		ANKI_ASSERT(m_derivedTextureType != TextureType::kCount);
		return m_derivedTextureType;
	}

private:
	VkImageView m_handle = VK_NULL_HANDLE;

	mutable U32 m_bindlessIndex = kMaxU32;
	mutable SpinLock m_bindlessIndexLock;

	/// Because for example a single surface view of a cube texture will be a 2D view.
	TextureType m_derivedTextureType = TextureType::kCount;
};

/// Texture container.
class TextureImpl final : public Texture, public VulkanObject<Texture, TextureImpl>
{
public:
	VkImage m_imageHandle = VK_NULL_HANDLE;

	GpuMemoryHandle m_memHandle;

	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;
	VkImageUsageFlags m_vkUsageFlags = 0;
	VkImageViewCreateInfo m_viewCreateInfoTemplate;
	VkImageViewASTCDecodeModeEXT m_astcDecodeMode;

	TextureImpl(GrManager* manager, CString name)
		: Texture(manager, name)
	{
	}

	~TextureImpl();

	Error init(const TextureInitInfo& init)
	{
		return initInternal(VK_NULL_HANDLE, init);
	}

	Error initExternal(VkImage image, const TextureInitInfo& init)
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
		case TextureType::k2D:
			layer = 0;
			break;
		case TextureType::kCube:
			layer = surf.m_face;
			break;
		case TextureType::k2DArray:
			layer = surf.m_layer;
			break;
		case TextureType::kCubeArray:
			layer = surf.m_layer * 6 + surf.m_face;
			break;
		default:
			ANKI_ASSERT(0);
		}

		return layer;
	}

	U32 computeVkArrayLayer([[maybe_unused]] const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_texType == TextureType::k3D);
		return 0;
	}

	Bool usageValid(TextureUsageBit usage) const
	{
#if ANKI_ENABLE_ASSERTIONS
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

	/// This is a special optimization for textures that have only one surface. In this case we don't need to go through
	/// the hashmap above.
	MicroImageView m_singleSurfaceImageView;

#if ANKI_ENABLE_ASSERTIONS
	mutable TextureUsageBit m_usedFor = TextureUsageBit::kNone;
	mutable SpinLock m_usedForMtx;
#endif

	[[nodiscard]] static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	[[nodiscard]] Bool imageSupported(const TextureInitInfo& init);

	Error initImage(const TextureInitInfo& init);

	/// Compute the new type of a texture view.
	TextureType computeNewTexTypeOfSubresource(const TextureSubresourceInfo& subresource) const;

	Error initInternal(VkImage externalImage, const TextureInitInfo& init);

	void computeBarrierInfo(TextureUsageBit usage, Bool src, U32 level, VkPipelineStageFlags& stages,
							VkAccessFlags& accesses) const;
};
/// @}

} // end namespace anki
