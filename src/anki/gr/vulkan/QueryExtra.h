// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class QueryAllocatorChunk;

/// @addtogroup vulkan
/// @{

const U MAX_SUB_ALLOCATIONS_PER_QUERY_CHUNK = 64;

/// The return handle of a query allocation.
class QueryAllocationHandle
{
	friend class QueryAllocator;

public:
	VkQueryPool m_pool = VK_NULL_HANDLE;
	U32 m_queryIndex = MAX_U32;

	operator Bool() const
	{
		return m_pool != VK_NULL_HANDLE;
	}

private:
	QueryAllocatorChunk* m_chunk = nullptr;
};

/// An allocation chunk.
class QueryAllocatorChunk : public IntrusiveListEnabled<QueryAllocatorChunk>
{
	friend class QueryAllocator;

private:
	VkQueryPool m_pool = VK_NULL_HANDLE;
	BitSet<MAX_SUB_ALLOCATIONS_PER_QUERY_CHUNK> m_allocatedMask = {false};
	U32 m_subAllocationCount = 0;
};

/// Batch allocator of queries.
class QueryAllocator : public NonCopyable
{
public:
	QueryAllocator()
	{
	}

	~QueryAllocator();

	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		m_alloc = alloc;
		m_dev = dev;
	}

	ANKI_USE_RESULT Error newQuery(QueryAllocationHandle& handle);

	void deleteQuery(QueryAllocationHandle& handle);

private:
	using Chunk = QueryAllocatorChunk;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev;
	IntrusiveList<Chunk> m_chunks;
	Mutex m_mtx;
};
/// @}

} // end namespace anki
