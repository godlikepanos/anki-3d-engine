// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Gr.h>

using namespace anki;

static void clearSwapchain(CommandBufferPtr cmdb = CommandBufferPtr())
{
	const Bool continueCmdb = cmdb.isCreated();

	TexturePtr presentTex = GrManager::getSingleton().acquireNextPresentableTexture();

	if(!continueCmdb)
	{
		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		cmdb = GrManager::getSingleton().newCommandBuffer(cinit);
	}

	const TextureBarrierInfo barrier = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
										TextureUsageBit::kFramebufferWrite};
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

	RenderTarget rt;
	rt.m_textureView = TextureView(presentTex.get(), TextureSubresourceDesc::all());
	rt.m_clearValue.m_colorf = {1.0f, F32(rand()) / F32(RAND_MAX), 1.0f, 1.0f};
	cmdb->beginRenderPass({rt});
	cmdb->endRenderPass();

	const TextureBarrierInfo barrier2 = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kFramebufferWrite,
										 TextureUsageBit::kPresent};
	cmdb->setPipelineBarrier({&barrier2, 1}, {}, {});

	if(!continueCmdb)
	{
		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());
	}
}

ANKI_TEST(Gr, WorkGraphHelloWorld)
{
	// CVarSet::getSingleton().setMultiple(Array<const Char*, 2>{"Device", "1"});
	commonInit();

	{
		const Char* kSrc = R"(
struct FirstNodeRecord
{
	uint3 m_gridSize : SV_DispatchGrid;
	uint m_value;
};

struct SecondNodeRecord
{
	uint3 m_gridSize : SV_DispatchGrid;
	uint m_value;
};

struct ThirdNodeRecord
{
	uint m_value;
};

RWStructuredBuffer<uint> g_buff : register(u0);

[Shader("node")] [NodeLaunch("broadcasting")] [NodeIsProgramEntry] [NodeMaxDispatchGrid(1, 1, 1)] [NumThreads(16, 1, 1)]
void main(DispatchNodeInputRecord<FirstNodeRecord> inp, [MaxRecords(2)] NodeOutput<SecondNodeRecord> secondNode, uint svGroupIndex : SV_GroupIndex)
{
	GroupNodeOutputRecords<SecondNodeRecord> rec = secondNode.GetGroupNodeOutputRecords(2);

	if(svGroupIndex < 2)
	{
		rec[svGroupIndex].m_gridSize = uint3(16, 1, 1);
		rec[svGroupIndex].m_value = inp.Get().m_value;
	}

	rec.OutputComplete();
}

[Shader("node")] [NodeLaunch("broadcasting")] [NumThreads(16, 1, 1)] [NodeMaxDispatchGrid(16, 1, 1)]
void secondNode(DispatchNodeInputRecord<SecondNodeRecord> inp, [MaxRecords(32)] NodeOutput<ThirdNodeRecord> thirdNode,
				uint svGroupIndex : SV_GROUPINDEX)
{
	GroupNodeOutputRecords<ThirdNodeRecord> recs = thirdNode.GetGroupNodeOutputRecords(32);

	recs[svGroupIndex * 2 + 0].m_value = inp.Get().m_value;
	recs[svGroupIndex * 2 + 1].m_value = inp.Get().m_value;

	recs.OutputComplete();
}

[Shader("node")] [NodeLaunch("coalescing")] [NumThreads(16, 1, 1)]
void thirdNode([MaxRecords(32)] GroupNodeInputRecords<ThirdNodeRecord> inp, uint svGroupIndex : SV_GroupIndex)
{
	if (svGroupIndex * 2 < inp.Count())
		InterlockedAdd(g_buff[0], inp[svGroupIndex * 2].m_value);

	if (svGroupIndex * 2 + 1 < inp.Count())
		InterlockedAdd(g_buff[0], inp[svGroupIndex * 2 + 1].m_value);
}
)";

		ShaderPtr shader = createShader(kSrc, ShaderType::kWorkGraph);

		ShaderProgramInitInfo progInit;
		progInit.m_workGraph.m_shader = shader.get();
		WorkGraphNodeSpecialization wgSpecialization = {"main", UVec3(4, 1, 1)};
		progInit.m_workGraph.m_nodeSpecializations = ConstWeakArray<WorkGraphNodeSpecialization>(&wgSpecialization, 1);
		ShaderProgramPtr prog = GrManager::getSingleton().newShaderProgram(progInit);

		BufferPtr counterBuff = createBuffer(BufferUsageBit::kAllStorage | BufferUsageBit::kTransferSource, 0u, 1, "CounterBuffer");

		BufferInitInfo scratchInit("scratch");
		scratchInit.m_size = prog->getWorkGraphMemoryRequirements();
		scratchInit.m_usage = BufferUsageBit::kAllStorage;
		BufferPtr scratchBuff = GrManager::getSingleton().newBuffer(scratchInit);

		struct FirstNodeRecord
		{
			UVec3 m_gridSize;
			U32 m_value;
		};

		Array<FirstNodeRecord, 2> records;
		for(U32 i = 0; i < records.getSize(); ++i)
		{
			records[i].m_gridSize = UVec3(4, 1, 1);
			records[i].m_value = (i + 1) * 10;
		}

		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(CommandBufferInitInfo(CommandBufferFlag::kSmallBatch));
		cmdb->bindShaderProgram(prog.get());
		cmdb->bindStorageBuffer(ANKI_REG(u0), BufferView(counterBuff.get()));
		cmdb->dispatchGraph(BufferView(scratchBuff.get()), records.getBegin(), records.getSize(), sizeof(records[0]));
		cmdb->endRecording();

		FencePtr fence;
		GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
		fence->clientWait(kMaxSecond);

		validateBuffer(counterBuff, 122880);
	}

	commonDestroy();
}

