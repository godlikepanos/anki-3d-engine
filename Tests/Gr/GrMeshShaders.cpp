// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>
#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/HighRezTimer.h>

ANKI_TEST(Gr, MeshShaders)
{
	constexpr U32 kTileCount = 4;
	constexpr U32 kVertCount = 4;

	g_validationCVar.set(true);
	g_windowWidthCVar.set(64 * kTileCount);
	g_windowHeightCVar.set(64);
	g_meshShadersCVar.set(true);

	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);
	ShaderCompilerMemoryPool::allocateSingleton(allocAligned, nullptr);
	initWindow();
	initGrManager();

	{
		const CString taskShaderSrc = R"(
struct Payload
{
	uint m_meshletIndices[64];
};

groupshared Payload s_payload;
groupshared uint s_visibleCount;

struct Meshlet
{
	uint m_firstIndex;
	uint m_firstVertex;
};

StructuredBuffer<Meshlet> g_meshlets : register(t3);

[numthreads(64, 1, 1)] void main(uint svDispatchThreadId : SV_DISPATCHTHREADID)
{
	uint meshletCount, unused;
	g_meshlets.GetDimensions(meshletCount, unused);

	bool visible = ((svDispatchThreadId & 1u) == 0u) && (svDispatchThreadId < meshletCount);

	s_visibleCount = 0;
	GroupMemoryBarrierWithGroupSync();

	if(visible)
	{
		uint index;
		InterlockedAdd(s_visibleCount, 1u, index);

		s_payload.m_meshletIndices[index] = svDispatchThreadId;
	}

	GroupMemoryBarrierWithGroupSync();
	DispatchMesh(s_visibleCount, 1, 1, s_payload);
})";

		const CString meshShaderSrc = R"(
struct Payload
{
	uint m_meshletIndices[64];
};

struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float3 m_color : COLOR0;
};

struct Meshlet
{
	uint m_firstIndex;
	uint m_firstVertex;
};

StructuredBuffer<uint> g_indices : register(t0);
StructuredBuffer<float4> g_positions : register(t1);
StructuredBuffer<float4> g_colors : register(t2);
StructuredBuffer<Meshlet> g_meshlets : register(t3);

[numthreads(6, 1, 1)] [outputtopology("triangle")] void main(in payload Payload payload, out vertices VertOut verts[4],
                                                             out indices uint3 indices[6], uint svGroupId : SV_GROUPID,
                                                             uint svGroupIndex : SV_GROUPINDEX)
{
	uint meshletIdx = payload.m_meshletIndices[svGroupId];
	Meshlet meshlet = g_meshlets[meshletIdx];

	SetMeshOutputCounts(4, 6);

	if(svGroupIndex < 4)
	{
		verts[svGroupIndex].m_svPosition = g_positions[meshlet.m_firstVertex + svGroupIndex];
		verts[svGroupIndex].m_color = g_colors[meshlet.m_firstVertex + svGroupIndex];
	}

	[unroll] for(uint i = 0; i < 6; ++i)
	{
		indices[svGroupIndex][i] = g_indices[meshlet.m_firstIndex + i];
	}
})";

		const CString fragShaderSrc = R"(
struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float3 m_color : COLOR0;
};

