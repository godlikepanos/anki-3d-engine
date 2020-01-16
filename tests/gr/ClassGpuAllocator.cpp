// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/utils/ClassGpuAllocator.h>
#include <tests/framework/Framework.h>
#include <random>
#include <algorithm>

namespace anki
{

class Mem : public ClassGpuAllocatorMemory
{
public:
	void* m_mem = nullptr;
	PtrSize m_size = 0;
};

class Interface final : public ClassGpuAllocatorInterface
{
public:
	class Class
	{
	public:
		Class(PtrSize slot, PtrSize cluster)
			: m_slotSize(slot)
			, m_clusterSize(cluster)
		{
		}

		PtrSize m_slotSize;
		PtrSize m_clusterSize;
	};

	std::vector<Class> m_classes;
	PtrSize m_maxSize = 128 * 1024 * 1024;
	PtrSize m_crntSize = 0;

	Interface()
	{
		m_classes.push_back(Class(256, 16 * 1024));
		m_classes.push_back(Class(4 * 1024, 256 * 1024));
		m_classes.push_back(Class(128 * 1024, 8 * 1024 * 1024));
		m_classes.push_back(Class(1 * 1024 * 1024, 32 * 1024 * 1024));
		m_classes.push_back(Class(16 * 1024 * 1024, 128 * 1024 * 1024));
		m_classes.push_back(Class(64 * 1024 * 1024, 256 * 1024 * 1024));
		m_classes.push_back(Class(128 * 1024 * 1024, 256 * 1024 * 1024));
	}

	ANKI_USE_RESULT Error allocate(U32 classIdx, ClassGpuAllocatorMemory*& mem)
	{
		PtrSize size = m_classes[classIdx].m_clusterSize;

		if(m_crntSize + size > m_maxSize)
		{
			return Error::OUT_OF_MEMORY;
		}

		PtrSize alignment = 256;

		Mem* m = new Mem();

		m_crntSize += size;

		m->m_mem = mallocAligned(size, alignment);
		m->m_size = size;
		mem = m;

		return Error::NONE;
	}

	void free(ClassGpuAllocatorMemory* mem)
	{
		Mem* m = static_cast<Mem*>(mem);
		m_crntSize -= m->m_size;

		freeAligned(m->m_mem);
		delete m;
	}

	U32 getClassCount() const
	{
		return U32(m_classes.size());
	}

	void getClassInfo(U32 classIdx, PtrSize& slotSize, PtrSize& chunkSize) const
	{
		slotSize = m_classes[classIdx].m_slotSize;
		chunkSize = m_classes[classIdx].m_clusterSize;
	}
};

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

ANKI_TEST(Gr, ClassGpuAllocator)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	Interface iface;

	ClassGpuAllocator calloc;
	calloc.init(alloc, &iface);

	std::mt19937 gen(0);

	const U SHIFT = 15;
	std::discrete_distribution<U> dis(16 * SHIFT, 0.0, F32(SHIFT), [](F32 c) { return exp2(-0.5 * c); });

	auto nextAllocSize = [&]() -> U {
		U size = U(256.0 * exp2(F64(dis(gen)) / 16.0));
		return size;
	};

	std::vector<ClassGpuAllocatorHandle> handles;
	const U TEST_COUNT = 100;
	const U ITERATIONS = 20;

	for(U tests = 0; tests < TEST_COUNT; ++tests)
	{
		for(U i = 0; i < ITERATIONS; ++i)
		{
			// Fill up the heap.
			while(1)
			{
				ClassGpuAllocatorHandle handle;
				PtrSize size = nextAllocSize();

				if(calloc.allocate(size, 1, handle))
				{
					break;
				}

				handles.push_back(handle);
			}

			std::shuffle(handles.begin(), handles.end(), gen);

			U halfSize = (handles.size() * 3) / 4;
			for(U i = halfSize; i < handles.size(); ++i)
			{
				calloc.free(handles[i]);
			}

			handles.erase(handles.begin() + halfSize, handles.end());
		}

		// The heap should be roughly half-full now, so test fragmentation.
		U32 freeSize = U32(iface.m_maxSize - iface.m_crntSize);
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
			ClassGpuAllocatorHandle handle;
			while(calloc.allocate(baseFreeSize, 1, handle) == Error::NONE)
			{
				score += (pow(POWER, (log2(F32(baseFreeSize / BASE_SIZE)) + BIAS)) + OFFSET) * F32(baseFreeSize);
				handles.push_back(handle);
				handle = {};
			}

			baseFreeSize >>= 1;
		}

		printf("Score: %.3f\n", score / bestCase);

		// Cleanup
		for(ClassGpuAllocatorHandle& h : handles)
		{
			calloc.free(h);
		}
		handles.clear();
	}
}

} // end namespace anki
