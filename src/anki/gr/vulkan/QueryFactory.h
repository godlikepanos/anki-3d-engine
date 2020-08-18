// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/util/BitSet.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class QueryFactoryChunk;

/// @addtogroup vulkan
/// @{

const U MAX_SUB_ALLOCATIONS_PER_QUERY_CHUNK = 64;

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
		ANKI_ASSERT(m_queryIndex != MAX_U32);
		return m_queryIndex;
	}

	explicit operator Bool() const
	{
		return m_pool != VK_NULL_HANDLE;
	}

private:
	VkQueryPool m_pool = VK_NULL_HANDLE;
	U32 m_queryIndex = MAX_U32;
	QueryFactoryChunk* m_chunk = nullptr;
};

/// An allocation chunk.
class QueryFactoryChunk : public IntrusiveListEnabled<QueryFactoryChunk>
{
	friend class QueryFactory;

private:
	VkQueryPool m_pool = VK_NULL_HANDLE;
	BitSet<MAX_SUB_ALLOCATIONS_PER_QUERY_CHUNK> m_allocatedMask = {false};
	U32 m_subAllocationCount = 0;
};

/// Batch allocator of queries.
class QueryFactory : public NonCopyable
{
public:
	QueryFactory()
	{
	}

	~QueryFactory();

	void init(GrAllocator<U8> alloc, VkDevice dev, VkQueryType poolType)
	{
		m_alloc = alloc;
		m_dev = dev;
		m_poolType = poolType;
	}

	/// @note It's thread-safe.
	ANKI_USE_RESULT Error newQuery(MicroQuery& handle);

	/// @note It's thread-safe.
	void deleteQuery(MicroQuery& handle);

private:
	using Chunk = QueryFactoryChunk;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev;
	IntrusiveList<Chunk> m_chunks;
	Mutex m_mtx;
	VkQueryType m_poolType = VK_QUERY_TYPE_MAX_ENUM;
};
/// @}

} // end namespace anki
