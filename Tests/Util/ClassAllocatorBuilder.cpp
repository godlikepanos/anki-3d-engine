// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/ClassAllocatorBuilder.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/BitSet.h>
#include <Tests/Framework/Framework.h>
#include <random>
#include <algorithm>

using namespace anki;

namespace {

class Chunk : public IntrusiveListEnabled<Chunk>
{
public:
	Chunk() = default;

	void* m_memory;

	BitSet<128> m_inUseSuballocations = {false};

	U32 m_suballocationCount;

	void* m_class;

	PtrSize m_size;
};

class Interface
{
public:
	class Class
	{
	public:
		Class(PtrSize suballocationSize, PtrSize cluster)
			: m_suballocationSize(suballocationSize)
			, m_chunkSize(cluster)
		{
		}

		PtrSize m_suballocationSize;
		PtrSize m_chunkSize;
	};

	Array<Class, 7> m_classes = {{Class(256, 16_KB), Class(4_KB, 256_KB), Class(128_KB, 8_MB), Class(1_MB, 32_MB),
								  Class(16_MB, 128_MB), Class(64_MB, 256_MB), Class(128_MB, 256_MB)}};
	static constexpr PtrSize MAX_SIZE = 128_MB;
	PtrSize m_crntSize = 0;

	Error allocateChunk(U32 classIdx, Chunk*& chunk)
	{
		PtrSize size = m_classes[classIdx].m_chunkSize;

		if(m_crntSize + size > MAX_SIZE)
		{
			return Error::kOutOfMemory;
		}

		PtrSize alignment = 256;

		chunk = new Chunk;
		chunk->m_memory = mallocAligned(size, alignment);
		chunk->m_size = size;

		m_crntSize += size;

		return Error::kNone;
	}

	void freeChunk(Chunk* chunk)
	{
		m_crntSize -= chunk->m_size;

		freeAligned(chunk->m_memory);
		delete chunk;
	}

	static constexpr U32 getClassCount()
	{
		return 7;
	}

	void getClassInfo(U32 classIdx, PtrSize& chunkSize, PtrSize& suballocationSize) const
	{
		suballocationSize = m_classes[classIdx].m_suballocationSize;
		chunkSize = m_classes[classIdx].m_chunkSize;
	}
};

} // namespace

static inline U32 floorPow2(U32 v)
{
	v |= v >> 16;
	v |= v >> 8;
	v |= v >> 4;
	v |= v >> 2;
	v |= v >> 1;
	v++;
	return v >> 1;
}

ANKI_TEST(Util, ClassAllocatorBuilder)
{
	HeapMemoryPool pool(allocAligned, nullptr);
	ClassAllocatorBuilder<Chunk, Interface, Mutex> calloc;
	calloc.init(&pool);

	std::mt19937 gen(0);

	const U SHIFT = 15;
	std::discrete_distribution<U> dis(16 * SHIFT, 0.0, F32(SHIFT), [](F32 c) {
		return exp2(-0.5 * c);
	});

	auto nextAllocSize = [&]() -> U {
		U size = U(256.0 * exp2(F64(dis(gen)) / 16.0));
		return size;
	};

	std::vector<std::pair<Chunk*, PtrSize>> allocations;
	const U TEST_COUNT = 100;
	const U ITERATIONS = 20;
	const U maxAlignment = 256;

	auto getRandAlignment = [&]() -> U {
		U out = rand() % maxAlignment;
		out = nextPowerOfTwo(out);
		out = max<U>(1, out);
		return out;
	};

	for(U tests = 0; tests < TEST_COUNT; ++tests)
	{
		for(U i = 0; i < ITERATIONS; ++i)
		{
			// Fill up the heap.
			while(1)
			{
				const PtrSize size = nextAllocSize();
				Chunk* chunk;
				PtrSize offset;
				const U alignment = getRandAlignment();

				if(calloc.allocate(size, alignment, chunk, offset))
				{
					break;
				}

				ANKI_TEST_EXPECT_EQ(isAligned(alignment, offset), true);
				allocations.push_back({chunk, offset});
			}

			std::shuffle(allocations.begin(), allocations.end(), gen);

			PtrSize halfSize = (allocations.size() * 3) / 4;
			for(PtrSize i = halfSize; i < allocations.size(); ++i)
			{
				calloc.free(allocations[i].first, allocations[i].second);
			}

			allocations.erase(allocations.begin() + halfSize, allocations.end());
		}

		// The heap should be roughly half-full now, so test fragmentation.
		const U32 freeSize = U32(Interface::MAX_SIZE - calloc.getInterface().m_crntSize);
		U32 baseFreeSize = floorPow2(freeSize);

		const U32 BASE_SIZE = 256;
		const F32 BIAS = 0.0;
		const F32 POWER = 1.2f;
		const F32 OFFSET = -1.0f;

		F32 bestCase = 0.0;
		{
			// Best case is when we can allocate once for every bit that is set in the pow2 structure.
			U32 freeBits = freeSize / BASE_SIZE;
			for(U32 bit = 0; bit < 32; bit++)
			{
				if(freeBits & (1u << bit))
				{
					bestCase += (pow(POWER, F32(BIAS + F32(bit))) + OFFSET) * F32(BASE_SIZE << bit);
				}
			}
		}

		F32 score = 0.0;

		while(baseFreeSize >= BASE_SIZE)
		{
			Chunk* chunk;
			PtrSize offset;
			const U alignment = getRandAlignment();
			while(calloc.allocate(baseFreeSize, alignment, chunk, offset) == Error::kNone)
			{
				ANKI_TEST_EXPECT_EQ(isAligned(alignment, offset), true);
				score += (pow(POWER, (log2(F32(baseFreeSize / BASE_SIZE)) + BIAS)) + OFFSET) * F32(baseFreeSize);
				allocations.push_back({chunk, offset});
			}

			baseFreeSize >>= 1;
		}

		printf("Score: %.3f\n", score / bestCase);

		// Cleanup
		for(auto& h : allocations)
		{
			calloc.free(h.first, h.second);
		}
		allocations.clear();
	}
}
