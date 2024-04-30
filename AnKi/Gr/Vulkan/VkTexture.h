// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Vulkan/VkGpuMemoryManager.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Texture container.
class TextureImpl final : public Texture
{
	friend class Texture;

public:
	VkImage m_imageHandle = VK_NULL_HANDLE;

	GpuMemoryHandle m_memHandle;

	VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;
	VkImageUsageFlags m_vkUsageFlags = 0;

	TextureImpl(CString name)
		: Texture(name)
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

	Bool usageValid(TextureUsageBit usage) const
	{
#if ANKI_ASSERTIONS_ENABLED
		LockGuard<SpinLock> lock(m_usedForMtx);
		m_usedFor |= usage;
#endif
		return (usage & m_usage) == usage;
	}

	/// By knowing the previous and new texture usage calculate the relavant info for a ppline barrier.
	void computeBarrierInfo(TextureUsageBit before, TextureUsageBit after, U32 level, VkPipelineStageFlags& srcStages, VkAccessFlags& srcAccesses,
							VkPipelineStageFlags& dstStages, VkAccessFlags& dstAccesses) const;

	/// Predict the image layout.
	VkImageLayout computeLayout(TextureUsageBit usage, U level) const;

	VkImageSubresourceRange computeVkImageSubresourceRange(const TextureSubresourceDescriptor& subresource) const
	{
		const TextureView in(this, subresource);
		VkImageSubresourceRange range = {};
		range.aspectMask = convertImageAspect(in.getDepthStencilAspect());
		range.baseMipLevel = in.getFirstMipmap();
		range.levelCount = in.getMipmapCount();

		const U32 faceCount = textureTypeIsCube(in.getTexture().getTextureType()) ? 6 : 1;
		range.baseArrayLayer = in.getFirstLayer() * faceCount + in.getFirstFace();
		range.layerCount = in.getLayerCount() * in.getFaceCount();
		return range;
	}

	VkImageView getImageView(const TextureSubresourceDescriptor& subresource) const
	{
		return getTextureViewEntry(subresource).m_handle;
	}

private:
	class TextureViewEntry
	{
	public:
		VkImageView m_handle = VK_NULL_HANDLE;

		mutable U32 m_bindlessIndex = kMaxU32;
		mutable SpinLock m_bindlessIndexLock;
	};

	enum class ViewClass
	{
		kDefault,
		kDepth,
		kStencil,

		kCount,
		kFirst = 0
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS_FRIEND(ViewClass)

	Array<GrDynamicArray<TextureViewEntry>, U32(ViewClass::kCount)> m_textureViews;
	Array<TextureViewEntry, U32(ViewClass::kCount)> m_wholeTextureViews;

#if ANKI_ASSERTIONS_ENABLED
	mutable TextureUsageBit m_usedFor = TextureUsageBit::kNone;
	mutable SpinLock m_usedForMtx;
#endif

	[[nodiscard]] static VkImageCreateFlags calcCreateFlags(const TextureInitInfo& init);

	[[nodiscard]] Bool imageSupported(const TextureInitInfo& init);

	Error initImage(const TextureInitInfo& init);

	Error initViews();

	Error initInternal(VkImage externalImage, const TextureInitInfo& init);

	void computeBarrierInfo(TextureUsageBit usage, Bool src, U32 level, VkPipelineStageFlags& stages, VkAccessFlags& accesses) const;

	U32 translateSurfaceOrVolume(U32 layer, U32 face, U32 mip) const
	{
		const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
		ANKI_ASSERT(layer < m_layerCount && face < faceCount && mip < m_mipCount);
		return layer * faceCount * m_mipCount + face * m_mipCount + mip;
	}

	const TextureViewEntry& getTextureViewEntry(const TextureSubresourceDescriptor& subresource) const;
};
/// @}

} // end namespace anki
