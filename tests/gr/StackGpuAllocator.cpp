// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/StackGpuAllocator.h>
#include <anki/util/ThreadHive.h>
#include <tests/framework/Framework.h>
#include <algorithm>

using namespace anki;

namespace
{

const U ALLOCATION_COUNT = 1024;
const U THREAD_COUNT = 4;
const U32 MIN_ALLOCATION_SIZE = 256;
const U32 MAX_ALLOCATION_SIZE = 1024 * 10;
const U32 ALIGNMENT = 256;

class Mem : public StackGpuAllocatorMemory
{
public:
	void* m_mem = nullptr;
	PtrSize m_size = 0;
};

class Interface final : public StackGpuAllocatorInterface
{
public:
	ANKI_USE_RESULT Error allocate(PtrSize size, StackGpuAllocatorMemory*& mem)
	{
		Mem* m = new Mem();

		m->m_mem = mallocAligned(size, ALIGNMENT);
		m->m_size = size;
		mem = m;

		return ErrorCode::NONE;
	}

	void free(StackGpuAllocatorMemory* mem)
	{
		Mem* m = static_cast<Mem*>(mem);
		freeAligned(m->m_mem);
		delete m;
	}

	void getChunkGrowInfo(F32& scale, PtrSize& bias, PtrSize& initialSize)
	{
		scale = 2.0;
		bias = 0;
		initialSize = ALIGNMENT * 1024;
	}

	U32 getMaxAlignment()
	{
		return ALIGNMENT;
	}
};

class Allocation
{
public:
	StackGpuAllocatorHandle m_handle;
	PtrSize m_size;
};

class TestContext
{
public:
	StackGpuAllocator* m_salloc;
	Array<Allocation, ALLOCATION_COUNT> m_allocs;
	Atomic<U32> m_allocCount;
};

static void doAllocation(void* arg, U32 threadId, ThreadHive& hive)
{
	TestContext* ctx = static_cast<TestContext*>(arg);

	U allocCount = ctx->m_allocCount.fetchAdd(1);
	PtrSize allocSize = randRange(MIN_ALLOCATION_SIZE, MAX_ALLOCATION_SIZE);
	ctx->m_allocs[allocCount].m_size = allocSize;
	ANKI_TEST_EXPECT_NO_ERR(ctx->m_salloc->allocate(allocSize, ctx->m_allocs[allocCount].m_handle));
}

} // end anonymous namespace

ANKI_TEST(Gr, StackGpuAllocator)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	Interface iface;
	StackGpuAllocator salloc;
	salloc.init(alloc, &iface);

	ThreadHive hive(THREAD_COUNT, alloc);

	for(U i = 0; i < 1024; ++i)
	{
		TestContext ctx;
		memset(&ctx, 0, sizeof(ctx));
		ctx.m_salloc = &salloc;

		ThreadHiveTask task;
		task.m_callback = doAllocation;
		task.m_argument = &ctx;

		for(U i = 0; i < ALLOCATION_COUNT; ++i)
		{
			hive.submitTasks(&task, 1);
		}

		hive.waitAllTasks();

		// Make sure memory doesn't overlap
		std::sort(ctx.m_allocs.getBegin(), ctx.m_allocs.getEnd(), [](const Allocation& a, const Allocation& b) {
			if(a.m_handle.m_memory != b.m_handle.m_memory)
			{
				return a.m_handle.m_memory < b.m_handle.m_memory;
			}

			if(a.m_handle.m_offset != b.m_handle.m_offset)
			{
				return a.m_handle.m_offset <= b.m_handle.m_offset;
			}

			ANKI_TEST_EXPECT_EQ(1, 0);
			return true;
		});

		for(U i = 1; i < ALLOCATION_COUNT; ++i)
		{
			const Allocation& a = ctx.m_allocs[i - 1];
			const Allocation& b = ctx.m_allocs[i];

			if(a.m_handle.m_memory == b.m_handle.m_memory)
			{
				ANKI_TEST_EXPECT_LEQ(a.m_handle.m_offset + a.m_size, b.m_handle.m_offset);
			}
		}

		salloc.reset();
	}
}
