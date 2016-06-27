// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Command bufffer object recycler.
class CommandBufferFactory
{
public:
	CommandBufferFactory()
	{
	}

	~CommandBufferFactory();

	ANKI_USE_RESULT Error init(GenericMemoryPoolAllocator<U8> alloc,
		VkDevice dev,
		uint32_t queueFamily);

	/// Request a new command buffer.
	VkCommandBuffer newCommandBuffer(Bool secondLevel);

	/// Free a command buffer.
	void deleteCommandBuffer(VkCommandBuffer cmdb, Bool secondLevel);

	void collect();

	Bool isCreated() const
	{
		return m_dev != VK_NULL_HANDLE;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	VkCommandPool m_pool = VK_NULL_HANDLE;

	class CmdbType
	{
	public:
		DynamicArray<VkCommandBuffer> m_cmdbs; ///< A stack.
		U32 m_count = 0;
		Mutex m_mtx; ///< Lock because the allocations may happen anywhere.
	};

	Array<CmdbType, 2> m_types;
};
/// @}

} // end namespace anki
