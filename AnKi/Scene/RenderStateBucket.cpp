// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/RenderStateBucket.h>

namespace anki {

RenderStateBucketContainer::~RenderStateBucketContainer()
{
	for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
	{
		for([[maybe_unused]] ExtendedBucket& b : m_buckets[t])
		{
			ANKI_ASSERT(!b.m_program.isCreated() && b.m_userCount == 0);
		}
	}
}

RenderStateBucketIndex RenderStateBucketContainer::addUser(const RenderStateInfo& state, RenderingTechnique technique)
{
	// Compute state gash
	Array<U64, 3> toHash;
	toHash[0] = state.m_program->getUuid();
	toHash[1] = U64(state.m_primitiveTopology);
	toHash[2] = state.m_indexedDrawcall;
	const U64 hash = computeHash(toHash.getBegin(), toHash.getSizeInBytes());

	SceneDynamicArray<ExtendedBucket>& buckets = m_buckets[technique];

	RenderStateBucketIndex out;
	out.m_technique = technique;

	LockGuard lock(m_mtx);

	// Search bucket
	for(U32 i = 0; i < buckets.getSize(); ++i)
	{
		if(buckets[i].m_hash == hash)
		{
			++buckets[i].m_userCount;

			if(buckets[i].m_userCount == 1)
			{
				ANKI_ASSERT(!buckets[i].m_program.isCreated());
				buckets[i].m_program = state.m_program;
			}
			else
			{
				ANKI_ASSERT(buckets[i].m_program.isCreated());
			}

			out.m_index = i;
			return out;
		}
	}

	// Bucket not found, create one
	ExtendedBucket& newBucket = *buckets.emplaceBack();
	newBucket.m_hash = hash;
	newBucket.m_indexedDrawcall = state.m_indexedDrawcall;
	newBucket.m_primitiveTopology = state.m_primitiveTopology;
	newBucket.m_program = state.m_program;
	newBucket.m_userCount = 1;

	out.m_index = buckets.getSize() - 1;
	return out;
}

void RenderStateBucketContainer::removeUser(RenderStateBucketIndex& bucketIndex)
{
	if(!bucketIndex.isValid())
	{
		return;
	}

	{
		LockGuard lock(m_mtx);

		ANKI_ASSERT(bucketIndex.m_index < m_buckets[bucketIndex.m_technique].getSize());

		ExtendedBucket& bucket = m_buckets[bucketIndex.m_technique][bucketIndex.m_index];
		ANKI_ASSERT(bucket.m_userCount > 0 && bucket.m_program.isCreated());

		--bucket.m_userCount;

		if(bucket.m_userCount == 0)
		{
			// No more users, make sure you release any references
			bucket.m_program.reset(nullptr);
		}
	}

	bucketIndex.invalidate();
}

} // end namespace anki
