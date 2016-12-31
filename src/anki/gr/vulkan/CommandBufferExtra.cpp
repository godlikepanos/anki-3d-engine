// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferExtra.h>
#include <anki/core/Trace.h>

namespace anki
{

CommandBufferFactory::~CommandBufferFactory()
{
	for(U i = 0; i < 2; ++i)
	{
		for(U j = 0; j < 2; ++j)
		{
			CmdbType& type = m_types[i][j];

			if(type.m_count)
			{
				vkFreeCommandBuffers(m_dev, m_pool, type.m_count, &type.m_cmdbs[0]);
			}

			type.m_cmdbs.destroy(m_alloc);
		}
	}

	if(m_pool)
	{
		vkDestroyCommandPool(m_dev, m_pool, nullptr);
	}
}

Error CommandBufferFactory::init(GenericMemoryPoolAllocator<U8> alloc, VkDevice dev, uint32_t queueFamily)
{
	m_alloc = alloc;

	VkCommandPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	ci.queueFamilyIndex = queueFamily;

	ANKI_VK_CHECK(vkCreateCommandPool(dev, &ci, nullptr, &m_pool));

	m_dev = dev;
	return ErrorCode::NONE;
}

VkCommandBuffer CommandBufferFactory::newCommandBuffer(CommandBufferFlag cmdbFlags)
{
	ANKI_ASSERT(isCreated());

	Bool secondLevel = !!(cmdbFlags & CommandBufferFlag::SECOND_LEVEL);
	Bool smallBatch = !!(cmdbFlags & CommandBufferFlag::SMALL_BATCH);
	CmdbType& type = m_types[secondLevel][smallBatch];

	LockGuard<Mutex> lock(type.m_mtx);

	VkCommandBuffer out = VK_NULL_HANDLE;
	if(type.m_count > 0)
	{
		// Recycle

		--type.m_count;
		out = type.m_cmdbs[type.m_count];
	}
	else
	{
		// Create a new one

		VkCommandBufferAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ci.commandPool = m_pool;
		ci.level = (secondLevel) ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		ci.commandBufferCount = 1;

		ANKI_TRACE_INC_COUNTER(VK_CMD_BUFFER_CREATE, 1);
		ANKI_VK_CHECKF(vkAllocateCommandBuffers(m_dev, &ci, &out));
	}

	ANKI_ASSERT(out);
	return out;
}

void CommandBufferFactory::deleteCommandBuffer(VkCommandBuffer cmdb, CommandBufferFlag cmdbFlags)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(cmdb);

	Bool secondLevel = !!(cmdbFlags & CommandBufferFlag::SECOND_LEVEL);
	Bool smallBatch = !!(cmdbFlags & CommandBufferFlag::SMALL_BATCH);
	CmdbType& type = m_types[secondLevel][smallBatch];

	LockGuard<Mutex> lock(type.m_mtx);

	if(type.m_cmdbs.getSize() <= type.m_count)
	{
		// Grow storage
		type.m_cmdbs.resize(m_alloc, type.m_cmdbs.getSize() + 1);
	}

	type.m_cmdbs[type.m_count] = cmdb;
	++type.m_count;
}

} // end namespace anki
