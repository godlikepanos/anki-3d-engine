// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DQueryFactory.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

namespace anki {

Error QueryFactory::newQuery(QueryHandle& handle)
{
	LockGuard lock(m_mtx);

	// Find a not-full chunk
	BlockArray::Iterator chunkIt = m_chunkArray.getEnd();
	for(auto it = m_chunkArray.getBegin(); it != m_chunkArray.getEnd(); ++it)
	{
		if(it->getAllocationCount() < kMaxQueriesPerQueryChunk)
		{
			// Found one

			if(chunkIt == m_chunkArray.getEnd())
			{
				chunkIt = it;
			}
			else if(it->getAllocationCount() > chunkIt->getAllocationCount())
			{
				// To decrease fragmentation use the most full chunk
				chunkIt = it;
			}
		}
	}

	if(chunkIt == m_chunkArray.getEnd())
	{
		// Create new chunk

		chunkIt = m_chunkArray.emplace();

		Chunk& chunk = *chunkIt;

		BufferInitInfo buffInit("QueryBuffer");
		buffInit.m_mapAccess = BufferMapAccessBit::kRead;
		buffInit.m_usage = BufferUsageBit::kCopyDestination;
		buffInit.m_size = kMaxQueriesPerQueryChunk * m_resultStructSize;
		chunk.m_resultsBuffer = GrManager::getSingleton().newBuffer(buffInit);

		chunk.m_resultsBufferCpuAddr = static_cast<U64*>(chunk.m_resultsBuffer->map(0, buffInit.m_size, BufferMapAccessBit::kRead));

		const D3D12_QUERY_HEAP_DESC desc = {.Type = m_type, .Count = kMaxQueriesPerQueryChunk, .NodeMask = 0};
		ANKI_D3D_CHECK(getDevice().CreateQueryHeap(&desc, IID_PPV_ARGS(&chunk.m_heap)));
	}

	ANKI_ASSERT(chunkIt != m_chunkArray.getEnd());

	// Allocate from chunk
	BitSet<kMaxQueriesPerQueryChunk, U64> freeBits = ~chunkIt->m_allocationMask;
	const U32 freeIndex = freeBits.getLeastSignificantBit();
	ANKI_ASSERT(freeIndex < kMaxU32);

	chunkIt->m_allocationMask.set(freeIndex);
	chunkIt->m_queryWritten.unset(freeIndex);

	handle.m_chunkIndex = U16(chunkIt.getArrayIndex());
	handle.m_queryIndex = U16(freeIndex);
#if ANKI_ASSERTIONS_ENABLED
	handle.m_type = m_type;
#endif

	return Error::kNone;
}

void QueryFactory::deleteQuery(QueryHandle& handle)
{
	ANKI_ASSERT(handle.isValid() && handle.m_type == m_type);

	LockGuard lock(m_mtx);

	BlockArray::Iterator chunkIt = m_chunkArray.indexToIterator(handle.m_chunkIndex);

	if(chunkIt->m_fenceArr[handle.m_queryIndex].isCreated())
	{
		ANKI_ASSERT(chunkIt->m_fenceArr[handle.m_queryIndex]->signaled() && "Trying to delete while the query is in flight");
		chunkIt->m_fenceArr[handle.m_queryIndex].reset(nullptr);
	}

	// Free
	ANKI_ASSERT(chunkIt->m_allocationMask.get(handle.m_queryIndex));
	chunkIt->m_allocationMask.unset(handle.m_queryIndex);

	// Free the chunk
	if(chunkIt->getAllocationCount() == 0)
	{
		safeRelease(chunkIt->m_heap);
		chunkIt->m_resultsBuffer->unmap();

		m_chunkArray.erase(chunkIt);
	}

	handle = {};
}

Bool QueryFactory::getResult(QueryHandle handle, U64& result)
{
	ANKI_ASSERT(handle.isValid() && handle.m_type == m_type);

	auto it = m_chunkArray.indexToIterator(handle.m_chunkIndex);
	ANKI_ASSERT(it->m_queryWritten.get(handle.m_queryIndex) && "Trying to get the result of a query that wasn't written");

	const Bool available = (it->m_fenceArr[handle.m_queryIndex].isCreated()) ? it->m_fenceArr[handle.m_queryIndex]->signaled() : true;
	if(available)
	{
		result = it->m_resultsBufferCpuAddr[(handle.m_queryIndex * m_resultStructSize + m_resultMemberOffset) / sizeof(U64)];
		it->m_fenceArr[handle.m_queryIndex].reset(nullptr);
	}
	else
	{
		result = 0;
	}

	return available;
}

void QueryFactory::postSubmitWork(QueryHandle handle, D3DMicroFence* fence)
{
	ANKI_ASSERT(handle.isValid() && handle.m_type == m_type && fence);

	auto it = m_chunkArray.indexToIterator(handle.m_chunkIndex);

	ANKI_ASSERT(!it->m_queryWritten.get(handle.m_queryIndex) && "A query can only be written once");
	ANKI_ASSERT(!it->m_fenceArr[handle.m_queryIndex].isCreated());

	it->m_fenceArr[handle.m_queryIndex].reset(fence);
	it->m_queryWritten.set(handle.m_queryIndex);
}

} // end namespace anki
