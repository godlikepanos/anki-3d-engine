// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/TextureUsageTracker.h>

namespace anki
{

inline VkImageAspectFlags TextureImpl::convertAspect(DepthStencilAspectBit ak) const
{
	VkImageAspectFlags out = 0;
	if(m_aspect == (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT))
	{
		out = !!(ak & DepthStencilAspectBit::DEPTH) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		out |= !!(ak & DepthStencilAspectBit::STENCIL) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
	}
	else
	{
		out = m_aspect;
	}

	ANKI_ASSERT(out != 0);
	ANKI_ASSERT((out & m_aspect) == out);
	return out;
}

inline void TextureImpl::computeSubResourceRange(
	const TextureSurfaceInfo& surf, DepthStencilAspectBit aspect, VkImageSubresourceRange& range) const
{
	checkSurfaceOrVolume(surf);
	range.aspectMask = convertAspect(aspect);
	range.baseMipLevel = surf.m_level;
	range.levelCount = 1;
	switch(m_type)
	{
	case TextureType::_2D:
		range.baseArrayLayer = 0;
		break;
	case TextureType::_3D:
		range.baseArrayLayer = 0;
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
		range.baseArrayLayer = 0;
	}
	range.layerCount = 1;
}

inline void TextureImpl::computeSubResourceRange(
	const TextureVolumeInfo& vol, DepthStencilAspectBit aspect, VkImageSubresourceRange& range) const
{
	checkSurfaceOrVolume(vol);
	range.aspectMask = convertAspect(aspect);
	range.baseMipLevel = vol.m_level;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
}

inline U TextureImpl::computeVkArrayLayer(const TextureSurfaceInfo& surf) const
{
	checkSurfaceOrVolume(surf);
	U layer = 0;
	switch(m_type)
	{
	case TextureType::_2D:
		layer = 0;
		break;
	case TextureType::_3D:
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

inline U TextureImpl::computeSubresourceIdx(const TextureSurfaceInfo& surf) const
{
	checkSurfaceOrVolume(surf);

	switch(m_type)
	{
	case TextureType::_1D:
	case TextureType::_2D:
		return surf.m_level;
		break;
	case TextureType::_2D_ARRAY:
		return surf.m_layer * m_mipCount + surf.m_level;
		break;
	case TextureType::CUBE:
		return surf.m_face * m_mipCount + surf.m_level;
		break;
	case TextureType::CUBE_ARRAY:
		return surf.m_layer * m_mipCount * 6 + surf.m_face * m_mipCount + surf.m_level;
		break;
	default:
		ANKI_ASSERT(0);
		return 0;
	}
}

template<typename TextureInfo>
inline VkImageLayout TextureImpl::findLayoutFromTracker(
	const TextureInfo& surfOrVol, const TextureUsageTracker& tracker) const
{
	checkSurfaceOrVolume(surfOrVol);

	auto it = tracker.m_map.find(m_uuid);
	if(it == tracker.m_map.getEnd())
	{
		ANKI_ASSERT(m_usageWhenEncountered != TextureUsageBit::NONE && "Cannot find the layout of the tex");
		return computeLayout(m_usageWhenEncountered, surfOrVol.m_level);
	}
	else
	{
		const U idx = computeSubresourceIdx(surfOrVol);
		const VkImageLayout out = it->m_subResources[idx];
		ANKI_ASSERT(out != VK_IMAGE_LAYOUT_UNDEFINED);
		return out;
	}
}

inline VkImageLayout TextureImpl::findLayoutFromTracker(const TextureUsageTracker& tracker) const
{
	auto it = tracker.m_map.find(m_uuid);
	if(it == tracker.m_map.getEnd())
	{
		ANKI_ASSERT(m_usageWhenEncountered != TextureUsageBit::NONE && "Cannot find the layout of the tex");
		return computeLayout(m_usageWhenEncountered, 0);
	}
	else
	{
		for(U i = 0; i < it->m_subResources.getSize(); ++i)
		{
			ANKI_ASSERT(
				it->m_subResources[i] == it->m_subResources[0] && "Not all image subresources are in the same layout");
		}
		const VkImageLayout out = it->m_subResources[0];
		ANKI_ASSERT(out != VK_IMAGE_LAYOUT_UNDEFINED);
		return out;
	}
}

template<typename TextureInfo>
inline void TextureImpl::updateTracker(
	const TextureInfo& surfOrVol, TextureUsageBit usage, TextureUsageTracker& tracker) const
{
	ANKI_ASSERT(usage != TextureUsageBit::NONE);
	ANKI_ASSERT(usageValid(usage));
	checkSurfaceOrVolume(surfOrVol);

	auto it = tracker.m_map.find(m_uuid);
	if(it != tracker.m_map.getEnd())
	{
		// Found
		updateUsageState(surfOrVol, usage, tracker.m_alloc, *it);
	}
	else
	{
		// Not found
		TextureUsageState state;
		updateUsageState(surfOrVol, usage, tracker.m_alloc, state);
		tracker.m_map.emplaceBack(tracker.m_alloc, m_uuid, std::move(state));
	}
}

inline void TextureImpl::updateTracker(TextureUsageBit usage, TextureUsageTracker& tracker) const
{
	ANKI_ASSERT(usage != TextureUsageBit::NONE);
	ANKI_ASSERT(usageValid(usage));

	auto it = tracker.m_map.find(m_uuid);
	if(it != tracker.m_map.getEnd())
	{
		// Found
		updateUsageState(usage, tracker.m_alloc, *it);
	}
	else
	{
		// Not found
		TextureUsageState state;
		updateUsageState(usage, tracker.m_alloc, state);
		tracker.m_map.emplaceBack(tracker.m_alloc, m_uuid, std::move(state));
	}
}

template<typename TextureInfo>
inline void TextureImpl::updateUsageState(
	const TextureInfo& surfOrVol, TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const
{
	checkSurfaceOrVolume(surfOrVol);
	if(ANKI_UNLIKELY(state.m_subResources.getSize() == 0))
	{
		state.m_subResources.create(alloc, m_surfaceOrVolumeCount);
		memorySet(
			reinterpret_cast<int*>(&state.m_subResources[0]), int(VK_IMAGE_LAYOUT_UNDEFINED), m_surfaceOrVolumeCount);
	}
	const VkImageLayout lay = computeLayout(usage, surfOrVol.m_level);
	ANKI_ASSERT(lay != VK_IMAGE_LAYOUT_UNDEFINED);
	state.m_subResources[computeSubresourceIdx(surfOrVol)] = lay;
}

inline void TextureImpl::updateUsageState(
	TextureUsageBit usage, StackAllocator<U8>& alloc, TextureUsageState& state) const
{
	if(ANKI_UNLIKELY(state.m_subResources.getSize() == 0))
	{
		state.m_subResources.create(alloc, m_surfaceOrVolumeCount);
	}
	const VkImageLayout lay = computeLayout(usage, 0);
	ANKI_ASSERT(lay != VK_IMAGE_LAYOUT_UNDEFINED);

	memorySet(reinterpret_cast<int*>(&state.m_subResources[0]), int(lay), m_surfaceOrVolumeCount);
}

} // end namespace anki
