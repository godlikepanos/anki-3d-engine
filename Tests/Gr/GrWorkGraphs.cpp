// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>

ANKI_TEST(Gr, WorkGraphHelloWorld)
{
	// CVarSet::getSingleton().setMultiple(Array<const Char*, 2>{"Device", "2"});
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

[Shader("node")] [NodeLaunch("broadcasting")] [NodeMaxDispatchGrid(16, 1, 1)] [NumThreads(16, 1, 1)]
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
		progInit.m_workGraphShader = shader.get();
		ShaderProgramPtr prog = GrManager::getSingleton().newShaderProgram(progInit);

		BufferPtr counterBuff = createBuffer(BufferUsageBit::kAllStorage | BufferUsageBit::kTransferSource, 0u, "CounterBuffer");

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