ANKI_TEST(Gr, WorkGraphAmplification)
{
	constexpr Bool benchmark = true;

	// CVarSet::getSingleton().setMultiple(Array<const Char*, 2>{"Device", "2"});
	commonInit(!benchmark);

	{
		const Char* kSrc = R"(
struct FirstNodeRecord
{
	uint3 m_dispatchGrid : SV_DispatchGrid;
};

struct SecondNodeRecord
{
	uint3 m_dispatchGrid : SV_DispatchGrid;
	uint m_objectIndex;
};

struct Aabb
{
	uint m_min;
	uint m_max;
};

struct Object
{
	uint m_positionsStart; // Points to g_positions
	uint m_positionCount;
};

RWStructuredBuffer<Aabb> g_aabbs : register(u0);
StructuredBuffer<Object> g_objects : register(t0);
StructuredBuffer<uint> g_positions : register(t1);

#define THREAD_COUNT 64u

// Operates per object
[Shader("node")] [NodeLaunch("broadcasting")] [NodeIsProgramEntry] [NodeMaxDispatchGrid(1, 1, 1)]
[NumThreads(THREAD_COUNT, 1, 1)]
void main(DispatchNodeInputRecord<FirstNodeRecord> inp, [MaxRecords(THREAD_COUNT)] NodeOutput<SecondNodeRecord> computeAabb,
		  uint svGroupIndex : SV_GroupIndex, uint svDispatchThreadId : SV_DispatchThreadId)
{
	GroupNodeOutputRecords<SecondNodeRecord> recs = computeAabb.GetGroupNodeOutputRecords(THREAD_COUNT);

	const Object obj = g_objects[svDispatchThreadId];

	recs[svGroupIndex].m_objectIndex = svDispatchThreadId;
	recs[svGroupIndex].m_dispatchGrid = uint3((obj.m_positionCount + (THREAD_COUNT - 1)) / THREAD_COUNT, 1, 1);

	recs.OutputComplete();
}

groupshared Aabb g_aabb;

// Operates per position
[Shader("node")] [NodeLaunch("broadcasting")] [NodeMaxDispatchGrid(1, 1, 1)] [NumThreads(THREAD_COUNT, 1, 1)]
void computeAabb(DispatchNodeInputRecord<SecondNodeRecord> inp, uint svDispatchThreadId : SV_DispatchThreadId, uint svGroupIndex : SV_GroupIndex)
{
	const Object obj = g_objects[inp.Get().m_objectIndex];

	svDispatchThreadId = min(svDispatchThreadId, obj.m_positionCount - 1);

	if(svGroupIndex == 0)
	{
		g_aabb.m_min = 0xFFFFFFFF;
		g_aabb.m_max = 0;
	}

	Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

	const uint positionIndex = obj.m_positionsStart + svDispatchThreadId;

	const uint pos = g_positions[positionIndex];
	InterlockedMin(g_aabb.m_min, pos);
	InterlockedMax(g_aabb.m_max, pos);

	Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

	InterlockedMin(g_aabbs[inp.Get().m_objectIndex].m_min, g_aabb.m_min);
	InterlockedMax(g_aabbs[inp.Get().m_objectIndex].m_max, g_aabb.m_max);
}
)";

		const Char* kComputeSrc = R"(
struct Aabb
{
	uint m_min;
	uint m_max;
};

struct Object
{
	uint m_positionsStart; // Points to g_positions
	uint m_positionCount;
};

struct PushConsts
{
	uint m_objectIndex;
	uint m_padding1;
	uint m_padding2;
	uint m_padding3;
};

RWStructuredBuffer<Aabb> g_aabbs : register(u0);
StructuredBuffer<Object> g_objects : register(t0);
StructuredBuffer<uint> g_positions : register(t1);

#if defined(__spirv__)
[[vk::push_constant]] ConstantBuffer<PushConsts> g_pushConsts;
#else
ConstantBuffer<PushConsts> g_pushConsts : register(b0, space3000);
#endif

#define THREAD_COUNT 64u

groupshared Aabb g_aabb;

[NumThreads(THREAD_COUNT, 1, 1)]
void main(uint svDispatchThreadId : SV_DispatchThreadId, uint svGroupIndex : SV_GroupIndex)
{
	const Object obj = g_objects[g_pushConsts.m_objectIndex];

	svDispatchThreadId = min(svDispatchThreadId, obj.m_positionCount - 1);

	if(svGroupIndex == 0)
	{
		g_aabb.m_min = 0xFFFFFFFF;
		g_aabb.m_max = 0;
	}

	Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

	const uint positionIndex = obj.m_positionsStart + svDispatchThreadId;

	const uint pos = g_positions[positionIndex];
	InterlockedMin(g_aabb.m_min, pos);
	InterlockedMax(g_aabb.m_max, pos);

	Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);

	InterlockedMin(g_aabbs[g_pushConsts.m_objectIndex].m_min, g_aabb.m_min);
	InterlockedMax(g_aabbs[g_pushConsts.m_objectIndex].m_max, g_aabb.m_max);
}
)";

		constexpr U32 kObjectCount = 4000 * 64;
		constexpr U32 kPositionsPerObject = 10 * 64; // 1 * 1024;
		constexpr U32 kThreadCount = 64;
		constexpr Bool useWorkgraphs = true;

		ShaderProgramPtr prog;
		if(useWorkgraphs)
		{
			ShaderPtr shader = createShader(kSrc, ShaderType::kWorkGraph);
			ShaderProgramInitInfo progInit;
			Array<WorkGraphNodeSpecialization, 2> specializations = {
				{{"main", UVec3((kObjectCount + kThreadCount - 1) / kThreadCount, 1, 1)},
				 {"computeAabb", UVec3((kPositionsPerObject + (kThreadCount - 1)) / kThreadCount, 1, 1)}}};
			progInit.m_workGraph.m_nodeSpecializations = specializations;
			progInit.m_workGraph.m_shader = shader.get();
			prog = GrManager::getSingleton().newShaderProgram(progInit);
		}
		else
		{
			ShaderPtr shader = createShader(kComputeSrc, ShaderType::kCompute);
			ShaderProgramInitInfo progInit;
			progInit.m_computeShader = shader.get();
			prog = GrManager::getSingleton().newShaderProgram(progInit);
		}

		struct Aabb
		{
			U32 m_min = kMaxU32;
			U32 m_max = 0;

			Bool operator==(const Aabb&) const = default;
		};

		struct Object
		{
			U32 m_positionsStart; // Points to g_positions
			U32 m_positionCount;
		};

		// Objects
		DynamicArray<Object> objects;
		objects.resize(kObjectCount);
		U32 positionCount = 0;
		for(Object& obj : objects)
		{
			obj.m_positionsStart = positionCount;
			obj.m_positionCount = kPositionsPerObject;

			positionCount += obj.m_positionCount;
		}

		printf("Obj count %u, pos count %u\n", kObjectCount, positionCount);

		BufferPtr objBuff = createBuffer(BufferUsageBit::kStorageComputeRead, ConstWeakArray(objects), "Objects");

		// AABBs
		BufferPtr aabbsBuff = createBuffer(BufferUsageBit::kStorageComputeWrite, Aabb(), kObjectCount, "AABBs");

		// Positions
		GrDynamicArray<U32> positions;
		positions.resize(positionCount);
		positionCount = 0;
		for(U32 iobj = 0; iobj < kObjectCount; ++iobj)
		{
			const Object& obj = objects[iobj];

			const U32 min = getRandomRange<U32>(0, kMaxU32 / 2 - 1);
			const U32 max = getRandomRange<U32>(kMaxU32 / 2, kMaxU32);

			for(U32 ipos = obj.m_positionsStart; ipos < obj.m_positionsStart + obj.m_positionCount; ++ipos)
			{
				positions[ipos] = getRandomRange<U32>(min, max);

				positions[ipos] = iobj;
			}

			positionCount += obj.m_positionCount;
		}

		BufferPtr posBuff = createBuffer(BufferUsageBit::kStorageComputeRead, ConstWeakArray(positions), "Positions");

		// Execute
		for(U32 i = 0; i < ((benchmark) ? 200 : 1); ++i)
		{
			[[maybe_unused]] const Error err = Input::getSingleton().handleEvents();

			BufferPtr scratchBuff;
			if(useWorkgraphs)
			{
				BufferInitInfo scratchInit("scratch");
				scratchInit.m_size = prog->getWorkGraphMemoryRequirements();
				scratchInit.m_usage = BufferUsageBit::kAllStorage;
				scratchBuff = GrManager::getSingleton().newBuffer(scratchInit);
			}

			const Second timeA = HighRezTimer::getCurrentTime();

			CommandBufferPtr cmdb;
			if(useWorkgraphs)
			{
				struct FirstNodeRecord
				{
					UVec3 m_gridSize;
				};

				Array<FirstNodeRecord, 1> records;
				records[0].m_gridSize = UVec3((objects.getSize() + kThreadCount - 1) / kThreadCount, 1, 1);

				cmdb = GrManager::getSingleton().newCommandBuffer(
					CommandBufferInitInfo(CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork));
				cmdb->bindShaderProgram(prog.get());
				cmdb->bindStorageBuffer(ANKI_REG(u0), BufferView(aabbsBuff.get()));
				cmdb->bindStorageBuffer(ANKI_REG(t0), BufferView(objBuff.get()));
				cmdb->bindStorageBuffer(ANKI_REG(t1), BufferView(posBuff.get()));
				cmdb->dispatchGraph(BufferView(scratchBuff.get()), records.getBegin(), records.getSize(), sizeof(records[0]));
			}
			else
			{
				cmdb = GrManager::getSingleton().newCommandBuffer(CommandBufferInitInfo(CommandBufferFlag::kGeneralWork));
				cmdb->bindShaderProgram(prog.get());
				cmdb->bindStorageBuffer(ANKI_REG(u0), BufferView(aabbsBuff.get()));
				cmdb->bindStorageBuffer(ANKI_REG(t0), BufferView(objBuff.get()));
				cmdb->bindStorageBuffer(ANKI_REG(t1), BufferView(posBuff.get()));

				for(U32 iobj = 0; iobj < kObjectCount; ++iobj)
				{
					const UVec4 pc(iobj);
					cmdb->setPushConstants(&pc, sizeof(pc));

					cmdb->dispatchCompute((objects[iobj].m_positionCount + kThreadCount - 1) / kThreadCount, 1, 1);
				}
			}

			clearSwapchain(cmdb);

			cmdb->endRecording();

			const Second timeB = HighRezTimer::getCurrentTime();

			FencePtr fence;
			GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
			fence->clientWait(kMaxSecond);

			GrManager::getSingleton().swapBuffers();

			const Second timeC = HighRezTimer::getCurrentTime();

			printf("GPU time: %fms, cmdb build time: %fms\n", (timeC - timeB) * 1000.0, (timeB - timeA) * 1000.0);
		}

		// Check
		DynamicArray<Aabb> aabbs;
		readBuffer(aabbsBuff, aabbs);
		for(U32 i = 0; i < kObjectCount; ++i)
		{
			const Object& obj = objects[i];
			Aabb aabb;
			for(U32 ipos = obj.m_positionsStart; ipos < obj.m_positionsStart + obj.m_positionCount; ++ipos)
			{
				aabb.m_min = min(aabb.m_min, positions[ipos]);
				aabb.m_max = max(aabb.m_max, positions[ipos]);
			}

			if(aabb != aabbs[i])
			{
				printf("%u: %u %u | %u %u\n", i, aabb.m_min, aabbs[i].m_min, aabb.m_max, aabbs[i].m_max);
			}
			ANKI_TEST_EXPECT_EQ(aabb, aabbs[i]);
		}
	}

	commonDestroy();
}
