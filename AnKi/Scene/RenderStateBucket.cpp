// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
			ANKI_ASSERT(!b.m_program.isCreated() && b.m_userCount == 0 && b.m_lod0MeshletCount == 0);
		}

		ANKI_ASSERT(m_bucketActiveUserCount[t] == 0);
		ANKI_ASSERT(m_activeBucketCount[t] == 0);
	}
}

RenderStateBucketIndex RenderStateBucketContainer::addUser(const RenderStateInfo& state, RenderingTechnique technique, U32 lod0MeshletCount)
{
	// Compute state gash
	Array<U32, 2> toHash;
	toHash[0] = state.m_program->getUuid();
	toHash[1] = U32(state.m_primitiveTopology);
	const U64 hash = computeHash(toHash.getBegin(), toHash.getSizeInBytes());

	SceneDynamicArray<ExtendedBucket>& buckets = m_buckets[technique];

	RenderStateBucketIndex out;
	out.m_technique = technique;

	LockGuard lock(m_mtx);

	++m_bucketActiveUserCount[technique];
	m_bucketActiveUserCountWithMeshlets[technique] += (lod0MeshletCount) ? 1 : 0;
	m_lod0MeshletCount[technique] += lod0MeshletCount;

	// Search bucket
	for(U32 i = 0; i < buckets.getSize(); ++i)
	{
		if(buckets[i].m_hash == hash)
		{
			// Bucket found

			if(buckets[i].m_userCount > 0)
			{
				if(lod0MeshletCount)
				{
					ANKI_ASSERT(buckets[i].m_lod0MeshletCount > 0 && "A bucket either does meshlet rendering or not");
				}
				else
				{
					ANKI_ASSERT(buckets[i].m_lod0MeshletCount == 0 && "A bucket either does meshlet rendering or not");
				}
			}

			++buckets[i].m_userCount;
			buckets[i].m_lod0MeshletCount += lod0MeshletCount;

			if(buckets[i].m_userCount == 1)
			{
				ANKI_ASSERT(!buckets[i].m_program.isCreated());
				ANKI_ASSERT(buckets[i].m_lod0MeshletCount == lod0MeshletCount);
				buckets[i].m_program = state.m_program;
				++m_activeBucketCount[technique];

				createPerfOrder(technique);
			}
			else
			{
				ANKI_ASSERT(buckets[i].m_program.isCreated());
			}

			out.m_index = i;
			out.m_lod0MeshletCount = lod0MeshletCount;
			return out;
		}
	}

	// Bucket not found, create one
	ExtendedBucket& newBucket = *buckets.emplaceBack();
	newBucket.m_hash = hash;
	newBucket.m_primitiveTopology = state.m_primitiveTopology;
	newBucket.m_program = state.m_program;
	newBucket.m_userCount = 1;
	newBucket.m_lod0MeshletCount = lod0MeshletCount;

	++m_activeBucketCount[technique];

	createPerfOrder(technique);

	out.m_index = buckets.getSize() - 1;
	out.m_lod0MeshletCount = lod0MeshletCount;
	return out;
}

void RenderStateBucketContainer::removeUser(RenderStateBucketIndex& bucketIndex)
{
	if(!bucketIndex.isValid())
	{
		return;
	}

	const RenderingTechnique technique = bucketIndex.m_technique;
	const U32 idx = bucketIndex.m_index;
	const U32 meshletCount = bucketIndex.m_lod0MeshletCount;
	bucketIndex.invalidate();

	LockGuard lock(m_mtx);

	ANKI_ASSERT(idx < m_buckets[technique].getSize());

	ANKI_ASSERT(m_bucketActiveUserCount[technique] > 0);
	--m_bucketActiveUserCount[technique];

	if(meshletCount)
	{
		ANKI_ASSERT(m_bucketActiveUserCountWithMeshlets[technique] >= 1);
		--m_bucketActiveUserCountWithMeshlets[technique];
	}

	ANKI_ASSERT(m_lod0MeshletCount[technique] >= meshletCount);
	m_lod0MeshletCount[technique] -= meshletCount;

	ExtendedBucket& bucket = m_buckets[technique][idx];
	ANKI_ASSERT(bucket.m_userCount > 0 && bucket.m_program.isCreated() && bucket.m_lod0MeshletCount >= meshletCount);

	--bucket.m_userCount;
	bucket.m_lod0MeshletCount -= meshletCount;

	if(bucket.m_userCount == 0)
	{
		// No more users, make sure you release any references
		bucket.m_program.reset(nullptr);

		ANKI_ASSERT(m_activeBucketCount[technique] > 0);
		--m_activeBucketCount[technique];

		createPerfOrder(technique);
	}
}

void RenderStateBucketContainer::createPerfOrder(RenderingTechnique t)
{
	const U32 bucketCount = m_buckets[t].getSize();

	m_bucketPerfOrder[t].resize(bucketCount);
	for(U32 i = 0; i < bucketCount; ++i)
	{
		m_bucketPerfOrder[t][i] = i;
	}

	std::sort(m_bucketPerfOrder[t].getBegin(), m_bucketPerfOrder[t].getBegin() + bucketCount, [&, this](U32 a, U32 b) {
		auto getProgramHeaviness = [](const ShaderProgram& p) {
			U64 size = U64(p.getShaderBinarySize(ShaderType::kPixel)) << 32u; // Fragment is more important
			if(!!(p.getShaderTypes() & ShaderTypeBit::kVertex))
			{
				size |= p.getShaderBinarySize(ShaderType::kVertex);
			}
			else
			{
				ANKI_ASSERT(!!(p.getShaderTypes() & ShaderTypeBit::kMesh));
				size |= p.getShaderBinarySize(ShaderType::kMesh);
			}
			return size;
		};

		const Bool aIsActive = m_buckets[t][a].m_program.isCreated();
		const Bool bIsActive = m_buckets[t][b].m_program.isCreated();
		const Bool aHasDiscard = (aIsActive) ? m_buckets[t][a].m_program->hasDiscard() : false;
		const Bool bHasDiscard = (bIsActive) ? m_buckets[t][b].m_program->hasDiscard() : false;
		const U64 aProgramHeaviness = (aIsActive) ? getProgramHeaviness(*m_buckets[t][a].m_program) : 0;
		const U64 bProgramHeaviness = (bIsActive) ? getProgramHeaviness(*m_buckets[t][b].m_program) : 0;

		if(aHasDiscard != bHasDiscard)
		{
			return !aHasDiscard;
		}
		else
		{
			return aProgramHeaviness < bProgramHeaviness;
		}
	});
}

} // end namespace anki
