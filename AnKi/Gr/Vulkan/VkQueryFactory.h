// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/List.h>

namespace anki {

// Forward
class QueryFactoryChunk;

/// @addtogroup vulkan
/// @{

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
	BitSet<kMaxQueriesPerQueryChunk> m_allocatedMask = {false};
	U32 m_subAllocationCount = 0;
};

/// Batch allocator of queries.
class QueryFactory
{
public:
	QueryFactory(VkQueryType poolType, VkQueryPipelineStatisticFlags pplineStatisticsFlags = 0)
	{
		m_poolType = poolType;
		m_pplineStatisticsFlags = pplineStatisticsFlags;
	}

	QueryFactory(const QueryFactory&) = delete; // Non-copyable

	~QueryFactory();

	QueryFactory& operator=(const QueryFactory&) = delete; // Non-copyable

	/// @note It's thread-safe.
	Error newQuery(MicroQuery& handle);

	/// @note It's thread-safe.
	void deleteQuery(MicroQuery& handle);

private:
	using Chunk = QueryFactoryChunk;

	IntrusiveList<Chunk> m_chunks;
	Mutex m_mtx;
	VkQueryType m_poolType = VK_QUERY_TYPE_MAX_ENUM;
	VkQueryPipelineStatisticFlags m_pplineStatisticsFlags = 0;
};

class OcclusionQueryFactory : public QueryFactory, public MakeSingleton<OcclusionQueryFactory>
{
public:
	OcclusionQueryFactory()
		: QueryFactory(VK_QUERY_TYPE_OCCLUSION)
	{
	}
};

class TimestampQueryFactory : public QueryFactory, public MakeSingleton<TimestampQueryFactory>
{
public:
	TimestampQueryFactory()
		: QueryFactory(VK_QUERY_TYPE_TIMESTAMP)
	{
	}
};

class PrimitivesPassedClippingFactory : public QueryFactory, public MakeSingleton<PrimitivesPassedClippingFactory>
{
public:
	PrimitivesPassedClippingFactory()
		: QueryFactory(VK_QUERY_TYPE_PIPELINE_STATISTICS, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT)
	{
	}
};
/// @}

} // end namespace anki