float3 main(VertOut input) : SV_TARGET0
{
	return input.m_color;
})";

		ShaderProgramPtr prog;
		{
			ShaderPtr taskShader = createShader(taskShaderSrc, ShaderType::kTask);
			ShaderPtr meshShader = createShader(meshShaderSrc, ShaderType::kMesh);
			ShaderPtr fragShader = createShader(fragShaderSrc, ShaderType::kFragment);

			ShaderProgramInitInfo progInit("Program");
			progInit.m_graphicsShaders[ShaderType::kTask] = taskShader.get();
			progInit.m_graphicsShaders[ShaderType::kMesh] = meshShader.get();
			progInit.m_graphicsShaders[ShaderType::kFragment] = fragShader.get();
			prog = GrManager::getSingleton().newShaderProgram(progInit);
		}

		BufferPtr indexBuff;
		{
			BufferInitInfo buffInit("Index");
			buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
			buffInit.m_usage = BufferUsageBit::kSrvGeometry;
			buffInit.m_size = sizeof(U32) * 6;
			indexBuff = GrManager::getSingleton().newBuffer(buffInit);

			void* mapped = indexBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
			const U32 indices[] = {0, 1, 2, 2, 1, 3};
			memcpy(mapped, indices, sizeof(indices));
			indexBuff->unmap();
		}

		BufferPtr positionsBuff;
		{
			BufferInitInfo buffInit("Positions");
			buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
			buffInit.m_usage = BufferUsageBit::kSrvGeometry;
			buffInit.m_size = kVertCount * sizeof(Vec4) * kTileCount;
			positionsBuff = GrManager::getSingleton().newBuffer(buffInit);

			Vec4* mapped = static_cast<Vec4*>(positionsBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

			for(U32 t = 0; t < kTileCount; t++)
			{
				const F32 left = (1.0f / F32(kTileCount) * F32(t)) * 2.0f - 1.0f;
				const F32 right = (1.0f / F32(kTileCount) * F32(t + 1)) * 2.0f - 1.0f;
				mapped[0] = Vec4(left, -1.0f, 1.0f, 1.0f);
				mapped[1] = Vec4(right, -1.0f, 1.0f, 1.0f);
				mapped[2] = Vec4(left, 1.0f, 1.0f, 1.0f);
				mapped[3] = Vec4(right, 1.0f, 1.0f, 1.0f);

				mapped += 4;
			}

			positionsBuff->unmap();
		}

		BufferPtr colorsBuff;
		{
			BufferInitInfo buffInit("Colors");
			buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
			buffInit.m_usage = BufferUsageBit::kSrvGeometry;
			buffInit.m_size = kVertCount * sizeof(Vec4) * kTileCount;
			colorsBuff = GrManager::getSingleton().newBuffer(buffInit);

			Vec4* mapped = static_cast<Vec4*>(colorsBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

			const Array<Vec4, kTileCount> colors = {Vec4(1.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 1.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
													Vec4(1.0f, 1.0f, 0.0f, 0.0f)};
			for(U32 t = 0; t < kTileCount; t++)
			{
				mapped[0] = mapped[1] = mapped[2] = mapped[3] = colors[t];

				mapped += 4;
			}

			colorsBuff->unmap();
		}

		BufferPtr meshletsBuff;
		{
			class Meshlet
			{
			public:
				U32 m_firstIndex;
				U32 m_firstVertex;
			};

			BufferInitInfo buffInit("Meshlets");
			buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
			buffInit.m_usage = BufferUsageBit::kSrvGeometry;
			buffInit.m_size = sizeof(Meshlet) * kTileCount;
			meshletsBuff = GrManager::getSingleton().newBuffer(buffInit);

			Meshlet* mapped = static_cast<Meshlet*>(meshletsBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

			for(U32 t = 0; t < kTileCount; t++)
			{
				mapped[t].m_firstIndex = 0;
				mapped[t].m_firstVertex = kVertCount * t;
			}

			meshletsBuff->unmap();
		}

		for(U32 i = 0; i < 100; ++i)
		{
			TexturePtr swapchainTex = GrManager::getSingleton().acquireNextPresentableTexture();

			CommandBufferInitInfo cmdbinit;
			CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbinit);

			cmdb->setViewport(0, 0, g_windowWidthCVar.get(), g_windowHeightCVar.get());

			TextureBarrierInfo barrier;
			barrier.m_textureView = TextureView(swapchainTex.get(), TextureSubresourceDesc::all());
			barrier.m_previousUsage = TextureUsageBit::kNone;
			barrier.m_nextUsage = TextureUsageBit::kRtvDsvWrite;
			cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

			RenderTarget rt;
			rt.m_textureView = TextureView(swapchainTex.get(), TextureSubresourceDesc::all());
			rt.m_clearValue.m_colorf = {1.0f, 0.0f, 1.0f, 0.0f};
			cmdb->beginRenderPass({rt});

			cmdb->bindStorageBuffer(ANKI_REG(t0), BufferView(indexBuff.get()));
			cmdb->bindStorageBuffer(ANKI_REG(t1), BufferView(positionsBuff.get()));
			cmdb->bindStorageBuffer(ANKI_REG(t2), BufferView(colorsBuff.get()));
			cmdb->bindStorageBuffer(ANKI_REG(t3), BufferView(meshletsBuff.get()));

			cmdb->bindShaderProgram(prog.get());

			cmdb->drawMeshTasks(1, 1, 1);

			cmdb->endRenderPass();

			barrier.m_previousUsage = TextureUsageBit::kRtvDsvWrite;
			barrier.m_nextUsage = TextureUsageBit::kPresent;
			cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

			cmdb->endRecording();
			GrManager::getSingleton().submit(cmdb.get());

			GrManager::getSingleton().swapBuffers();

			HighRezTimer::sleep(1.0_sec / 60.0);
		}
	}

	GrManager::freeSingleton();
	NativeWindow::freeSingleton();
	ShaderCompilerMemoryPool::freeSingleton();
	DefaultMemoryPool::freeSingleton();
}
