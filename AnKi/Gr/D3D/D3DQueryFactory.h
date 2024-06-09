// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DBuffer.h>
#include <AnKi/Gr/D3D/D3DFence.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

/// @addtogroup d3d
/// @{

/// @memberof QueryFactory
class QueryHandle
{
	friend class QueryFactory;

private:
	U16 m_chunkIndex = kMaxU16;
	U16 m_queryIndex = kMaxU16;
#if ANKI_ASSERTIONS_ENABLED
	D3D12_QUERY_HEAP_TYPE m_type = {};
#endif

	Bool isValid() const
	{
		return m_chunkIndex < kMaxU16 && m_queryIndex < kMaxU16;
	}
};

/// @memberof QueryFactory
class QueryInfo
{
public:
	ID3D12Resource* m_resultsBuffer;
	U32 m_resultsBufferOffset;

	ID3D12QueryHeap* m_queryHeap;
	U32 m_indexInHeap;
};

/// Query allocator.
class QueryFactory
{
public:
	QueryFactory(D3D12_QUERY_HEAP_TYPE type, U32 resultStructSize, U32 resultMemberOffset)
		: m_type(type)
		, m_resultStructSize64(resultStructSize / sizeof(U64))
		, m_resultMemberOffset64(resultMemberOffset / sizeof(U64))
	{
		ANKI_ASSERT(resultStructSize > 0);
		ANKI_ASSERT((resultStructSize % sizeof(U64)) == 0);
		ANKI_ASSERT((resultMemberOffset % sizeof(U64)) == 0);
		ANKI_ASSERT(resultMemberOffset + sizeof(U64) <= resultStructSize);
	}

	~QueryFactory()
	{
		ANKI_ASSERT(m_chunkArray.isEmpty() && "Forgot to free some queries");
	}

	/// @note It's thread-safe.
	Error newQuery(QueryHandle& handle);

	/// @note It's thread-safe.
	void deleteQuery(QueryHandle& handle);

	Bool getResult(QueryHandle handle, U64& result);

	QueryInfo getQueryInfo(QueryHandle handle) const
	{
		QueryInfo info;
		ANKI_ASSERT(handle.isValid() && handle.m_type == m_type);
		auto it = m_chunkArray.indexToIterator(handle.m_chunkIndex);

		info.m_resultsBuffer = &static_cast<const BufferImpl&>(*it->m_resultsBuffer).getD3DResource();
		info.m_resultsBufferOffset = (handle.m_queryIndex * m_resultStructSize64 + m_resultMemberOffset64) / sizeof(U64);
		info.m_queryHeap = it->m_heap;
		info.m_indexInHeap = handle.m_queryIndex;
		return info;
	}

	/// Call this on submit if the query was written.
	void postSubmitWork(QueryHandle handle, MicroFence* fence);

private:
	class Chunk
	{
	public:
		ID3D12QueryHeap* m_heap = nullptr;

		BitSet<kMaxQueriesPerQueryChunk, U64> m_allocationMask = {false};

		BitSet<kMaxQueriesPerQueryChunk, U64> m_queryWritten = {false};

		Array<MicroFencePtr, kMaxQueriesPerQueryChunk> m_fenceArr;

		BufferPtr m_resultsBuffer;
		const U64* m_resultsBufferCpuAddr = nullptr;

		Chunk() = default;

		Chunk(const Chunk&) = delete;

		Chunk& operator=(const Chunk&) = delete;

		U32 getAllocationCount() const
		{
			return m_allocationMask.getSetBitCount();
		}
	};

	template<typename Chunk>
	class BlockArrayConfig
	{
	public:
		static constexpr U32 getElementCountPerBlock()
		{
			return 8;
		}
	};

	using BlockArray = BlockArray<Chunk, SingletonMemoryPoolWrapper<GrMemoryPool>, BlockArrayConfig<Chunk>>;

	BlockArray m_chunkArray;
	Mutex m_mtx;

	D3D12_QUERY_HEAP_TYPE m_type;
	U32 m_resultStructSize64;
	U32 m_resultMemberOffset64;
};

class OcclusionQueryFactory : public QueryFactory, public MakeSingleton<OcclusionQueryFactory>
{
public:
	OcclusionQueryFactory()
		: QueryFactory(D3D12_QUERY_HEAP_TYPE_OCCLUSION, sizeof(U64), 0)
	{
	}
};

class TimestampQueryFactory : public QueryFactory, public MakeSingleton<TimestampQueryFactory>
{
public:
	TimestampQueryFactory()
		: QueryFactory(D3D12_QUERY_HEAP_TYPE_TIMESTAMP, sizeof(U64), 0)
	{
	}
};

class PrimitivesPassedClippingFactory : public QueryFactory, public MakeSingleton<PrimitivesPassedClippingFactory>
{
public:
	PrimitivesPassedClippingFactory()
		: QueryFactory(D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS, sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS),
					   offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, CInvocations))
	{
	}
};
/// @}

} // end namespace anki
