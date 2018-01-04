// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureImpl.h>

namespace anki
{

inline U TextureImpl::computeVkArrayLayer(const TextureSurfaceInfo& surf) const
{
	checkSurfaceOrVolume(surf);
	U layer = 0;
	switch(m_texType)
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

	switch(m_texType)
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

} // end namespace anki
