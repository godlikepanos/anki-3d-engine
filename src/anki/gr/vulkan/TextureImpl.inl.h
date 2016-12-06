// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureImpl.h>

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
	checkSurface(surf);
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
	checkVolume(vol);
	range.aspectMask = convertAspect(aspect);
	range.baseMipLevel = vol.m_level;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
}

inline U TextureImpl::computeVkArrayLayer(const TextureSurfaceInfo& surf) const
{
	checkSurface(surf);
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

} // end namespace anki
