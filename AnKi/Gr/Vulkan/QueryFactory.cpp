// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/QueryFactory.h>

namespace anki {

QueryFactory::~QueryFactory()
{
	if(!m_chunks.isEmpty())
	{
		ANKI_VK_LOGW("Forgot the delete some queries");
	}
}

Error QueryFactory::newQuery(MicroQuery& handle)
{
	ANKI_ASSERT(!handle);

	LockGuard<Mutex> lock(m_mtx);

	// Find a not-full chunk
	Chunk* chunk = nullptr;
	for(Chunk& c : m_chunks)
	{
		if(c.m_subAllocationCount < kMaxSuballocationsPerQueryChunk)
		{
			// Found one

			if(chunk == nullptr)
			{
				chunk = &c;
			}
			else if(c.m_subAllocationCount > chunk->m_subAllocationCount)
			{
				// To decrease fragmentation use the most full chunk
				chunk = &c;
			}
		}
	}

	if(chunk == nullptr)
	{
		// Create new chunk
		chunk = newInstance<Chunk>(*m_pool);

		VkQueryPoolCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		ci.queryType = m_poolType;
		ci.queryCount = kMaxSuballocationsPerQueryChunk;

		ANKI_VK_CHECK(vkCreateQueryPool(m_dev, &ci, nullptr, &chunk->m_pool));
		m_chunks.pushBack(chunk);
	}

	ANKI_ASSERT(chunk);

	// Allocate from chunk
	for(U32 i = 0; i < kMaxSuballocationsPerQueryChunk; ++i)
	{
		if(chunk->m_allocatedMask.get(i) == 0)
		{
			chunk->m_allocatedMask.set(i);
			++chunk->m_subAllocationCount;

			handle.m_pool = chunk->m_pool;
			handle.m_queryIndex = i;
			handle.m_chunk = chunk;
			break;
		}
	}

	ANKI_ASSERT(!!handle);
	return Error::kNone;
}

void QueryFactory::deleteQuery(MicroQuery& handle)
{
	ANKI_ASSERT(handle.m_pool && handle.m_queryIndex != kMaxU32 && handle.m_chunk);

	LockGuard<Mutex> lock(m_mtx);

	Chunk* chunk = handle.m_chunk;
	ANKI_ASSERT(chunk->m_subAllocationCount > 0);
	--chunk->m_subAllocationCount;

	if(chunk->m_subAllocationCount == 0)
	{
		// Delete the chunk

		ANKI_ASSERT(chunk->m_allocatedMask.getAny());
		vkDestroyQueryPool(m_dev, chunk->m_pool, nullptr);

		m_chunks.erase(chunk);
		deleteInstance(*m_pool, chunk);
	}
	else
	{
		ANKI_ASSERT(chunk->m_allocatedMask.get(handle.m_queryIndex));
		chunk->m_allocatedMask.unset(handle.m_queryIndex);
	}

	handle = {};
}

} // end namespace anki
