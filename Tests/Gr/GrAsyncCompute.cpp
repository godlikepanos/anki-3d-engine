// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>
#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/HighRezTimer.h>

using namespace anki;

static void generateSphere(DynamicArray<Vec3>& positions, DynamicArray<UVec3>& indices, U32 sliceCount, U32 stackCount)
{
	positions.emplaceBack(0.0f, 1.0f, 0.0f);
	const U32 v0 = 0;

	// generate vertices per stack / slice
	for(U32 i = 0u; i < stackCount - 1; i++)
	{
		const F32 phi = kPi * (i + 1) / stackCount;
		for(F32 j = 0u; j < sliceCount; j++)
		{
			const F32 theta = 2.0f * kPi * F32(j) / sliceCount;
			const F32 x = sin(phi) * cos(theta);
			const F32 y = cos(phi);
			const F32 z = sin(phi) * sin(theta);
			positions.emplaceBack(x, y, z);
		}
	}

	// add bottom vertex
	positions.emplaceBack(0.0f, -1.0f, 0.0f);
	const U32 v1 = U32(positions.getSize() - 1);

	// add top / bottom triangles
	for(auto i = 0u; i < sliceCount; ++i)
	{
		auto i0 = i + 1;
		auto i1 = (i + 1) % sliceCount + 1;
		indices.emplaceBack(v0, i1, i0);
		i0 = i + sliceCount * (stackCount - 2) + 1;
		i1 = (i + 1) % sliceCount + sliceCount * (stackCount - 2) + 1;
		indices.emplaceBack(v1, i0, i1);
	}

	// add quads per stack / slice
	for(U32 j = 0u; j < stackCount - 2; j++)
	{
		const U32 j0 = j * sliceCount + 1;
		const U32 j1 = (j + 1) * sliceCount + 1;
		for(U32 i = 0u; i < sliceCount; i++)
		{
			const U32 i0 = j0 + i;
			const U32 i1 = j0 + (i + 1) % sliceCount;
			const U32 i2 = j1 + (i + 1) % sliceCount;
			const U32 i3 = j1 + i;

			indices.emplaceBack(i0, i1, i2);
			indices.emplaceBack(i0, i2, i3);
		}
	}
}

ANKI_TEST(Gr, AsyncComputeBench)
{
	const Bool useAsyncQueue = true;
	const Bool runConcurently = true;
	const U32 spheresToDrawPerDimension = 100;
	const U32 windowSize = 512;

	g_validationCVar = false; // TODO
	g_debugMarkersCVar = false;
	g_windowWidthCVar = windowSize;
	g_windowHeightCVar = windowSize;
	g_asyncComputeCVar = 0;

	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);
	ShaderCompilerMemoryPool::allocateSingleton(allocAligned, nullptr);
	initWindow();
	initGrManager();
	Input::allocateSingleton();

	{
		const CString computeShaderSrc = R"(
RWTexture2D<float4> g_inTex : register(u0);
RWTexture2D<float4> g_outTex : register(u1);

[NumThreads(8, 8, 1)] void main(uint2 svDispatchThreadId : SV_DispatchThreadID)
{
	uint2 texSize;
	g_inTex.GetDimensions(texSize.x, texSize.y);

	float4 val = 0.0;
	for(int x = -9; x <= 9; ++x)
	{
		for(int y = -9; y <= 9; ++y)
		{
			int2 coord = int2(svDispatchThreadId) + int2(x, y);
			if(coord.x < 0 || coord.y < 0 || coord.x >= texSize.x || coord.y >= texSize.y)
			{
				continue;
			}

			val += g_inTex[coord];
		}
	}

	g_outTex[svDispatchThreadId] = val;
})";
		const CString vertShaderSrc = R"(
struct Consts
{
	float3 m_worldPosition;
	float m_scale;

	float4x4 m_viewProjMat;
};

#if defined(__spirv__)
[[vk::push_constant]] ConstantBuffer<Consts> g_consts;
#else
ConstantBuffer<Consts> g_consts : register(b0, space3000);
#endif

float4 main(float3 svPosition : POSITION) : SV_Position
{
	return mul(g_consts.m_viewProjMat, float4(svPosition * g_consts.m_scale + g_consts.m_worldPosition, 1.0));
})";

		const CString pixelShaderSrc = R"(
float4 main() : SV_Target0
{
	return float4(1.0, 0.0, 0.5, 0.0);
})";

		const CString blitVertShader = R"(
struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float2 m_uv : TEXCOORD;
};

VertOut main(uint vertId : SV_VERTEXID)
{
	const float2 coord = float2(vertId >> 1, vertId & 1);

	VertOut output;
	output.m_svPosition = float4(coord * float2(4.0, -4.0) + float2(-1.0, 1.0), 0.0, 1.0);
	output.m_uv = coord * 2.0f;

	return output;
})";

		const CString blitPixelShader = R"(
struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float2 m_uv : TEXCOORD;
};

Texture2D g_inTex : register(t0);
SamplerState g_sampler : register(s0);

