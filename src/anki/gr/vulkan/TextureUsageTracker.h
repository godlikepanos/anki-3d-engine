// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

class TextureUsageState
{
public:
	DynamicArray<VkImageLayout> m_subResources; ///< Volumes or surfaces.

	void destroy(StackAllocator<U8>& alloc)
	{
		m_subResources.destroy(alloc);
	}
};

class TextureUsageTracker
{
	friend class TextureImpl;

public:
	~TextureUsageTracker()
	{
		for(auto& it : m_map)
		{
			it.destroy(m_alloc);
		}

		m_map.destroy(m_alloc);
	}

	void init(StackAllocator<U8> alloc)
	{
		m_alloc = alloc;
	}

private:
	StackAllocator<U8> m_alloc;
	HashMap<U64, TextureUsageState> m_map;
};
/// @}

} // end namespace anki