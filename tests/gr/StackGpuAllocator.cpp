// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/StackGpuAllocator.h>
#include <anki/util/ThreadHive.h>
#include <tests/framework/Framework.h>

using namespace anki;

namespace
{

const U ALLOCATION_COUNT = 1024;
const U THREAD_COUNT = 4;
const U32 MIN_ALLOCATION_SIZE = 1024;
const U32 MAX_ALLOCATION_SIZE = 1024 * 10;

class Mem : public StackGpuAllocatorMemory
{
public:
	void* m_mem = nullptr;
	PtrSize m_size = 0;
};

class Interface final : public StackGpuAllocatorInterface
{
public:
	U32 m_alignment = 256;

	ANKI_USE_RESULT Error allocate(PtrSize size, StackGpuAllocatorMemory*& mem)
	{
		Mem* m = new Mem();

		m->m_mem = mallocAligned(size, m_alignment);
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
		initialSize = m_alignment * 1024;
	}

	U32 getMaxAlignment()
	{
		return m_alignment;
	}
};

class TestContext
{
public:
	StackGpuAllocator* m_salloc;
	Array<StackGpuAllocatorHandle, ALLOCATION_COUNT> m_allocs;
	Array<U32, ALLOCATION_COUNT> m_allocSize;
	Atomic<U32> m_allocCount;
};

static void doAllocation(void* arg, U32 threadId, ThreadHive& hive)
{
	TestContext* ctx = static_cast<TestContext*>(arg);

	U allocCount = ctx->m_allocCount.fetchAdd(1);
	ctx->m_allocSize[allocCount] = randRange(MIN_ALLOCATION_SIZE, MAX_ALLOCATION_SIZE);
	ctx->m_salloc->allocate(ctx->m_allocSize[allocCount], ctx->m_allocs[allocCount]);
}

} // end anonymous namespace

ANKI_TEST(Gr, StackGpuAllocator)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	Interface iface;
	StackGpuAllocator salloc;
	salloc.init(alloc, &iface);

	TestContext ctx;
	memset(&ctx, 0, sizeof(ctx));
	ctx.m_salloc = &salloc;

	ThreadHiveTask task;
	task.m_callback = doAllocation;
	task.m_argument = &ctx;
	ThreadHive hive(THREAD_COUNT, alloc);

	for(U i = 0; i < ALLOCATION_COUNT; ++i)
	{
		hive.submitTasks(&task, 1);
	}

	hive.waitAllTasks();

	// Make sure memory doesn't overlap
	// TODO
}