float4 main(VertOut input) : SV_Target0
{
	return g_inTex.Sample(g_sampler, input.m_uv);
})";

		ShaderProgramPtr compProg = createComputeProg(computeShaderSrc);
		ShaderProgramPtr graphicsProg = createVertFragProg(vertShaderSrc, pixelShaderSrc);
		ShaderProgramPtr blitProg = createVertFragProg(blitVertShader, blitPixelShader);

		DynamicArray<Vec3> positions;
		DynamicArray<UVec3> indices;
		generateSphere(positions, indices, 50, 50);

		BufferPtr posBuff = createBuffer(BufferUsageBit::kVertexOrIndex, ConstWeakArray(positions), "PosBuffer");
		BufferPtr indexBuff = createBuffer(BufferUsageBit::kVertexOrIndex, ConstWeakArray(indices), "IdxBuffer");

		TextureInitInfo texInit("Tex");
		texInit.m_width = texInit.m_height = 2048;
		texInit.m_format = Format::kR32G32B32A32_Sfloat;
		texInit.m_usage = TextureUsageBit::kUavCompute;
		TexturePtr inTex = createTexture2d(texInit, Vec4(0.5f));
		TexturePtr outTex = createTexture2d(texInit, Vec4(0.1f));

		{
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
			CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cinit);

			const TextureBarrierInfo barrier2 = {TextureView(inTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kCopyDestination,
												 TextureUsageBit::kUavCompute};
			cmdb->setPipelineBarrier({&barrier2, 1}, {}, {});
			cmdb->endRecording();

			FencePtr fence;
			GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
			fence->clientWait(kMaxSecond);
		}

		TextureInitInfo texInit2("RT");
		texInit2.m_width = texInit2.m_height = windowSize;
		texInit2.m_format = Format::kR32G32B32A32_Sfloat;
		texInit2.m_usage = TextureUsageBit::kRtvDsvWrite | TextureUsageBit::kSrvPixel;
		TexturePtr rtTex = createTexture2d(texInit2, Vec4(0.5f));

		SamplerInitInfo samplerInit("sampler");
		SamplerPtr sampler = GrManager::getSingleton().newSampler(samplerInit);

		Array<TimestampQueryPtr, 2> startTimestamps = {GrManager::getSingleton().newTimestampQuery(), GrManager::getSingleton().newTimestampQuery()};
		TimestampQueryPtr endTimestamp = GrManager::getSingleton().newTimestampQuery();

		FencePtr finalFence;

		const U32 iterationCount = 1000;
		for(U32 i = 0; i < iterationCount; ++i)
		{
			ANKI_TEST_EXPECT_NO_ERR(Input::getSingleton().handleEvents());

			GrManager::getSingleton().beginFrame();

			TexturePtr presentTex = GrManager::getSingleton().acquireNextPresentableTexture();

			// Init command buffers
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
			CommandBufferPtr gfxCmdb = GrManager::getSingleton().newCommandBuffer(cinit);

			CommandBufferPtr compCmdb;
			if(useAsyncQueue)
			{
				CommandBufferInitInfo cinit;
				cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
				compCmdb = GrManager::getSingleton().newCommandBuffer(cinit);
			}
			else
			{
				compCmdb = gfxCmdb;
			}

			CommandBufferPtr blitCmdb = GrManager::getSingleton().newCommandBuffer(cinit);

			// Barriers
			{
				const TextureBarrierInfo rtBarrier = {TextureView(rtTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
													  TextureUsageBit::kRtvDsvWrite};
				gfxCmdb->setPipelineBarrier({&rtBarrier, 1}, {}, {});

				const TextureBarrierInfo uavBarrier = {TextureView(outTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
													   TextureUsageBit::kUavCompute};
				compCmdb->setPipelineBarrier({&uavBarrier, 1}, {}, {});

				const TextureBarrierInfo blitBarrier = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
														TextureUsageBit::kRtvDsvWrite};
				blitCmdb->setPipelineBarrier({&blitBarrier, 1}, {}, {});
			}

			// Compute dispatch
			{
				if(i == 0)
				{
					compCmdb->writeTimestamp(startTimestamps[0].get());
				}

				compCmdb->bindShaderProgram(compProg.get());
				compCmdb->bindUav(0, 0, TextureView(inTex.get(), TextureSubresourceDesc::all()));
				compCmdb->bindUav(1, 0, TextureView(outTex.get(), TextureSubresourceDesc::all()));
				compCmdb->dispatchCompute(inTex->getWidth() / 8, inTex->getHeight() / 8, 1);
			}

			// Draw spheres
			{
				if(i == 0)
				{
					compCmdb->writeTimestamp(startTimestamps[1].get());
				}

				RenderTarget rt;
				rt.m_textureView = TextureView(rtTex.get(), TextureSubresourceDesc::all());
				rt.m_loadOperation = RenderTargetLoadOperation::kClear;
				rt.m_clearValue.m_colorf = {getRandomRange(0.0f, 1.0f), getRandomRange(0.0f, 1.0f), getRandomRange(0.0f, 1.0f), 1.0f};
				gfxCmdb->beginRenderPass({rt});

				gfxCmdb->bindVertexBuffer(0, BufferView(posBuff.get()), sizeof(Vec3));
				gfxCmdb->setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
				gfxCmdb->bindIndexBuffer(BufferView(indexBuff.get()), IndexType::kU32);
				gfxCmdb->bindShaderProgram(graphicsProg.get());
				gfxCmdb->setViewport(0, 0, windowSize, windowSize);

				struct Consts
				{
					Vec3 m_worldPosition;
					F32 m_scale;

					Mat4 m_viewProjMat;
				} consts;

				constexpr F32 orthoHalfSize = 10.0f;
				constexpr F32 orthoSize = orthoHalfSize * 2.0f;
				const Mat4 viewMat = Mat4::getIdentity().invert();
				const Mat4 projMat =
					Mat4::calculateOrthographicProjectionMatrix(orthoHalfSize, -orthoHalfSize, orthoHalfSize, -orthoHalfSize, 0.1f, 200.0f);
				consts.m_viewProjMat = projMat * viewMat;

				consts.m_scale = 0.07f;

				for(U32 x = 0; x < spheresToDrawPerDimension; ++x)
				{
					for(U32 y = 0; y < spheresToDrawPerDimension; ++y)
					{
						consts.m_worldPosition = Vec3(F32(x) / (spheresToDrawPerDimension - 1) * orthoSize - orthoHalfSize,
													  F32(y) / (spheresToDrawPerDimension - 1) * orthoSize - orthoHalfSize, -1.0f);

						gfxCmdb->setFastConstants(&consts, sizeof(consts));

						gfxCmdb->drawIndexed(PrimitiveTopology::kTriangles, U32(indexBuff->getSize() / sizeof(U32)));
					}
				}

				gfxCmdb->endRenderPass();
			}

			// Blit
			{
				const TextureBarrierInfo blitBarrier = {TextureView(rtTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kRtvDsvWrite,
														TextureUsageBit::kSrvPixel};
				blitCmdb->setPipelineBarrier({&blitBarrier, 1}, {}, {});

				RenderTarget rt;
				rt.m_textureView = TextureView(presentTex.get(), TextureSubresourceDesc::all());
				rt.m_loadOperation = RenderTargetLoadOperation::kDontCare;
				rt.m_clearValue.m_colorf = {getRandomRange(0.0f, 1.0f), getRandomRange(0.0f, 1.0f), getRandomRange(0.0f, 1.0f), 1.0f};
				blitCmdb->beginRenderPass({rt});

				blitCmdb->bindShaderProgram(blitProg.get());
				blitCmdb->bindSrv(0, 0, TextureView(rtTex.get(), TextureSubresourceDesc::all()));
				blitCmdb->bindSampler(0, 0, sampler.get());
				blitCmdb->setViewport(0, 0, windowSize, windowSize);
				blitCmdb->draw(PrimitiveTopology::kTriangles, 3);

				blitCmdb->endRenderPass();

				const TextureBarrierInfo presentBarrier = {TextureView(presentTex.get(), TextureSubresourceDesc::all()),
														   TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kPresent};
				blitCmdb->setPipelineBarrier({&presentBarrier, 1}, {}, {});

				if(i == iterationCount - 1)
				{
					compCmdb->writeTimestamp(endTimestamp.get());
				}
			}

			gfxCmdb->endRecording();
			blitCmdb->endRecording();
			if(useAsyncQueue)
			{
				compCmdb->endRecording();
			}

			if(useAsyncQueue)
			{
				WeakArray<Fence*> firstWaveWaitFences;
				Array<Fence*, 1> arr;
				if(finalFence.isCreated())
				{
					arr = {finalFence.get()};
					firstWaveWaitFences = {arr};
				}

				FencePtr fence2;
				GrManager::getSingleton().submit(compCmdb.get(), firstWaveWaitFences, &fence2);

				FencePtr fence1;
				GrManager::getSingleton().submit(gfxCmdb.get(), firstWaveWaitFences, &fence1);

				Array<Fence*, 2> waitFences = {{fence1.get(), fence2.get()}};
				GrManager::getSingleton().submit(blitCmdb.get(), {waitFences}, &finalFence);
			}
			else
			{
				GrManager::getSingleton().submit(gfxCmdb.get());
				GrManager::getSingleton().submit(blitCmdb.get(), {}, &finalFence);
			}

			GrManager::getSingleton().endFrame();
		}

		finalFence->clientWait(kMaxSecond);

		Array<Second, 2> startTime;
		ANKI_TEST_EXPECT_EQ(startTimestamps[0]->getResult(startTime[0]), TimestampQueryResult::kAvailable);
		ANKI_TEST_EXPECT_EQ(startTimestamps[1]->getResult(startTime[1]), TimestampQueryResult::kAvailable);
		Second endTime;
		ANKI_TEST_EXPECT_EQ(endTimestamp->getResult(endTime), TimestampQueryResult::kAvailable);

		ANKI_TEST_LOGI("GPU time %f\n", endTime - min(startTime[0], startTime[1]));
	}

	Input::freeSingleton();
	GrManager::freeSingleton();
	NativeWindow::freeSingleton();
	ShaderCompilerMemoryPool::freeSingleton();
	DefaultMemoryPool::freeSingleton();
}
