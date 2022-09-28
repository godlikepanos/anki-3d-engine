// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/List.h>

namespace anki {

// Forward
class QueryFactoryChunk;

/// @addtogroup vulkan
/// @{

constexpr U kMaxSuballocationsPerQueryChunk = 64;

/// The return handle of a query allocation.
class MicroQuery
{
	friend class QueryFactory;

public:
	VkQueryPool getQueryPool() const
	{
		ANKI_ASSERT(m_pool != VK_NULL_HANDLE);
		return m_pool;
	}

	/// Get the index of the query inside the query pool.
	U32 getQueryIndex() const
	{
		ANKI_ASSERT(m_queryIndex != kMaxU32);
		return m_queryIndex;
	}

	explicit operator Bool() const
	{
		return m_pool != VK_NULL_HANDLE;
	}

private:
	VkQueryPool m_pool = VK_NULL_HANDLE;
	U32 m_queryIndex = kMaxU32;
	QueryFactoryChunk* m_chunk = nullptr;
};

/// An allocation chunk.
class QueryFactoryChunk : public IntrusiveListEnabled<QueryFactoryChunk>
{
	friend class QueryFactory;

private:
	VkQueryPool m_pool = VK_NULL_HANDLE;
	BitSet<kMaxSuballocationsPerQueryChunk> m_allocatedMask = {false};
	U32 m_subAllocationCount = 0;
};

/// Batch allocator of queries.
class QueryFactory
{
public:
	QueryFactory()
	{
	}

	QueryFactory(const QueryFactory&) = delete; // Non-copyable

	~QueryFactory();

	QueryFactory& operator=(const QueryFactory&) = delete; // Non-copyable

	void init(HeapMemoryPool* pool, VkDevice dev, VkQueryType poolType)
	{
		ANKI_ASSERT(pool);
		m_pool = pool;
		m_dev = dev;
		m_poolType = poolType;
	}

	/// @note It's thread-safe.
	Error newQuery(MicroQuery& handle);

	/// @note It's thread-safe.
	void deleteQuery(MicroQuery& handle);

private:
	using Chunk = QueryFactoryChunk;

	HeapMemoryPool* m_pool = nullptr;
	VkDevice m_dev;
	IntrusiveList<Chunk> m_chunks;
	Mutex m_mtx;
	VkQueryType m_poolType = VK_QUERY_TYPE_MAX_ENUM;
};
/// @}

} // end namespace anki
