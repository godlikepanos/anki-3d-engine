
// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>
#include <AnKi/Window/NativeWindow.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Resource/TransferGpuAllocator.h>
#include <AnKi/ShaderCompiler/ShaderParser.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>
#include <ctime>

using namespace anki;

ANKI_TEST(Gr, GrManager)
{
	g_validationCVar = true;

	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);
	initWindow();
	ANKI_TEST_EXPECT_NO_ERR(Input::allocateSingleton().init());
	initGrManager();

	GrManager::freeSingleton();
	Input::freeSingleton();
	NativeWindow::freeSingleton();
	DefaultMemoryPool::freeSingleton();
}

ANKI_TEST(Gr, Shader)
{
#if 0
	COMMON_BEGIN()

	ShaderPtr shader = createShader(FRAG_MRT_SRC, ShaderType::kPixel, *g_gr);

	COMMON_END()
#endif
}

ANKI_TEST(Gr, ShaderProgram)
{
#if 0
	COMMON_BEGIN()

	constexpr const char* kVertSrc = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, 1.0), vec2(0.0, -1.0), vec2(1.0, 1.0));

	gl_Position = vec4(POSITIONS[gl_VertexID % 3], 0.0, 1.0);
})";

	constexpr const char* kFragSrc = R"(layout (location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(0.5);
})";

	ShaderProgramPtr ppline = createProgram(kVertSrc, kFragSrc, *g_gr);

	COMMON_END()
#endif
}

ANKI_TEST(Gr, ClearScreen)
{
	commonInit();
	ANKI_TEST_LOGI("Expect to see a magenta background");

	constexpr U kIterations = 100;
	U iterations = kIterations;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		TexturePtr presentTex = GrManager::getSingleton().acquireNextPresentableTexture();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cinit);

		const TextureBarrierInfo barrier = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
											TextureUsageBit::kRtvDsvWrite};
		cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

		RenderTarget rt;
		rt.m_textureView = TextureView(presentTex.get(), TextureSubresourceDesc::all());
		const F32 col = 1.0f - F32(iterations) / F32(kIterations);
		rt.m_clearValue.m_colorf = {col, 0.0f, col, 1.0f};
		cmdb->beginRenderPass({rt});
		cmdb->endRenderPass();

		const TextureBarrierInfo barrier2 = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kRtvDsvWrite,
											 TextureUsageBit::kPresent};
		cmdb->setPipelineBarrier({&barrier2, 1}, {}, {});

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		GrManager::getSingleton().swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	commonDestroy();
}

ANKI_TEST(Gr, SimpleCompute)
{
	commonInit();

	{
		BufferInitInfo buffInit;
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_size = sizeof(Vec4);
		buffInit.m_usage = BufferUsageBit::kSrvCompute;

		BufferPtr readBuff = GrManager::getSingleton().newBuffer(buffInit);

		Vec4* inData = static_cast<Vec4*>(readBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
		const Vec4 kValue(123.456f, -666.666f, 172.2f, -16.0f);
		*inData = kValue;
		readBuff->unmap();

		buffInit.m_mapAccess = BufferMapAccessBit::kRead;
		buffInit.m_usage = BufferUsageBit::kUavCompute;

		BufferPtr writeBuff = GrManager::getSingleton().newBuffer(buffInit);

		constexpr const char* kSrc = R"(
StructuredBuffer<float4> g_in : register(t0);
RWStructuredBuffer<float4> g_out : register(u0);

[numthreads(1, 1, 1)]
void main()
{
	g_out[0] = g_in[0];
}
)";
		ShaderPtr compShader = createShader(kSrc, ShaderType::kCompute);

		ShaderProgramInitInfo progInit("Program");
		progInit.m_computeShader = compShader.get();
		ShaderProgramPtr prog = GrManager::getSingleton().newShaderProgram(progInit);

		CommandBufferInitInfo cmdbInit;
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;

		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		// Record
		cmdb->bindShaderProgram(prog.get());
		cmdb->bindSrv(0, 0, BufferView(readBuff.get()));
		cmdb->bindUav(0, 0, BufferView(writeBuff.get()));
		cmdb->dispatchCompute(1, 1, 1);
		cmdb->endRecording();

		FencePtr signalFence;
		GrManager::getSingleton().submit(cmdb.get(), {}, &signalFence);

		signalFence->clientWait(kMaxSecond);

		// Check results
		Vec4* outData = static_cast<Vec4*>(writeBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kRead));
		ANKI_TEST_EXPECT_EQ(*outData, kValue);
		writeBuff->unmap();
	}

	commonDestroy();
}

ANKI_TEST(Gr, Bindings)
{
	commonInit();

	{
		constexpr const char* kSrc = R"(
struct Foo3
{
	float x;
	float y;
	float z;
};

StructuredBuffer<float4> g_structured : register(t0);
Texture2D g_tex : register(t2);
Buffer<float4> g_buff : register(t3);

RWStructuredBuffer<Foo3> g_rwstructured : register(u0, space2);
RWTexture2D<float4> g_rwtex[3] : register(u2);
RWBuffer<float4> g_rwbuff : register(u7);

struct Foo
{
	float4 m_val;
};
ConstantBuffer<Foo> g_consts : register(b0);


struct Foo2
{
	uint4 m_val;
};
#if defined(__spirv__)
[[vk::push_constant]] ConstantBuffer<Foo2> g_consts;

[[vk::binding(0, 1000000)]] Texture2D g_bindless[];
#else
ConstantBuffer<Foo2> g_consts : register(b0, space3000);
#endif

SamplerState g_sampler : register(s2);

[numthreads(1, 1, 1)]
void main()
{
	float3 tmp = (g_structured[0] + g_structured[1]).xyz;
	Foo3 tmp3 = {tmp.x, tmp.y, tmp.z};
	g_rwstructured[0] = tmp3;
	tmp *= 2.0f;
	Foo3 tmp3_ = {tmp.x, tmp.y, tmp.z};
	g_rwstructured[1] = tmp3_;

	g_rwtex[0][uint2(0, 0)] = g_consts.m_val;

#if defined(__spirv__)
	Texture2D tex = g_bindless[g_consts.m_val.x];
#else
	Texture2D tex = ResourceDescriptorHeap[g_consts.m_val.x];
#endif
	g_rwtex[1][uint2(0, 0)] = tex.SampleLevel(g_sampler, 0.0f, 0.0f);

	g_rwtex[2][uint2(0, 0)] = g_tex.SampleLevel(g_sampler, 0.0f, 0.0f);

	g_rwbuff[0] = g_buff[0];
}
)";
		struct Foo
		{
			F32 x;
			F32 y;
			F32 z;

			Foo() = default;

			Foo(Vec4 v)
				: x(v.x())
				, y(v.y())
				, z(v.z())
			{
			}

			Bool operator==(const Foo& b) const
			{
				return x == b.x && y == b.y && z == b.z;
			}
		};

		TextureInitInfo texInit;
		texInit.m_width = texInit.m_height = 1;
		texInit.m_format = Format::kR32G32B32A32_Sfloat;

		ShaderPtr compShader = createShader(kSrc, ShaderType::kCompute);

		ShaderProgramInitInfo progInit("Program");
		progInit.m_computeShader = compShader.get();
		ShaderProgramPtr prog = GrManager::getSingleton().newShaderProgram(progInit);

		const Vec4 kMagicVec(1.0f, 2.0f, 3.0f, 4.0f);
		const Vec4 kInvalidVec(100.0f, 200.0f, 300.0f, 400.0f);

		const Array<Vec4, 2> data = {kMagicVec, kMagicVec * 2.0f};
		BufferPtr structured = createBuffer(BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv, ConstWeakArray<Vec4>(data), "structured");

		texInit.m_usage = TextureUsageBit::kSrvCompute | TextureUsageBit::kCopyDestination;
		TexturePtr tex = createTexture2d(texInit, kMagicVec * 2.0f);

		BufferPtr buff = createBuffer(BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv, kMagicVec * 2.0f, 1, "buff");
		BufferPtr rwstructured = createBuffer(BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv, Foo(kInvalidVec), 2, "rwstructured");
		BufferPtr rwbuff = createBuffer(BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv, kInvalidVec, 1, "rwbuff");

		Array<TexturePtr, 3> rwtex;

		texInit.m_usage = TextureUsageBit::kUavCompute | TextureUsageBit::kCopyDestination;
		rwtex[0] = createTexture2d(texInit, kInvalidVec);
		rwtex[1] = createTexture2d(texInit, kInvalidVec);
		rwtex[2] = createTexture2d(texInit, kInvalidVec);

		BufferPtr consts = createBuffer(BufferUsageBit::kConstantCompute, kMagicVec * 3.0f, 1, "consts");

		SamplerInitInfo samplInit;
		SamplerPtr sampler = GrManager::getSingleton().newSampler(samplInit);

		const U32 bindlessIdx = tex->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());

		// Record
		CommandBufferInitInfo cmdbInit;
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		cmdb->bindShaderProgram(prog.get());
		cmdb->bindSrv(0, 0, BufferView(structured.get()));
		cmdb->bindSrv(2, 0, TextureView(tex.get(), TextureSubresourceDesc::all()));
		cmdb->bindSrv(3, 0, BufferView(buff.get()), Format::kR32G32B32A32_Sfloat);
		cmdb->bindUav(0, 2, BufferView(rwstructured.get()));
		cmdb->bindUav(2, 0, TextureView(rwtex[0].get(), TextureSubresourceDesc::firstSurface()));
		cmdb->bindUav(3, 0, TextureView(rwtex[1].get(), TextureSubresourceDesc::firstSurface()));
		cmdb->bindUav(4, 0, TextureView(rwtex[2].get(), TextureSubresourceDesc::firstSurface()));
		cmdb->bindUav(7, 0, BufferView(rwbuff.get()), Format::kR32G32B32A32_Sfloat);
		cmdb->bindConstantBuffer(0, 0, BufferView(consts.get()));
		cmdb->bindSampler(2, 0, sampler.get());

		const UVec4 pc(bindlessIdx);
		cmdb->setFastConstants(&pc, sizeof(pc));

		cmdb->dispatchCompute(1, 1, 1);
		cmdb->endRecording();

		FencePtr signalFence;
		GrManager::getSingleton().submit(cmdb.get(), {}, &signalFence);
		signalFence->clientWait(kMaxSecond);

		// Check
		validateBuffer(rwstructured, ConstWeakArray(Array<Foo, 2>{kMagicVec + kMagicVec * 2.0f, (kMagicVec + kMagicVec * 2.0f) * 2.0f}));
		validateBuffer(rwbuff, ConstWeakArray(Array<Vec4, 1>{kMagicVec * 2.0f}));
	}

	commonDestroy();
}

ANKI_TEST(Gr, CoordinateSystem)
{
	commonInit();

	{
		constexpr const Char* kVertSrc = R"(
struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float2 m_uv : TEXCOORDS;
};

VertOut main(uint svVertexId : SV_VERTEXID)
{
	const float2 positions[6] = {float2(-1, -1), float2(1, 1), float2(-1, 1), float2(-1, -1), float2(1, -1), float2(1, 1)};
	svVertexId = min(svVertexId, 5u);

	VertOut o;
	o.m_svPosition = float4(positions[svVertexId], 0.0f, 1.0f);
	o.m_uv = positions[svVertexId].xy / 2.0f + 0.5f;
	return o;
})";

		constexpr const Char* kFragSrc = R"(
struct Viewport
{
	float4 m_viewport;
};

#if defined(__spirv__)
[[vk::push_constant]] ConstantBuffer<Viewport> g_viewport;
#else
ConstantBuffer<Viewport> g_viewport : register(b0, space3000);
#endif

Texture2D g_tex : register(t0);
SamplerState g_sampler : register(s0);

float4 main(float4 svPosition : SV_POSITION, float2 uv : TEXCOORDS, uint svPrimId : SV_PRIMITIVEID) : SV_TARGET0
{
	if(svPrimId == 1u)
	{	
		return float4(svPosition.xy / g_viewport.m_viewport.zw, 0.0f, 0.0f);
	}
	else
	{
		return g_tex.Sample(g_sampler, float2(uv.x, -uv.y) * 2.0f);
	}
})";

		ShaderProgramPtr prog = createVertFragProg(kVertSrc, kFragSrc);

		// Create a texture
		TextureInitInfo texInit;
		texInit.m_width = 2;
		texInit.m_height = 2;
		texInit.m_format = Format::kR32G32B32A32_Sfloat;
		texInit.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kAllCopy;
		TexturePtr tex = GrManager::getSingleton().newTexture(texInit);

		BufferInitInfo buffInit;
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		buffInit.m_size = sizeof(Vec4) * 4;
		buffInit.m_usage = BufferUsageBit::kCopySource;
		BufferPtr uploadBuff = GrManager::getSingleton().newBuffer(buffInit);
		void* mappedMem = uploadBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
		const Array<Vec4, 4> texelData = {Vec4(1.0f, 0.0f, 0.0f, 1.0f), Vec4(0.0f, 1.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 1.0f, 1.0f),
										  Vec4(1.0f, 0.0f, 1.0f, 1.0f)};
		memcpy(mappedMem, &texelData[0][0], sizeof(texelData));
		uploadBuff->unmap();

		CommandBufferInitInfo cmdbInit;
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);
		TextureBarrierInfo barrier = {TextureView(tex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
									  TextureUsageBit::kCopyDestination};
		cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
		cmdb->copyBufferToTexture(BufferView(uploadBuff.get()), TextureView(tex.get(), TextureSubresourceDesc::all()));
		barrier = {TextureView(tex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel};
		cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
		cmdb->endRecording();
		FencePtr fence;
		GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
		fence->clientWait(kMaxSecond);

		// Create sampler
		SamplerInitInfo samplInit;
		SamplerPtr sampler = GrManager::getSingleton().newSampler(samplInit);

		const U kIterationCount = 100;
		U iterations = kIterationCount;
		const Vec4 viewport(10.0f, 10.0f, F32(NativeWindow::getSingleton().getWidth() - 20), F32(NativeWindow::getSingleton().getHeight() - 30));
		while(iterations--)
		{
			HighRezTimer timer;
			timer.start();

			TexturePtr presentTex = GrManager::getSingleton().acquireNextPresentableTexture();

			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kGeneralWork;
			CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cinit);

			cmdb->setViewport(U32(viewport.x()), U32(viewport.y()), U32(viewport.z()), U32(viewport.w()));
			cmdb->bindShaderProgram(prog.get());

			const TextureBarrierInfo barrier = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
												TextureUsageBit::kRtvDsvWrite};
			cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

			cmdb->beginRenderPass({TextureView(presentTex.get(), TextureSubresourceDesc::firstSurface())});

			cmdb->setFastConstants(&viewport, sizeof(viewport));

			cmdb->bindSrv(0, 0, TextureView(tex.get(), TextureSubresourceDesc::all()));
			cmdb->bindSampler(0, 0, sampler.get());

			cmdb->draw(PrimitiveTopology::kTriangles, 6);
			cmdb->endRenderPass();

			const TextureBarrierInfo barrier2 = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kRtvDsvWrite,
												 TextureUsageBit::kPresent};
			cmdb->setPipelineBarrier({&barrier2, 1}, {}, {});

			cmdb->endRecording();
			GrManager::getSingleton().submit(cmdb.get());

			GrManager::getSingleton().swapBuffers();

			timer.stop();
			const Second kTick = 1.0f / 30.0f;
			if(timer.getElapsedTime() < kTick)
			{
				HighRezTimer::sleep(kTick - timer.getElapsedTime());
			}
		}
	}

	commonDestroy();
}

ANKI_TEST(Gr, ViewportAndScissor)
{
#if 0
	COMMON_BEGIN()

	ANKI_TEST_LOGI("Expect to see a grey quad appearing in the 4 corners. The clear color will change and affect only"
				   "the area around the quad");
	ShaderProgramPtr prog = createProgram(VERT_QUAD_STRIP_SRC, FRAG_SRC, *gr);

	srand(time(nullptr));
	Array<FramebufferPtr, 4> fb;
	for(FramebufferPtr& f : fb)
	{
		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentCount = 1;
		fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{randFloat(1.0), randFloat(1.0), randFloat(1.0), 1.0}};

		f = gr->newFramebuffer(fbinit);
	}

	static const Array2d<U, 4, 4> VIEWPORTS = {{{{0, 0, WIDTH / 2, HEIGHT / 2}},
		{{WIDTH / 2, 0, WIDTH / 2, HEIGHT / 2}},
		{{WIDTH / 2, HEIGHT / 2, WIDTH / 2, HEIGHT / 2}},
		{{0, HEIGHT / 2, WIDTH / 2, HEIGHT / 2}}}};

	const U ITERATIONS = 400;
	const U SCISSOR_MARGIN = 20;
	const U RENDER_AREA_MARGIN = 10;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		U idx = (i / 30) % 4;
		auto vp = VIEWPORTS[idx];
		cmdb->setViewport(vp[0], vp[1], vp[2], vp[3]);
		cmdb->setScissor(
			vp[0] + SCISSOR_MARGIN, vp[1] + SCISSOR_MARGIN, vp[2] - SCISSOR_MARGIN * 2, vp[3] - SCISSOR_MARGIN * 2);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb[i % 4],
			{},
			{},
			vp[0] + RENDER_AREA_MARGIN,
			vp[1] + RENDER_AREA_MARGIN,
			vp[2] - RENDER_AREA_MARGIN * 2,
			vp[3] - RENDER_AREA_MARGIN * 2);
		cmdb->draw(PrimitiveTopology::TRIANGLE_STRIP, 4);
		cmdb->endRenderPass();
		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
#endif
}

ANKI_TEST(Gr, ViewportAndScissorOffscreen)
{
#if 0
	srand(U32(time(nullptr)));
	COMMON_BEGIN()

	constexpr const char* kFragSrc = R"(layout (location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(0.5);
})";

	ANKI_TEST_LOGI("Expect to see a grey quad appearing in the 4 corners. "
				   "Around that quad is a border that changes color. "
				   "The quads appear counter-clockwise");
	ShaderProgramPtr prog = createProgram(VERT_QUAD_STRIP_SRC, kFragSrc, *g_gr);
	ShaderProgramPtr blitProg = createProgram(VERT_QUAD_SRC, FRAG_TEX_SRC, *g_gr);

	const Format COL_FORMAT = Format::kR8G8B8A8_Unorm;
	const U RT_WIDTH = 32;
	const U RT_HEIGHT = 16;
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = COL_FORMAT;
	init.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kAllRtvDsv;
	init.m_height = RT_HEIGHT;
	init.m_width = RT_WIDTH;
	init.m_mipmapCount = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_type = TextureType::k2D;
	TexturePtr rt = g_gr->newTexture(init);

	TextureViewInitInfo viewInit(rt.get());
	TextureViewPtr texView = g_gr->newTextureView(viewInit);

	Array<FramebufferPtr, 4> fb;
	for(FramebufferPtr& f : fb)
	{
		TextureViewInitInfo viewInf(rt.get());
		TextureViewPtr view = g_gr->newTextureView(viewInf);

		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentCount = 1;
		fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {
			{getRandomRange(0.0f, 1.0f), getRandomRange(0.0f, 1.0f), getRandomRange(0.0f, 1.0f), 1.0}};
		fbinit.m_colorAttachments[0].m_textureView = view;

		f = g_gr->newFramebuffer(fbinit);
	}

	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kBase;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	static const Array2d<U32, 4, 4> VIEWPORTS = {{{{0, 0, RT_WIDTH / 2, RT_HEIGHT / 2}},
												  {{RT_WIDTH / 2, 0, RT_WIDTH / 2, RT_HEIGHT / 2}},
												  {{RT_WIDTH / 2, RT_HEIGHT / 2, RT_WIDTH / 2, RT_HEIGHT / 2}},
												  {{0, RT_HEIGHT / 2, RT_WIDTH / 2, RT_HEIGHT / 2}}}};

	const U32 ITERATIONS = 400;
	const U32 SCISSOR_MARGIN = 2;
	const U32 RENDER_AREA_MARGIN = 1;
	for(U32 i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr dfb = createColorFb(*g_gr, presentTex);

		if(i == 0)
		{
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
			CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

			cmdb->setViewport(0, 0, RT_WIDTH, RT_HEIGHT);
			setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kNone, TextureUsageBit::kRtvDsvWrite, TextureSurfaceDescriptor(0, 0, 0, 0));
			cmdb->beginRenderPass(fb[0].get(), {{TextureUsageBit::kRtvDsvWrite}}, {});
			cmdb->endRenderPass();
			setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));
			cmdb->endRecording();
			GrManager::getSingleton().submit(cmdb.get());
		}

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		// Draw offscreen
		setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kSrvPixel, TextureUsageBit::kRtvDsvWrite, TextureSurfaceDescriptor(0, 0, 0, 0));
		auto vp = VIEWPORTS[(i / 30) % 4];
		cmdb->setViewport(vp[0], vp[1], vp[2], vp[3]);
		cmdb->setScissor(vp[0] + SCISSOR_MARGIN, vp[1] + SCISSOR_MARGIN, vp[2] - SCISSOR_MARGIN * 2, vp[3] - SCISSOR_MARGIN * 2);
		cmdb->bindShaderProgram(prog.get());
		cmdb->beginRenderPass(fb[i % 4].get(), {{TextureUsageBit::kRtvDsvWrite}}, {}, vp[0] + RENDER_AREA_MARGIN, vp[1] + RENDER_AREA_MARGIN,
							  vp[2] - RENDER_AREA_MARGIN * 2, vp[3] - RENDER_AREA_MARGIN * 2);
		cmdb->draw(PrimitiveTopology::kTriangleStrip, 4);
		cmdb->endRenderPass();

		// Draw onscreen
		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->setScissor(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->bindShaderProgram(blitProg.get());
		setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));
		// cmdb->bindTextureAndSampler(0, 0, texView.get(), sampler.get());
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb.get(), {TextureUsageBit::kRtvDsvWrite}, {});
		cmdb->draw(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
#endif
}

ANKI_TEST(Gr, Buffer)
{
#if 0
	COMMON_BEGIN()

	BufferInitInfo buffInit("a");
	buffInit.m_size = 512;
	buffInit.m_usage = BufferUsageBit::kAllConstant;
	buffInit.m_mapAccess = BufferMapAccessBit::kNone;
	BufferPtr a = g_gr->newBuffer(buffInit);

	buffInit.setName("b");
	buffInit.m_size = 64;
	buffInit.m_usage = BufferUsageBit::kAllStorage;
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite | BufferMapAccessBit::kRead;
	BufferPtr b = g_gr->newBuffer(buffInit);

	void* ptr = b->map(0, 64, BufferMapAccessBit::kWrite);
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
	U8 ptr2[64];
	memset(ptr, 0xCC, 64);
	memset(ptr2, 0xCC, 64);
	b->unmap();

	ptr = b->map(0, 64, BufferMapAccessBit::kRead);
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
	ANKI_TEST_EXPECT_EQ(memcmp(ptr, ptr2, 64), 0);
	b->unmap();

	COMMON_END()
#endif
}

ANKI_TEST(Gr, SimpleDrawcall)
{
	commonInit();

	{
		// Prog
		constexpr const char* kUboVert = R"(
struct A
{
	float4 m_color[3];
	float4 m_rotation2d;
};

#if defined(__spirv__)
[[vk::push_constant]] ConstantBuffer<A> g_consts;
#else
ConstantBuffer<A> g_consts : register(b0, space3000);
#endif

struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float3 m_color : COLOR;
};

VertOut main(uint svVertexId : SV_VERTEXID)
{
	VertOut o;

	// No dynamic branching with push constants
	float4 color;
	switch(svVertexId)
	{
	case 0:
		color = g_consts.m_color[0];
		break;
	case 1:
		color = g_consts.m_color[1];
		break;
	default:
		color = g_consts.m_color[2];
	};

	o.m_color = color.xyz;

	const float2 kPositions[3] = {float2(-1.0, 1.0), float2(0.0, -1.0), float2(1.0, 1.0)};

	float2x2 rot = float2x2(
		g_consts.m_rotation2d.x, g_consts.m_rotation2d.y, g_consts.m_rotation2d.z, g_consts.m_rotation2d.w);
	float2 pos = mul(rot, kPositions[svVertexId % 3]);

	o.m_svPosition = float4(pos, 0.0, 1.0);

	return o;
})";

		constexpr const char* kUboFrag = R"(
struct VertOut
{
	float4 m_svPosition : SV_POSITION;
	float3 m_color : COLOR;
};

float4 main(VertOut i) : SV_TARGET0
{
	return float4(i.m_color, 1.0);
})";

		ShaderProgramPtr prog = createVertFragProg(kUboVert, kUboFrag);

		const U kIterationCount = 100;
		U iterations = kIterationCount;
		while(iterations--)
		{
			HighRezTimer timer;
			timer.start();

			TexturePtr presentTex = GrManager::getSingleton().acquireNextPresentableTexture();

			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kGeneralWork;
			CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cinit);

			cmdb->setViewport(0, 0, NativeWindow::getSingleton().getWidth(), NativeWindow::getSingleton().getHeight());
			cmdb->bindShaderProgram(prog.get());

			const TextureBarrierInfo barrier = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kNone,
												TextureUsageBit::kRtvDsvWrite};
			cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

			cmdb->pushDebugMarker("AnKi", Vec3(1.0f, 0.0f, 0.0f));

			cmdb->beginRenderPass({TextureView(presentTex.get(), TextureSubresourceDesc::firstSurface())});

			// Set uniforms
			class A
			{
			public:
				Array<Vec4, 3> m_color;
				Vec4 m_rotation2d;
			} pc;

			const F32 angle = toRad(360.0f / F32(kIterationCount) * F32(iterations)) / 2.0f;
			pc.m_rotation2d[0] = cos(angle);
			pc.m_rotation2d[1] = -sin(angle);
			pc.m_rotation2d[2] = sin(angle);
			pc.m_rotation2d[3] = cos(angle);

			pc.m_color[0] = Vec4(1.0, 0.0, 0.0, 0.0);
			pc.m_color[1] = Vec4(0.0, 1.0, 0.0, 0.0);
			pc.m_color[2] = Vec4(0.0, 0.0, 1.0, 0.0);

			cmdb->setFastConstants(&pc, sizeof(pc));

			cmdb->draw(PrimitiveTopology::kTriangles, 3);
			cmdb->endRenderPass();

			const TextureBarrierInfo barrier2 = {TextureView(presentTex.get(), TextureSubresourceDesc::all()), TextureUsageBit::kRtvDsvWrite,
												 TextureUsageBit::kPresent};
			cmdb->setPipelineBarrier({&barrier2, 1}, {}, {});

			cmdb->popDebugMarker();

			cmdb->endRecording();
			GrManager::getSingleton().submit(cmdb.get());

			GrManager::getSingleton().swapBuffers();

			timer.stop();
			const Second kTick = 1.0f / 30.0f;
			if(timer.getElapsedTime() < kTick)
			{
				HighRezTimer::sleep(kTick - timer.getElapsedTime());
			}
		}
	}

	commonDestroy();
}

ANKI_TEST(Gr, DrawWithVertex)
{
#if 0
	COMMON_BEGIN()

	// The buffers
	struct Vert
	{
		Vec3 m_pos;
		Array<U8, 4> m_color;
	};
	static_assert(sizeof(Vert) == sizeof(Vec4), "See file");

	BufferPtr b = g_gr->newBuffer(BufferInitInfo(sizeof(Vert) * 3, BufferUsageBit::kVertex, BufferMapAccessBit::kWrite));

	Vert* ptr = static_cast<Vert*>(b->map(0, sizeof(Vert) * 3, BufferMapAccessBit::kWrite));
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

	ptr[0].m_pos = Vec3(-1.0, 1.0, 0.0);
	ptr[1].m_pos = Vec3(0.0, -1.0, 0.0);
	ptr[2].m_pos = Vec3(1.0, 1.0, 0.0);

	ptr[0].m_color = {{255, 0, 0}};
	ptr[1].m_color = {{0, 255, 0}};
	ptr[2].m_color = {{0, 0, 255}};
	b->unmap();

	BufferPtr c = g_gr->newBuffer(BufferInitInfo(sizeof(Vec3) * 3, BufferUsageBit::kVertex, BufferMapAccessBit::kWrite));

	Vec3* otherColor = static_cast<Vec3*>(c->map(0, sizeof(Vec3) * 3, BufferMapAccessBit::kWrite));

	otherColor[0] = Vec3(0.0, 1.0, 1.0);
	otherColor[1] = Vec3(1.0, 0.0, 1.0);
	otherColor[2] = Vec3(1.0, 1.0, 0.0);
	c->unmap();

	// Prog
	ShaderProgramPtr prog = createProgram(VERT_INP_SRC, FRAG_INP_SRC, *g_gr);

	U iterations = 100;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->bindVertexBuffer(0, b.get(), 0, sizeof(Vert));
		cmdb->bindVertexBuffer(1, c.get(), 0, sizeof(Vec3));
		cmdb->setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
		cmdb->setVertexAttribute(VertexAttributeSemantic::kColor, 0, Format::kR8G8B8_Unorm, sizeof(Vec3));
		cmdb->setVertexAttribute(VertexAttributeSemantic::kMisc0, 1, Format::kR32G32B32_Sfloat, 0);

		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->setPolygonOffset(0.0, 0.0);
		cmdb->bindShaderProgram(prog.get());
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb.get(), {TextureUsageBit::kRtvDsvWrite}, {});
		cmdb->draw(PrimitiveTopology::kTriangles, 3);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
#endif
}

ANKI_TEST(Gr, Sampler)
{
#if 0
	COMMON_BEGIN()

	SamplerInitInfo init;

	SamplerPtr b = g_gr->newSampler(init);

	COMMON_END()
#endif
}

ANKI_TEST(Gr, Texture)
{
#if 0
	COMMON_BEGIN()

	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::kR8G8B8_Unorm;
	init.m_usage = TextureUsageBit::kSrvPixel;
	init.m_height = 4;
	init.m_width = 4;
	init.m_mipmapCount = 2;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_type = TextureType::k2D;

	TexturePtr b = g_gr->newTexture(init);

	COMMON_END()
#endif
}

ANKI_TEST(Gr, DrawWithTexture)
{
#if 0
	COMMON_BEGIN()

	//
	// Create sampler
	//
	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kLinear;
	samplerInit.m_addressing = SamplingAddressing::kClamp;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	//
	// Create texture A
	//
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::kR8G8B8_Unorm;
	init.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kCopyDestination;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapCount = 2;
	init.m_samples = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::k2D;

	TexturePtr a = g_gr->newTexture(init);

	TextureViewPtr aView = g_gr->newTextureView(TextureViewInitInfo(a.get()));

	//
	// Create texture B
	//
	init.m_width = 4;
	init.m_height = 4;
	init.m_mipmapCount = 3;
	init.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kCopyDestination | TextureUsageBit::kGenerateMipmaps;

	TexturePtr b = g_gr->newTexture(init);

	TextureViewPtr bView = g_gr->newTextureView(TextureViewInitInfo(b.get()));

	//
	// Upload all textures
	//
	Array<U8, 2 * 2 * 3> mip0 = {{255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255}};

	Array<U8, 3> mip1 = {{128, 128, 128}};

	Array<U8, 4 * 4 * 3> bmip0 = {{255, 0, 0, 0, 255, 0,   0,   0, 255, 255, 255, 0, 255, 0,   255, 0,   255, 255, 255, 255, 255, 128, 0,  0, 0,
								   128, 0, 0, 0, 128, 128, 128, 0, 128, 0,   128, 0, 128, 128, 128, 128, 128, 255, 128, 0,   0,   128, 255}};

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbinit);

	// Set barriers
	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kSrvPixel, TextureUsageBit::kCopyDestination, TextureSurfaceDescriptor(0, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kSrvPixel, TextureUsageBit::kCopyDestination, TextureSurfaceDescriptor(1, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, b, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination, TextureSurfaceDescriptor(0, 0, 0, 0));

	TransferGpuAllocatorHandle handle0, handle1, handle2;
	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceDescriptor(0, 0, 0, 0), &mip0[0], sizeof(mip0), handle0);

	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceDescriptor(1, 0, 0, 0), &mip1[0], sizeof(mip1), handle1);

	UPLOAD_TEX_SURFACE(cmdb, b, TextureSurfaceDescriptor(0, 0, 0, 0), &bmip0[0], sizeof(bmip0), handle2);

	// Gen mips
	setTextureSurfaceBarrier(cmdb, b, TextureUsageBit::kCopyDestination, TextureUsageBit::kGenerateMipmaps, TextureSurfaceDescriptor(0, 0, 0, 0));

	cmdb->generateMipmaps2d(g_gr->newTextureView(TextureViewInitInfo(b.get())).get());

	// Set barriers
	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(1, 0, 0, 0));

	for(U32 i = 0; i < 3; ++i)
	{
		setTextureSurfaceBarrier(cmdb, b, TextureUsageBit::kGenerateMipmaps, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(i, 0, 0, 0));
	}

	FencePtr fence;
	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
	transfAlloc->release(handle0, fence);
	transfAlloc->release(handle1, fence);
	transfAlloc->release(handle2, fence);

	//
	// Create prog
	//
	static const char* FRAG_2TEX_SRC = R"(layout (location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(set = 0, binding = 0) uniform sampler2D u_tex0;
layout(set = 0, binding = 1) uniform sampler2D u_tex1;

layout(push_constant) uniform b_pc
{
	Vec4 u_viewport;
};

void main()
{
	if(gl_FragCoord.x < u_viewport.x / 2.0)
	{
		if(gl_FragCoord.y < u_viewport.y / 2.0)
		{
			vec2 uv = in_uv * 2.0;
			out_color = textureLod(u_tex0, uv, 0.0);
		}
		else
		{
			vec2 uv = in_uv * 2.0 - vec2(0.0, 1.0);
			out_color = textureLod(u_tex0, uv, 1.0);
		}
	}
	else
	{
		if(gl_FragCoord.y < u_viewport.y / 2.0)
		{
			vec2 uv = in_uv * 2.0 - vec2(1.0, 0.0);
			out_color = textureLod(u_tex1, uv, 0.0);
		}
		else
		{
			vec2 uv = in_uv * 2.0 - vec2(1.0, 1.0);
			out_color = textureLod(u_tex1, uv, 1.0);
		}
	}
})";
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_2TEX_SRC, *g_gr);

	//
	// Draw
	//
	const U ITERATION_COUNT = 200;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->bindShaderProgram(prog.get());
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb.get(), {TextureUsageBit::kRtvDsvWrite}, {});

		Vec4 pc(F32(g_win->getWidth()), F32(g_win->getHeight()), 0.0f, 0.0f);
		cmdb->setFastConstants(&pc, sizeof(pc));

		// cmdb->bindTextureAndSampler(0, 0, aView.get(), sampler.get());
		// cmdb->bindTextureAndSampler(0, 1, bView.get(), sampler.get());
		cmdb->draw(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
#endif
}

#if 0
static void drawOffscreenDrawcalls([[maybe_unused]] GrManager& gr, ShaderProgramPtr prog, CommandBufferPtr cmdb, U32 viewPortSize,
								   BufferPtr indexBuff, BufferPtr vertBuff)
{
	static F32 ang = -2.5f;
	ang += toRad(2.5f);

	Mat4 viewMat(Vec3(0.0, 0.0, 5.0), Mat3::getIdentity(), Vec3(1.0f));
	viewMat.invert();

	Mat4 projMat = Mat4::calculatePerspectiveProjectionMatrix(toRad(60.0f), toRad(60.0f), 0.1f, 100.0f);

	Mat4 modelMat(Vec3(-0.5, -0.5, 0.0), Mat3(Euler(ang, ang / 2.0f, ang / 3.0f)), Vec3(1.0f));

	Mat4* mvp = SET_UNIFORMS(Mat4*, sizeof(*mvp), cmdb, 0, 0);
	*mvp = projMat * viewMat * modelMat;

	Vec4* color = SET_UNIFORMS(Vec4*, sizeof(*color) * 2, cmdb, 0, 1);
	*color++ = Vec4(1.0, 0.0, 0.0, 0.0);
	*color = Vec4(0.0, 1.0, 0.0, 0.0);

	cmdb->bindVertexBuffer(0, BufferView(vertBuff.get()), sizeof(Vec3));
	cmdb->setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
	cmdb->bindShaderProgram(prog.get());
	cmdb->bindIndexBuffer(BufferView(indexBuff.get()), IndexType::kU16);
	cmdb->setViewport(0, 0, viewPortSize, viewPortSize);
	cmdb->drawIndexed(PrimitiveTopology::kTriangles, 6 * 2 * 3);

	// 2nd draw
	modelMat = Mat4(Vec3(0.5, 0.5, 0.0), Mat3(Euler(ang * 2.0f, ang, ang / 3.0f * 2.0f)), Vec3(1.0f));

	mvp = SET_UNIFORMS(Mat4*, sizeof(*mvp), cmdb, 0, 0);
	*mvp = projMat * viewMat * modelMat;

	color = SET_UNIFORMS(Vec4*, sizeof(*color) * 2, cmdb, 0, 1);
	*color++ = Vec4(0.0, 0.0, 1.0, 0.0);
	*color = Vec4(0.0, 1.0, 1.0, 0.0);

	cmdb->drawIndexed(PrimitiveTopology::kTriangles, 6 * 2 * 3);
}
#endif

static void drawOffscreen(GrManager& gr)
{
#if 0
	//
	// Create textures
	//
	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::kLinear;
	samplerInit.m_mipmapFilter = SamplingFilter::kLinear;
	SamplerPtr sampler = gr.newSampler(samplerInit);

	const Format COL_FORMAT = Format::kR8G8B8A8_Unorm;
	const U TEX_SIZE = 256;

	TextureInitInfo init;
	init.m_format = COL_FORMAT;
	init.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kAllRtvDsv;
	init.m_height = TEX_SIZE;
	init.m_width = TEX_SIZE;
	init.m_type = TextureType::k2D;

	TexturePtr col0 = gr.newTexture(init);
	TexturePtr col1 = gr.newTexture(init);

	TextureViewPtr col0View = gr.newTextureView(TextureViewInitInfo(col0.get()));
	TextureViewPtr col1View = gr.newTextureView(TextureViewInitInfo(col1.get()));

	init.m_format = kDsFormat;
	TexturePtr dp = gr.newTexture(init);

	//
	// Create FB
	//
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 2;
	fbinit.m_colorAttachments[0].m_textureView = gr.newTextureView(TextureViewInitInfo(col0.get()));
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{0.1f, 0.0f, 0.0f, 0.0f}};
	fbinit.m_colorAttachments[1].m_textureView = gr.newTextureView(TextureViewInitInfo(col1.get()));
	fbinit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0f, 0.1f, 0.0f, 0.0f}};
	TextureViewInitInfo viewInit(dp.get());
	viewInit.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	fbinit.m_depthStencilAttachment.m_textureView = gr.newTextureView(viewInit);
	fbinit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	FramebufferPtr fb = gr.newFramebuffer(fbinit);

	//
	// Create buffs
	//
	BufferPtr verts, indices;
	createCube(gr, verts, indices);

	//
	// Create progs
	//
	ShaderProgramPtr prog = createProgram(VERT_MRT_SRC, FRAG_MRT_SRC, gr);
	ShaderProgramPtr resolveProg = createProgram(VERT_QUAD_SRC, FRAG_MRT2_SRC, gr);

	//
	// Draw
	//
	const U ITERATION_COUNT = 200;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = gr.newCommandBuffer(cinit);

		cmdb->setPolygonOffset(0.0, 0.0);

		setTextureSurfaceBarrier(cmdb, col0, TextureUsageBit::kNone, TextureUsageBit::kRtvDsvWrite, TextureSurfaceDescriptor(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, col1, TextureUsageBit::kNone, TextureUsageBit::kRtvDsvWrite, TextureSurfaceDescriptor(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, dp, TextureUsageBit::kNone, TextureUsageBit::kAllRtvDsv, TextureSurfaceDescriptor(0, 0, 0, 0));
		cmdb->beginRenderPass(fb.get(), {{TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kRtvDsvWrite}}, TextureUsageBit::kAllRtvDsv);

		drawOffscreenDrawcalls(gr, prog, cmdb, TEX_SIZE, indices, verts);

		cmdb->endRenderPass();

		setTextureSurfaceBarrier(cmdb, col0, TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, col1, TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, dp, TextureUsageBit::kAllRtvDsv, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));

		// Draw quad
		TexturePtr presentTex = gr.acquireNextPresentableTexture();
		FramebufferPtr dfb = createColorFb(gr, presentTex);

		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb.get(), {TextureUsageBit::kRtvDsvWrite}, {});
		cmdb->bindShaderProgram(resolveProg.get());
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		// cmdb->bindTextureAndSampler(0, 0, col0View.get(), sampler.get());
		// cmdb->bindTextureAndSampler(0, 2, col1View.get(), sampler.get());
		cmdb->draw(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		// End
		gr.swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}
#endif
}

ANKI_TEST(Gr, DrawOffscreen)
{
#if 0
	COMMON_BEGIN()

	drawOffscreen(*g_gr);

	COMMON_END();
#endif
}

ANKI_TEST(Gr, ImageLoadStore)
{
#if 0
	COMMON_BEGIN()

	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kBase;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	TextureInitInfo init;
	init.m_width = init.m_height = 4;
	init.m_mipmapCount = 2;
	init.m_usage = TextureUsageBit::kCopyDestination | TextureUsageBit::kAllSrv | TextureUsageBit::kUavCompute;
	init.m_type = TextureType::k2D;
	init.m_format = Format::kR8G8B8A8_Unorm;

	TexturePtr tex = g_gr->newTexture(init);

	TextureViewInitInfo viewInit(tex.get());
	viewInit.m_firstMipmap = 1;
	viewInit.m_mipmapCount = 1;
	TextureViewPtr view = g_gr->newTextureView(viewInit);

	// Prog
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_SIMPLE_TEX_SRC, *g_gr);

	// Create shader & compute prog
	ShaderPtr shader = createShader(COMP_WRITE_IMAGE_SRC, ShaderType::kCompute, *g_gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_computeShader = shader.get();
	ShaderProgramPtr compProg = g_gr->newShaderProgram(sprogInit);

	// Write texture data
	CommandBufferInitInfo cmdbinit;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbinit);

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination, TextureSurfaceDescriptor(0, 0, 0, 0));

	ClearValue clear;
	clear.m_colorf = {{0.0, 1.0, 0.0, 1.0}};
	TextureViewInitInfo viewInit2(tex.get(), TextureSurfaceDescriptor(0, 0, 0, 0));
	cmdb->clearTextureView(g_gr->newTextureView(viewInit2).get(), clear);

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(0, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination, TextureSurfaceDescriptor(1, 0, 0, 0));

	clear.m_colorf = {{0.0, 0.0, 1.0, 1.0}};
	TextureViewInitInfo viewInit3(tex.get(), TextureSurfaceDescriptor(1, 0, 0, 0));
	cmdb->clearTextureView(g_gr->newTextureView(viewInit3).get(), clear);

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kCopyDestination, TextureUsageBit::kUavCompute, TextureSurfaceDescriptor(1, 0, 0, 0));

	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get());

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		// Write image
		Vec4* col = SET_STORAGE(Vec4*, sizeof(*col), cmdb, 1, 0);
		*col = Vec4(F32(iterations) / F32(ITERATION_COUNT));

		setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kUavCompute, TextureSurfaceDescriptor(1, 0, 0, 0));
		cmdb->bindShaderProgram(compProg.get());
		cmdb->bindStorageTexture(0, 0, view.get());
		cmdb->dispatchCompute(WIDTH / 2, HEIGHT / 2, 1);
		setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kUavCompute, TextureUsageBit::kSrvPixel, TextureSurfaceDescriptor(1, 0, 0, 0));

		// Present image
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);

		cmdb->bindShaderProgram(prog.get());
		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr dfb = createColorFb(*g_gr, presentTex);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb.get(), {TextureUsageBit::kRtvDsvWrite}, {});
		// cmdb->bindTextureAndSampler(0, 0, g_gr->newTextureView(TextureViewInitInfo(tex.get())).get(), sampler.get());
		cmdb->draw(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		// End
		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
#endif
}

ANKI_TEST(Gr, 3DTextures)
{
#if 0
	COMMON_BEGIN()

	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kBase;
	samplerInit.m_addressing = SamplingAddressing::kClamp;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	//
	// Create texture A
	//
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::kR8G8B8A8_Unorm;
	init.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kCopyDestination;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapCount = 2;
	init.m_samples = 1;
	init.m_depth = 2;
	init.m_layerCount = 1;
	init.m_type = TextureType::k3D;

	TexturePtr a = g_gr->newTexture(init);

	//
	// Upload all textures
	//
	Array<U8, 2 * 2 * 2 * 4> mip0 = {
		{255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 0, 0, 255, 0, 255, 0, 0, 255, 255, 0, 255, 255, 255, 0, 0, 0, 0, 0}};

	Array<U8, 4> mip1 = {{128, 128, 128, 0}};

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbinit);

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination, TextureVolumeDescriptor(0));

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination, TextureVolumeDescriptor(1));

	TransferGpuAllocatorHandle handle0, handle1;
	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeDescriptor(0), &mip0[0], sizeof(mip0), handle0);

	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeDescriptor(1), &mip1[0], sizeof(mip1), handle1);

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel, TextureVolumeDescriptor(0));

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel, TextureVolumeDescriptor(1));

	FencePtr fence;
	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
	transfAlloc->release(handle0, fence);
	transfAlloc->release(handle1, fence);

	//
	// Rest
	//
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_TEX3D_SRC, *g_gr);

	static Array<Vec4, 9> TEX_COORDS_LOD = {{Vec4(0, 0, 0, 0), Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(1, 1, 0, 0), Vec4(0, 0, 1, 0),
											 Vec4(1, 0, 1, 0), Vec4(0, 1, 1, 0), Vec4(1, 1, 1, 0), Vec4(0, 0, 0, 1)}};

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr dfb = createColorFb(*g_gr, presentTex);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb.get(), {TextureUsageBit::kRtvDsvWrite}, {});

		cmdb->bindShaderProgram(prog.get());

		Vec4* uv = SET_UNIFORMS(Vec4*, sizeof(Vec4), cmdb, 0, 0);

		U32 idx = U32((F32(ITERATION_COUNT - iterations - 1) / F32(ITERATION_COUNT)) * F32(TEX_COORDS_LOD.getSize()));
		*uv = TEX_COORDS_LOD[idx];

		// cmdb->bindTextureAndSampler(0, 1, g_gr->newTextureView(TextureViewInitInfo(a.get())).get(), sampler.get());
		cmdb->draw(PrimitiveTopology::kTriangles, 6);

		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		// End
		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 15.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
#endif
}

static RenderTargetDesc newRTDescr(CString name)
{
	RenderTargetDesc texInf(name);
	texInf.m_width = texInf.m_height = 16;
	texInf.m_usage = TextureUsageBit::kRtvDsvWrite | TextureUsageBit::kSrvPixel;
	texInf.m_format = Format::kR8G8B8A8_Unorm;
	texInf.bake();
	return texInf;
}

ANKI_TEST(Gr, RenderGraph)
{
#if 0
	COMMON_BEGIN()

	StackMemoryPool pool(allocAligned, nullptr, 2_MB);
	RenderGraphBuilder descr(&pool);
	RenderGraphPtr rgraph = g_gr->newRenderGraph();

	const U GI_MIP_COUNT = 4;

	TextureInitInfo texI("dummy");
	texI.m_width = texI.m_height = 16;
	texI.m_usage = TextureUsageBit::kRtvDsvWrite | TextureUsageBit::kSrvPixel;
	texI.m_format = Format::kR8G8B8A8_Unorm;
	TexturePtr dummyTex = g_gr->newTexture(texI);

	// SM
	RenderTargetHandle smScratchRt = descr.newRenderTarget(newRTDescr("SM scratch"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("SM");
		pass.newTextureDependency(smScratchRt, TextureUsageBit::kAllRtvDsv);
	}

	// SM to exponential SM
	RenderTargetHandle smExpRt = descr.importRenderTarget(dummyTex.get(), TextureUsageBit::kSrvPixel);
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("ESM");
		pass.newTextureDependency(smScratchRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(smExpRt, TextureUsageBit::kRtvDsvWrite);
	}

	// GI gbuff
	RenderTargetHandle giGbuffNormRt = descr.newRenderTarget(newRTDescr("GI GBuff norm"));
	RenderTargetHandle giGbuffDiffRt = descr.newRenderTarget(newRTDescr("GI GBuff diff"));
	RenderTargetHandle giGbuffDepthRt = descr.newRenderTarget(newRTDescr("GI GBuff depth"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("GI gbuff");
		pass.newTextureDependency(giGbuffNormRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(giGbuffDepthRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(giGbuffDiffRt, TextureUsageBit::kRtvDsvWrite);
	}

	// GI light
	RenderTargetHandle giGiLightRt = descr.importRenderTarget(dummyTex.get(), TextureUsageBit::kSrvPixel);
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		TextureSubresourceInfo subresource(TextureSurfaceDescriptor(0, faceIdx, 0));

		GraphicsRenderPass& pass = descr.newGraphicsRenderPass(String().sprintf("GI lp%u", faceIdx).toCString());
		pass.newTextureDependency(giGiLightRt, TextureUsageBit::kRtvDsvWrite, subresource);
		pass.newTextureDependency(giGbuffNormRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(giGbuffDepthRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(giGbuffDiffRt, TextureUsageBit::kSrvPixel);
	}

	// GI light mips
	{
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPass& pass = descr.newGraphicsRenderPass(String().sprintf("GI mip%u", faceIdx).toCString());

			for(U32 mip = 0; mip < GI_MIP_COUNT; ++mip)
			{
				TextureSurfaceDescriptor surf(mip, faceIdx, 0);
				pass.newTextureDependency(giGiLightRt, TextureUsageBit::kGenerateMipmaps, surf);
			}
		}
	}

	// Gbuffer
	RenderTargetHandle gbuffRt0 = descr.newRenderTarget(newRTDescr("GBuff RT0"));
	RenderTargetHandle gbuffRt1 = descr.newRenderTarget(newRTDescr("GBuff RT1"));
	RenderTargetHandle gbuffRt2 = descr.newRenderTarget(newRTDescr("GBuff RT2"));
	RenderTargetHandle gbuffDepth = descr.newRenderTarget(newRTDescr("GBuff RT2"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("G-Buffer");
		pass.newTextureDependency(gbuffRt0, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(gbuffRt1, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(gbuffRt2, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(gbuffDepth, TextureUsageBit::kRtvDsvWrite);
	}

	// Half depth
	RenderTargetHandle halfDepthRt = descr.newRenderTarget(newRTDescr("Depth/2"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("HalfDepth");
		pass.newTextureDependency(gbuffDepth, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(halfDepthRt, TextureUsageBit::kRtvDsvWrite);
	}

	// Quarter depth
	RenderTargetHandle quarterDepthRt = descr.newRenderTarget(newRTDescr("Depth/4"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("QuarterDepth");
		pass.newTextureDependency(quarterDepthRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(halfDepthRt, TextureUsageBit::kSrvPixel);
	}

	// SSAO
	RenderTargetHandle ssaoRt = descr.newRenderTarget(newRTDescr("SSAO"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("SSAO main");
		pass.newTextureDependency(ssaoRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(quarterDepthRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(gbuffRt2, TextureUsageBit::kSrvPixel);

		RenderTargetHandle ssaoVBlurRt = descr.newRenderTarget(newRTDescr("SSAO tmp"));
		GraphicsRenderPass& pass2 = descr.newGraphicsRenderPass("SSAO vblur");
		pass2.newTextureDependency(ssaoRt, TextureUsageBit::kSrvPixel);
		pass2.newTextureDependency(ssaoVBlurRt, TextureUsageBit::kRtvDsvWrite);

		GraphicsRenderPass& pass3 = descr.newGraphicsRenderPass("SSAO hblur");
		pass3.newTextureDependency(ssaoRt, TextureUsageBit::kRtvDsvWrite);
		pass3.newTextureDependency(ssaoVBlurRt, TextureUsageBit::kSrvPixel);
	}

	// Volumetric
	RenderTargetHandle volRt = descr.newRenderTarget(newRTDescr("Vol"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("Vol main");
		pass.newTextureDependency(volRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(quarterDepthRt, TextureUsageBit::kSrvPixel);

		RenderTargetHandle volVBlurRt = descr.newRenderTarget(newRTDescr("Vol tmp"));
		GraphicsRenderPass& pass2 = descr.newGraphicsRenderPass("Vol vblur");
		pass2.newTextureDependency(volRt, TextureUsageBit::kSrvPixel);
		pass2.newTextureDependency(volVBlurRt, TextureUsageBit::kRtvDsvWrite);

		GraphicsRenderPass& pass3 = descr.newGraphicsRenderPass("Vol hblur");
		pass3.newTextureDependency(volRt, TextureUsageBit::kRtvDsvWrite);
		pass3.newTextureDependency(volVBlurRt, TextureUsageBit::kSrvPixel);
	}

	// Forward shading
	RenderTargetHandle fsRt = descr.newRenderTarget(newRTDescr("FS"));
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("Forward shading");
		pass.newTextureDependency(fsRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(halfDepthRt, TextureUsageBit::kSrvPixel | TextureUsageBit::kRtvDsvRead);
		pass.newTextureDependency(volRt, TextureUsageBit::kSrvPixel);
	}

	// Light shading
	RenderTargetHandle lightRt = descr.importRenderTarget(dummyTex.get(), TextureUsageBit::kNone);
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("Light shading");

		pass.newTextureDependency(lightRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(gbuffRt0, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(gbuffRt1, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(gbuffRt2, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(gbuffDepth, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(smExpRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(giGiLightRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(ssaoRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(fsRt, TextureUsageBit::kSrvPixel);
	}

	// TAA
	RenderTargetHandle taaHistoryRt = descr.importRenderTarget(dummyTex.get(), TextureUsageBit::kSrvPixel);
	RenderTargetHandle taaRt = descr.importRenderTarget(dummyTex.get(), TextureUsageBit::kNone);
	{
		GraphicsRenderPass& pass = descr.newGraphicsRenderPass("Temporal AA");

		pass.newTextureDependency(lightRt, TextureUsageBit::kSrvPixel);
		pass.newTextureDependency(taaRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(taaHistoryRt, TextureUsageBit::kSrvPixel);
	}

	rgraph->compileNewGraph(descr, pool);
	COMMON_END()
#endif
}

/// Test workarounds for some unsupported formats
ANKI_TEST(Gr, VkWorkarounds)
{
#if 0
	COMMON_BEGIN()

	// Create program
	static const char* COMP_SRC = R"(
layout(local_size_x = 8, local_size_y = 8, local_size_z = 2) in;

layout(set = 0, binding = 0) uniform usampler2D u_tex;

layout(set = 0, binding = 1) buffer s_
{
	uvec4 u_result;
};

shared uint g_wrong;

void main()
{
	g_wrong = 0;
	memoryBarrierShared();
	barrier();

	int lod = -1;
	uint idx;

	if(gl_LocalInvocationID.z == 0)
	{
		// First mip

		lod = 0;
		idx = gl_LocalInvocationID.y * 8 + gl_LocalInvocationID.x;
	}
	else if(gl_LocalInvocationID.x < 4u && gl_LocalInvocationID.y < 4u)
	{
		lod = 1;
		idx = gl_LocalInvocationID.y * 4 + gl_LocalInvocationID.x;
	}

	if(lod != -1)
	{
		uvec3 col = texelFetch(u_tex, ivec2(gl_LocalInvocationID.x, gl_LocalInvocationID.y), lod).rgb;
		if(col.x != idx || col.y != idx + 1 || col.z != idx + 2)
		{
			atomicAdd(g_wrong, 1);
		}
	}

	memoryBarrierShared();
	barrier();

	if(g_wrong != 0)
	{
		u_result = uvec4(1);
	}
	else
	{
		u_result = uvec4(2);
	}
})";

	ShaderPtr comp = createShader(COMP_SRC, ShaderType::kCompute, *g_gr);
	ShaderProgramInitInfo sinf;
	sinf.m_computeShader = comp.get();
	ShaderProgramPtr prog = g_gr->newShaderProgram(sinf);

	// Create the texture
	TextureInitInfo texInit;
	texInit.m_width = texInit.m_height = 8;
	texInit.m_format = Format::kR8G8B8_Uint;
	texInit.m_type = TextureType::k2D;
	texInit.m_usage = TextureUsageBit::kCopyDestination | TextureUsageBit::kAllSrv;
	texInit.m_mipmapCount = 2;
	TexturePtr tex = g_gr->newTexture(texInit);
	TextureViewPtr texView = g_gr->newTextureView(TextureViewInitInfo(tex.get()));

	SamplerInitInfo samplerInit;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	// Create the buffer to copy to the texture
	BufferPtr uploadBuff =
		g_gr->newBuffer(BufferInitInfo(PtrSize(texInit.m_width) * texInit.m_height * 3, BufferUsageBit::kAllCopy, BufferMapAccessBit::kWrite));
	U8* data = static_cast<U8*>(uploadBuff->map(0, uploadBuff->getSize(), BufferMapAccessBit::kWrite));
	for(U32 i = 0; i < texInit.m_width * texInit.m_height; ++i)
	{
		data[0] = U8(i);
		data[1] = U8(i + 1);
		data[2] = U8(i + 2);
		data += 3;
	}
	uploadBuff->unmap();

	BufferPtr uploadBuff2 = g_gr->newBuffer(
		BufferInitInfo(PtrSize(texInit.m_width >> 1) * (texInit.m_height >> 1) * 3, BufferUsageBit::kAllCopy, BufferMapAccessBit::kWrite));
	data = static_cast<U8*>(uploadBuff2->map(0, uploadBuff2->getSize(), BufferMapAccessBit::kWrite));
	for(U i = 0; i < (texInit.m_width >> 1) * (texInit.m_height >> 1); ++i)
	{
		data[0] = U8(i);
		data[1] = U8(i + 1);
		data[2] = U8(i + 2);
		data += 3;
	}
	uploadBuff2->unmap();

	// Create the result buffer
	BufferPtr resultBuff = g_gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::kUavCompute, BufferMapAccessBit::kRead));

	// Upload data and test them
	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbInit);

	TextureSubresourceInfo subresource;
	subresource.m_mipmapCount = texInit.m_mipmapCount;
	setTextureBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination, subresource);
	cmdb->copyBufferToTexture(BufferView(uploadBuff.get()), g_gr->newTextureView(TextureViewInitInfo(tex.get(), TextureSurfaceDescriptor(0, 0, 0))).get());
	cmdb->copyBufferToTexture(BufferView(uploadBuff2.get()), g_gr->newTextureView(TextureViewInitInfo(tex.get(), TextureSurfaceDescriptor(1, 0, 0))).get());

	setTextureBarrier(cmdb, tex, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvCompute, subresource);
	cmdb->bindShaderProgram(prog.get());
	// cmdb->bindTextureAndSampler(0, 0, texView.get(), sampler.get());
	cmdb->bindStorageBuffer(0, 1, BufferView(resultBuff.get()));
	cmdb->dispatchCompute(1, 1, 1);

	setBufferBarrier(cmdb, resultBuff, BufferUsageBit::kUavCompute, BufferUsageBit::kUavCompute, 0, resultBuff->getSize());

	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get());
	g_gr->finish();

	// Get the result
	UVec4* result = static_cast<UVec4*>(resultBuff->map(0, resultBuff->getSize(), BufferMapAccessBit::kRead));
	ANKI_TEST_EXPECT_EQ(result->x(), 2);
	ANKI_TEST_EXPECT_EQ(result->y(), 2);
	ANKI_TEST_EXPECT_EQ(result->z(), 2);
	ANKI_TEST_EXPECT_EQ(result->w(), 2);
	resultBuff->unmap();

	COMMON_END()
#endif
}

ANKI_TEST(Gr, PushConsts)
{
#if 0
	COMMON_BEGIN()

	static const char* VERT_SRC = R"(
struct PC
{
	vec4 color;
	ivec4 icolor;
	vec4 arr[2];
	mat4 mat;
};
layout(push_constant, std140) uniform pc_
{
	PC regs;
};

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec4 out_color;

void main()
{
	vec2 uv = vec2(gl_VertexID & 1, gl_VertexID >> 1) * 2.0;
	vec2 pos = uv * 2.0 - 1.0;
	gl_Position = vec4(pos, 0.0, 1.0);

	out_color = regs.color;
}
)";

	static const char* FRAG_SRC = R"(
struct PC
{
	vec4 color;
	ivec4 icolor;
	vec4 arr[2];
	mat4 mat;
};
layout(push_constant, std140) uniform pc_
{
	PC regs;
};

layout(location = 0) in vec4 in_color;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) buffer s_
{
	uvec4 u_result;
};

void main()
{
	out_color = vec4(1.0);

	if(gl_FragCoord.x == 0.5 && gl_FragCoord.y == 0.5)
	{
		if(in_color != vec4(1.0, 0.0, 1.0, 0.0) || regs.icolor != ivec4(-1, 1, 2147483647, -2147483647)
			|| regs.arr[0] != vec4(1, 2, 3, 4) || regs.arr[1] != vec4(10, 20, 30, 40)
			|| regs.mat[1][0] != 0.5)
		{
			u_result = uvec4(1u);
		}
		else
		{
			u_result = uvec4(2u);
		}
	}
}
)";

	ShaderProgramPtr prog = createProgram(VERT_SRC, FRAG_SRC, *g_gr);

	// Create the result buffer
	BufferPtr resultBuff =
		g_gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::kAllStorage | BufferUsageBit::kCopyDestination, BufferMapAccessBit::kRead));

	// Draw
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

	cmdb->fillBuffer(resultBuff.get(), 0, resultBuff->getSize(), 0);
	setBufferBarrier(cmdb, resultBuff, BufferUsageBit::kCopyDestination, BufferUsageBit::kStorageFragmentWrite, 0, resultBuff->getSize());

	cmdb->setViewport(0, 0, WIDTH, HEIGHT);
	cmdb->bindShaderProgram(prog.get());

	struct PushConstants
	{
		Vec4 m_color = Vec4(1.0, 0.0, 1.0, 0.0);
		IVec4 m_icolor = IVec4(-1, 1, 2147483647, -2147483647);
		Vec4 m_arr[2] = {Vec4(1, 2, 3, 4), Vec4(10, 20, 30, 40)};
		Mat4 m_mat = Mat4(0.0f);
	} pc;
	pc.m_mat(0, 1) = 0.5f;
	cmdb->setFastConstants(&pc, sizeof(pc));

	cmdb->bindStorageBuffer(0, 0, resultBuff.get(), 0, resultBuff->getSize());
	TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
	FramebufferPtr dfb = createColorFb(*g_gr, presentTex);
	presentBarrierA(cmdb, presentTex);
	cmdb->beginRenderPass(dfb.get(), {TextureUsageBit::kRtvDsvWrite}, {});
	cmdb->draw(PrimitiveTopology::kTriangles, 3);
	cmdb->endRenderPass();
	presentBarrierB(cmdb, presentTex);
	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get());

	g_gr->swapBuffers();
	g_gr->finish();

	// Get the result
	UVec4* result = static_cast<UVec4*>(resultBuff->map(0, resultBuff->getSize(), BufferMapAccessBit::kRead));
	ANKI_TEST_EXPECT_EQ(result->x(), 2);
	ANKI_TEST_EXPECT_EQ(result->y(), 2);
	ANKI_TEST_EXPECT_EQ(result->z(), 2);
	ANKI_TEST_EXPECT_EQ(result->w(), 2);
	resultBuff->unmap();

	COMMON_END()
#endif
}

ANKI_TEST(Gr, BindingWithArray)
{
#if 0
	COMMON_BEGIN()

	// Create result buffer
	BufferPtr resBuff = g_gr->newBuffer(BufferInitInfo(sizeof(Vec4), BufferUsageBit::kAllCompute, BufferMapAccessBit::kRead));

	Array<BufferPtr, 4> uniformBuffers;
	F32 count = 1.0f;
	for(BufferPtr& ptr : uniformBuffers)
	{
		ptr = g_gr->newBuffer(BufferInitInfo(sizeof(Vec4), BufferUsageBit::kAllCompute, BufferMapAccessBit::kWrite));

		Vec4* mapped = static_cast<Vec4*>(ptr->map(0, sizeof(Vec4), BufferMapAccessBit::kWrite));
		*mapped = Vec4(count, count + 1.0f, count + 2.0f, count + 3.0f);
		count += 4.0f;
		ptr->unmap();
	}

	// Create program
	static const char* PROG_SRC = R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform u_
{
	vec4 m_vec;
} u_ubos[4];

layout(set = 0, binding = 1) writeonly buffer ss_
{
	vec4 u_result;
};

void main()
{
	u_result = u_ubos[0].m_vec + u_ubos[1].m_vec + u_ubos[2].m_vec + u_ubos[3].m_vec;
})";

	ShaderPtr shader = createShader(PROG_SRC, ShaderType::kCompute, *g_gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_computeShader = shader.get();
	ShaderProgramPtr prog = g_gr->newShaderProgram(sprogInit);

	// Run
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

	for(U32 i = 0; i < uniformBuffers.getSize(); ++i)
	{
		cmdb->bindUniformBuffer(0, 0, BufferView(uniformBuffers[i].get()), i);
	}

	cmdb->bindStorageBuffer(0, 1, BufferView(resBuff.get()));

	cmdb->bindShaderProgram(prog.get());
	cmdb->dispatchCompute(1, 1, 1);

	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get());
	g_gr->finish();

	// Check result
	Vec4* res = static_cast<Vec4*>(resBuff->map(0, sizeof(Vec4), BufferMapAccessBit::kRead));

	ANKI_TEST_EXPECT_EQ(res->x(), 28.0f);
	ANKI_TEST_EXPECT_EQ(res->y(), 32.0f);
	ANKI_TEST_EXPECT_EQ(res->z(), 36.0f);
	ANKI_TEST_EXPECT_EQ(res->w(), 40.0f);

	resBuff->unmap();

	COMMON_END();
#endif
}

ANKI_TEST(Gr, Bindless)
{
#if 0
	COMMON_BEGIN()

	// Create texture A
	TextureInitInfo texInit;
	texInit.m_width = 1;
	texInit.m_height = 1;
	texInit.m_format = Format::R32G32B32A32_UINT;
	texInit.m_usage = TextureUsageBit::kAllUav | TextureUsageBit::ALL_TRANSFER | TextureUsageBit::kAllSrv;
	texInit.m_mipmapCount = 1;

	TexturePtr texA = gr->newTexture(texInit);

	// Create texture B
	TexturePtr texB = gr->newTexture(texInit);

	// Create texture C
	texInit.m_format = Format::R32G32B32A32_SFLOAT;
	TexturePtr texC = gr->newTexture(texInit);

	// Create sampler
	SamplerInitInfo samplerInit;
	SamplerPtr sampler = gr->newSampler(samplerInit);

	// Create views
	TextureViewPtr viewA = gr->newTextureView(TextureViewInitInfo(texA, TextureSurfaceDescriptor()));
	TextureViewPtr viewB = gr->newTextureView(TextureViewInitInfo(texB, TextureSurfaceDescriptor()));
	TextureViewPtr viewC = gr->newTextureView(TextureViewInitInfo(texC, TextureSurfaceDescriptor()));

	// Create result buffer
	BufferPtr resBuff =
		gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::kAllCompute, BufferMapAccessBit::kRead));

	// Create program A
	static const char* PROG_SRC = R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

ANKI_BINDLESS_SET(0u);

layout(set = 1, binding = 0) writeonly buffer ss_
{
	uvec4 u_result;
};

layout(set = 1, binding = 1) uniform sampler u_sampler;

layout(push_constant) uniform pc_
{
	uvec4 u_texIndices;
};

void main()
{
	uvec4 val0 = imageLoad(u_bindlessImages2dU32[u_texIndices[0]], ivec2(0));
	uvec4 val1 = texelFetch(usampler2D(u_bindlessTextures2dU32[u_texIndices[1]], u_sampler), ivec2(0), 0);
	vec4 val2 = texelFetch(sampler2D(u_bindlessTextures2dF32[u_texIndices[2]], u_sampler), ivec2(0), 0);

	u_result = val0 + val1 + uvec4(val2);
})";

	ShaderPtr shader = createShader(PROG_SRC, ShaderType::kCompute, *gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_computeShader = shader;
	ShaderProgramPtr prog = gr->newShaderProgram(sprogInit);

	// Run
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

	setTextureSurfaceBarrier(cmdb, texA, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination,
								   TextureSurfaceDescriptor());
	setTextureSurfaceBarrier(cmdb, texB, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination,
								   TextureSurfaceDescriptor());
	setTextureSurfaceBarrier(cmdb, texC, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination,
								   TextureSurfaceDescriptor());

	TransferGpuAllocatorHandle handle0, handle1, handle2;
	const UVec4 mip0 = UVec4(1, 2, 3, 4);
	UPLOAD_TEX_SURFACE(cmdb, texA, TextureSurfaceDescriptor(0, 0, 0, 0), &mip0[0], sizeof(mip0), handle0);
	const UVec4 mip1 = UVec4(10, 20, 30, 40);
	UPLOAD_TEX_SURFACE(cmdb, texB, TextureSurfaceDescriptor(0, 0, 0, 0), &mip1[0], sizeof(mip1), handle1);
	const Vec4 mip2 = Vec4(2.2f, 3.3f, 4.4f, 5.5f);
	UPLOAD_TEX_SURFACE(cmdb, texC, TextureSurfaceDescriptor(0, 0, 0, 0), &mip2[0], sizeof(mip2), handle2);

	setTextureSurfaceBarrier(cmdb, texA, TextureUsageBit::kCopyDestination, TextureUsageBit::kUavComputeRead,
								   TextureSurfaceDescriptor());
	setTextureSurfaceBarrier(cmdb, texB, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvCompute,
								   TextureSurfaceDescriptor());
	setTextureSurfaceBarrier(cmdb, texC, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvCompute,
								   TextureSurfaceDescriptor());

	cmdb->bindStorageBuffer(1, 0, resBuff, 0, kMaxPtrSize);
	cmdb->bindSampler(1, 1, sampler);
	cmdb->bindShaderProgram(prog);

	const U32 idx0 = viewA->getOrCreateBindlessImageIndex();
	const U32 idx1 = viewB->getOrCreateBindlessTextureIndex();
	const U32 idx2 = viewC->getOrCreateBindlessTextureIndex();
	UVec4 pc(idx0, idx1, idx2, 0);
	cmdb->setFastConstants(&pc, sizeof(pc));

	cmdb->bindAllBindless(0);

	cmdb->dispatchCompute(1, 1, 1);

	// Read result
	FencePtr fence;
	cmdb->flush({}, &fence);
	transfAlloc->release(handle0, fence);
	transfAlloc->release(handle1, fence);
	transfAlloc->release(handle2, fence);
	gr->finish();

	// Check result
	UVec4* res = static_cast<UVec4*>(resBuff->map(0, sizeof(UVec4), BufferMapAccessBit::kRead));

	ANKI_TEST_EXPECT_EQ(res->x(), 13);
	ANKI_TEST_EXPECT_EQ(res->y(), 25);
	ANKI_TEST_EXPECT_EQ(res->z(), 37);
	ANKI_TEST_EXPECT_EQ(res->w(), 49);

	resBuff->unmap();

	COMMON_END()
#endif
}

ANKI_TEST(Gr, RayQuery)
{
#if 0
	COMMON_BEGIN();

	const Bool useRayTracing = g_gr->getDeviceCapabilities().m_rayTracingEnabled;
	if(!useRayTracing)
	{
		ANKI_TEST_LOGW("Test will run without using ray tracing");
	}

	// Index buffer
	BufferPtr idxBuffer;
	if(useRayTracing)
	{
		Array<U16, 3> indices = {0, 1, 2};
		BufferInitInfo init;
		init.m_mapAccess = BufferMapAccessBit::kWrite;
		init.m_usage = BufferUsageBit::kIndex;
		init.m_size = sizeof(indices);
		idxBuffer = g_gr->newBuffer(init);

		void* addr = idxBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
		memcpy(addr, &indices[0], sizeof(indices));
		idxBuffer->unmap();
	}

	// Position buffer (add some padding to complicate things a bit)
	BufferPtr vertBuffer;
	if(useRayTracing)
	{
		Array<Vec4, 3> verts = {{{-1.0f, 0.0f, 0.0f, 100.0f}, {1.0f, 0.0f, 0.0f, 100.0f}, {0.0f, 2.0f, 0.0f, 100.0f}}};

		BufferInitInfo init;
		init.m_mapAccess = BufferMapAccessBit::kWrite;
		init.m_usage = BufferUsageBit::kVertex;
		init.m_size = sizeof(verts);
		vertBuffer = g_gr->newBuffer(init);

		void* addr = vertBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
		memcpy(addr, &verts[0], sizeof(verts));
		vertBuffer->unmap();
	}

	// BLAS
	AccelerationStructurePtr blas;
	if(useRayTracing)
	{
		AccelerationStructureInitInfo init;
		init.m_type = AccelerationStructureType::kBottomLevel;
		init.m_bottomLevel.m_indexBuffer = idxBuffer.get();
		init.m_bottomLevel.m_indexCount = 3;
		init.m_bottomLevel.m_indexType = IndexType::kU16;
		init.m_bottomLevel.m_positionBuffer = vertBuffer.get();
		init.m_bottomLevel.m_positionCount = 3;
		init.m_bottomLevel.m_positionsFormat = Format::kR32G32B32_Sfloat;
		init.m_bottomLevel.m_positionStride = 4 * 4;

		blas = g_gr->newAccelerationStructure(init);
	}

	// TLAS
	AccelerationStructurePtr tlas;
	if(useRayTracing)
	{
		AccelerationStructureInitInfo init;
		init.m_type = AccelerationStructureType::kTopLevel;
		Array<AccelerationStructureInstanceInfo, 1> instances = {{{blas, Mat3x4::getIdentity()}}};
		init.m_topLevel.m_directArgs.m_instances = instances;

		tlas = g_gr->newAccelerationStructure(init);
	}

	// Program
	ShaderProgramPtr prog;
	{
		CString src = R"(

#if USE_RAY_TRACING
#extension GL_EXT_ray_query : enable
#endif

layout(push_constant, std140, row_major) uniform b_pc
{
	Mat4 u_vp;
	Vec3 u_cameraPos;
	F32 u_padding0;
};

#if USE_RAY_TRACING
layout(set = 0, binding = 0) uniform accelerationStructureEXT u_tlas;
#endif

layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_color;

Bool rayTriangleIntersect(Vec3 orig, Vec3 dir, Vec3 v0, Vec3 v1, Vec3 v2, out F32 t, out F32 u, out F32 v)
{
	const Vec3 v0v1 = v1 - v0;
	const Vec3 v0v2 = v2 - v0;
	const Vec3 pvec = cross(dir, v0v2);
	const F32 det = dot(v0v1, pvec);

	if(det < 0.00001)
	{
		return false;
	}

	const F32 invDet = 1.0 / det;

	const Vec3 tvec = orig - v0;
	u = dot(tvec, pvec) * invDet;
	if(u < 0.0 || u > 1.0)
	{
		return false;
	}

	const Vec3 qvec = cross(tvec, v0v1);
	v = dot(dir, qvec) * invDet;
	if(v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	t = dot(v0v2, qvec) * invDet;
	return true;
}

void main()
{
	// Unproject
	const Vec2 ndc = in_uv * 2.0 - 1.0;
	const Vec4 p4 = inverse(u_vp) * Vec4(ndc, 1.0, 1.0);
	const Vec3 p3 = p4.xyz / p4.w;

	const Vec3 rayDir = normalize(p3 - u_cameraPos);
	const Vec3 rayOrigin = u_cameraPos;

#if USE_RAY_TRACING
	Bool hit = false;
	F32 u = 0.0;
	F32 v = 0.0;

	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, u_tlas, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT, 0xFFu, rayOrigin,
		0.01, rayDir, 1000.0);

	rayQueryProceedEXT(rayQuery);

	const U32 committedStatus = rayQueryGetIntersectionTypeEXT(rayQuery, true);
	if(committedStatus == gl_RayQueryCommittedIntersectionTriangleEXT)
	{
		const Vec2 bary = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
		u = bary.x;
		v = bary.y;
		hit = true;
	}
#else
	// Manual trace
	Vec3 arr[3] = Vec3[](Vec3(-1.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 2.0f, 0.0f));
	F32 t;
	F32 u;
	F32 v;
	const Bool hit = rayTriangleIntersect(rayOrigin, rayDir, arr[0], arr[1], arr[2], t, u, v);
#endif

	if(hit)
	{
		out_color = Vec3(u, v, 1.0 - (u + v));
	}
	else
	{
		out_color = Vec3(mix(0.5, 0.2, in_uv.x));
	}
}
		)";

		String fragSrc;
		if(useRayTracing)
		{
			fragSrc += "#define USE_RAY_TRACING 1\n";
		}
		else
		{
			fragSrc += "#define USE_RAY_TRACING 0\n";
		}
		fragSrc += src;
		prog = createProgram(VERT_QUAD_STRIP_SRC, fragSrc, *g_gr);
	}

	// Build AS
	if(useRayTracing)
	{
		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		setAccelerationStructureBarrier(cmdb, blas, AccelerationStructureUsageBit::kNone, AccelerationStructureUsageBit::kBuild);
		BufferInitInfo scratchInit;
		scratchInit.m_size = blas->getBuildScratchBufferSize();
		scratchInit.m_usage = BufferUsageBit::kAccelerationStructureBuildScratch;
		BufferPtr scratchBuff = GrManager::getSingleton().newBuffer(scratchInit);
		cmdb->buildAccelerationStructure(blas.get(), scratchBuff.get(), 0);
		setAccelerationStructureBarrier(cmdb, blas, AccelerationStructureUsageBit::kBuild, AccelerationStructureUsageBit::kAttach);

		setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kNone, AccelerationStructureUsageBit::kBuild);
		scratchInit.m_size = tlas->getBuildScratchBufferSize();
		scratchBuff = GrManager::getSingleton().newBuffer(scratchInit);
		cmdb->buildAccelerationStructure(tlas.get(), scratchBuff.get(), 0);
		setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kBuild, AccelerationStructureUsageBit::kFragmentRead);

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());
	}

	// Draw
	constexpr U32 ITERATIONS = 200;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		const Vec4 cameraPos(0.0f, 0.0f, 3.0f, 0.0f);
		const Mat4 viewMat = Mat4(cameraPos.xyz(), Mat3::getIdentity(), Vec3(1.0f)).getInverse();
		const Mat4 projMat = Mat4::calculatePerspectiveProjectionMatrix(toRad(90.0f), toRad(90.0f), 0.01f, 1000.0f);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);

		cmdb->bindShaderProgram(prog.get());
		struct PC
		{
			Mat4 m_vp;
			Vec4 m_cameraPos;
		} pc;
		pc.m_vp = projMat * viewMat;
		pc.m_cameraPos = cameraPos;
		cmdb->setFastConstants(&pc, sizeof(pc));

		if(useRayTracing)
		{
			cmdb->bindAccelerationStructure(0, 0, tlas.get());
		}

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kNone, TextureUsageBit::kRtvDsvWrite, TextureSubresourceInfo{});

		cmdb->beginRenderPass(fb.get(), {TextureUsageBit::kRtvDsvWrite}, {});
		cmdb->draw(PrimitiveTopology::kTriangleStrip, 4);
		cmdb->endRenderPass();

		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kRtvDsvWrite, TextureUsageBit::kPresent, TextureSubresourceInfo{});

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END();
#endif
}

static void createCubeBuffers(GrManager& gr, Vec3 min, Vec3 max, BufferPtr& indexBuffer, BufferPtr& vertBuffer, Bool turnInsideOut = false)
{
	BufferInitInfo inf;
	inf.m_mapAccess = BufferMapAccessBit::kWrite;
	inf.m_usage = BufferUsageBit::kVertexOrIndex | BufferUsageBit::kSrvTraceRays;
	inf.m_size = sizeof(Vec3) * 8;
	vertBuffer = gr.newBuffer(inf);
	WeakArray<Vec3, PtrSize> positions = vertBuffer->map<Vec3>(0, 8, BufferMapAccessBit::kWrite);

	//   7------6
	//  /|     /|
	// 3------2 |
	// | |    | |
	// | 4 ---|-5
	// |/     |/
	// 0------1
	positions[0] = Vec3(min.x(), min.y(), max.z());
	positions[1] = Vec3(max.x(), min.y(), max.z());
	positions[2] = Vec3(max.x(), max.y(), max.z());
	positions[3] = Vec3(min.x(), max.y(), max.z());
	positions[4] = Vec3(min.x(), min.y(), min.z());
	positions[5] = Vec3(max.x(), min.y(), min.z());
	positions[6] = Vec3(max.x(), max.y(), min.z());
	positions[7] = Vec3(min.x(), max.y(), min.z());

	vertBuffer->unmap();

	inf.m_size = sizeof(U16) * 36;
	indexBuffer = gr.newBuffer(inf);
	WeakArray<U16, PtrSize> indices = indexBuffer->map<U16>(0, 36, BufferMapAccessBit::kWrite);
	U32 t = 0;

	// Top
	indices[t++] = 3;
	indices[t++] = 2;
	indices[t++] = 7;
	indices[t++] = 2;
	indices[t++] = 6;
	indices[t++] = 7;

	// Bottom
	indices[t++] = 1;
	indices[t++] = 0;
	indices[t++] = 4;
	indices[t++] = 1;
	indices[t++] = 4;
	indices[t++] = 5;

	// Left
	indices[t++] = 0;
	indices[t++] = 3;
	indices[t++] = 4;
	indices[t++] = 3;
	indices[t++] = 7;
	indices[t++] = 4;

	// Right
	indices[t++] = 1;
	indices[t++] = 5;
	indices[t++] = 2;
	indices[t++] = 2;
	indices[t++] = 5;
	indices[t++] = 6;

	// Front
	indices[t++] = 0;
	indices[t++] = 1;
	indices[t++] = 3;
	indices[t++] = 3;
	indices[t++] = 1;
	indices[t++] = 2;

	// Back
	indices[t++] = 4;
	indices[t++] = 7;
	indices[t++] = 6;
	indices[t++] = 5;
	indices[t++] = 4;
	indices[t++] = 6;

	ANKI_ASSERT(t == indices.getSize());

	if(turnInsideOut)
	{
		for(U32 i = 0; i < t; i += 3)
		{
			std::swap(indices[i + 1], indices[i + 2]);
		}
	}

	indexBuffer->unmap();
}

enum class GeomWhat
{
	SMALL_BOX,
	BIG_BOX,
	ROOM,
	LIGHT,
	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GeomWhat)

ANKI_TEST(Gr, RayGen)
{
#if 0
	COMMON_BEGIN();

	const Bool useRayTracing = g_gr->getDeviceCapabilities().m_rayTracingEnabled;
	if(!useRayTracing)
	{
		ANKI_TEST_LOGW("Ray tracing not supported");
		break;
	}

	using Mat3x4Scalar = Array2d<F32, 3, 4>;
#	define MAGIC_MACRO(x) x
#	include <Tests/Gr/RtTypes.h>
#	undef MAGIC_MACRO

	HeapMemoryPool pool(allocAligned, nullptr);

	// Create the offscreen RTs
	Array<TexturePtr, 2> offscreenRts;
	{
		TextureInitInfo inf("T_offscreen#1");
		inf.m_width = WIDTH;
		inf.m_height = HEIGHT;
		inf.m_format = Format::kR8G8B8A8_Unorm;
		inf.m_usage = TextureUsageBit::kUavTraceRaysRead | TextureUsageBit::kUavTraceRaysWrite | TextureUsageBit::kUavComputeRead;

		offscreenRts[0] = g_gr->newTexture(inf);

		inf.setName("T_offscreen#2");
		offscreenRts[1] = g_gr->newTexture(inf);
	}

	// Copy to present program
	ShaderProgramPtr copyToPresentProg;
	{
		const CString src = R"(
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform readonly image2D u_inImg;
layout(set = 0, binding = 1) uniform writeonly image2D u_outImg;

void main()
{
	const UVec2 size = UVec2(imageSize(u_inImg));
	if(gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y)
	{
		return;
	}

	const Vec4 col = imageLoad(u_inImg, IVec2(gl_GlobalInvocationID.xy));
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), col);
})";

		ShaderPtr shader = createShader(src, ShaderType::kCompute, *g_gr);
		ShaderProgramInitInfo sprogInit;
		sprogInit.m_computeShader = shader.get();
		copyToPresentProg = g_gr->newShaderProgram(sprogInit);
	}

	// Create the gometries
	struct Geom
	{
		BufferPtr m_vertexBuffer;
		BufferPtr m_indexBuffer;
		Aabb m_aabb;
		Mat3x4 m_worldTransform;
		Mat3 m_worldRotation;
		Bool m_insideOut = false;
		U8 m_asMask = 0b10;
		AccelerationStructurePtr m_blas;
		U32 m_indexCount = 36;
		Vec3 m_diffuseColor = Vec3(0.0f);
		Vec3 m_emissiveColor = Vec3(0.0f);
	};

	Array<Geom, U(GeomWhat::kCount)> geometries;
	geometries[GeomWhat::SMALL_BOX].m_aabb = Aabb(Vec3(130.0f, 0.0f, 65.0f), Vec3(295.0f, 160.0f, 230.0f));
	geometries[GeomWhat::SMALL_BOX].m_worldRotation = Mat3(Axisang(toRad(-18.0f), Vec3(0.0f, 1.0f, 0.0f)));
	geometries[GeomWhat::SMALL_BOX].m_worldTransform =
		Mat3x4(Vec3((geometries[GeomWhat::SMALL_BOX].m_aabb.getMin() + geometries[GeomWhat::SMALL_BOX].m_aabb.getMax()).xyz() / 2.0f),
			   geometries[GeomWhat::SMALL_BOX].m_worldRotation);
	geometries[GeomWhat::SMALL_BOX].m_diffuseColor = Vec3(0.75f);

	geometries[GeomWhat::BIG_BOX].m_aabb = Aabb(Vec3(265.0f, 0.0f, 295.0f), Vec3(430.0f, 330.0f, 460.0f));
	geometries[GeomWhat::BIG_BOX].m_worldRotation = Mat3(Axisang(toRad(15.0f), Vec3(0.0f, 1.0f, 0.0f)));
	geometries[GeomWhat::BIG_BOX].m_worldTransform =
		Mat3x4(Vec3((geometries[GeomWhat::BIG_BOX].m_aabb.getMin() + geometries[GeomWhat::BIG_BOX].m_aabb.getMax()).xyz() / 2.0f),
			   geometries[GeomWhat::BIG_BOX].m_worldRotation);
	geometries[GeomWhat::BIG_BOX].m_diffuseColor = Vec3(0.75f);

	geometries[GeomWhat::ROOM].m_aabb = Aabb(Vec3(0.0f), Vec3(555.0f));
	geometries[GeomWhat::ROOM].m_worldRotation = Mat3::getIdentity();
	geometries[GeomWhat::ROOM].m_worldTransform =
		Mat3x4(Vec3((geometries[GeomWhat::ROOM].m_aabb.getMin() + geometries[GeomWhat::ROOM].m_aabb.getMax()).xyz() / 2.0f),
			   geometries[GeomWhat::ROOM].m_worldRotation);
	geometries[GeomWhat::ROOM].m_insideOut = true;
	geometries[GeomWhat::ROOM].m_indexCount = 30;

	geometries[GeomWhat::LIGHT].m_aabb = Aabb(Vec3(213.0f + 1.0f, 554.0f, 227.0f + 1.0f), Vec3(343.0f - 1.0f, 554.0f + 0.001f, 332.0f - 1.0f));
	geometries[GeomWhat::LIGHT].m_worldRotation = Mat3::getIdentity();
	geometries[GeomWhat::LIGHT].m_worldTransform =
		Mat3x4(Vec3((geometries[GeomWhat::LIGHT].m_aabb.getMin() + geometries[GeomWhat::LIGHT].m_aabb.getMax()).xyz() / 2.0f),
			   geometries[GeomWhat::LIGHT].m_worldRotation);
	geometries[GeomWhat::LIGHT].m_asMask = 0b01;
	geometries[GeomWhat::LIGHT].m_emissiveColor = Vec3(15.0f);

	// Create Buffers
	for(Geom& g : geometries)
	{
		createCubeBuffers(*g_gr, -(g.m_aabb.getMax().xyz() - g.m_aabb.getMin().xyz()) / 2.0f,
						  (g.m_aabb.getMax().xyz() - g.m_aabb.getMin().xyz()) / 2.0f, g.m_indexBuffer, g.m_vertexBuffer, g.m_insideOut);
	}

	// Create AS
	AccelerationStructurePtr tlas;
	{
		for(Geom& g : geometries)
		{
			AccelerationStructureInitInfo inf;
			inf.m_type = AccelerationStructureType::kBottomLevel;
			inf.m_bottomLevel.m_indexBuffer = BufferView(g.m_indexBuffer.get());
			inf.m_bottomLevel.m_indexType = IndexType::kU16;
			inf.m_bottomLevel.m_indexCount = g.m_indexCount;
			inf.m_bottomLevel.m_positionBuffer = BufferView(g.m_vertexBuffer.get());
			inf.m_bottomLevel.m_positionCount = 8;
			inf.m_bottomLevel.m_positionsFormat = Format::kR32G32B32_Sfloat;
			inf.m_bottomLevel.m_positionStride = sizeof(Vec3);

			g.m_blas = g_gr->newAccelerationStructure(inf);
		}

		// TLAS
		Array<AccelerationStructureInstanceInfo, U32(GeomWhat::kCount)> instances;
		U32 count = 0;
		for(Geom& g : geometries)
		{
			instances[count].m_bottomLevel = g.m_blas;
			instances[count].m_transform = g.m_worldTransform;
			instances[count].m_hitgroupSbtRecordIndex = count;
			instances[count].m_mask = g.m_asMask;

			++count;
		}

		AccelerationStructureInitInfo inf;
		inf.m_type = AccelerationStructureType::kTopLevel;
		inf.m_topLevel.m_directArgs.m_instances = instances;

		tlas = g_gr->newAccelerationStructure(inf);
	}

	// Create model info
	BufferPtr modelBuffer;
	{
		BufferInitInfo inf;
		inf.m_mapAccess = BufferMapAccessBit::kWrite;
		inf.m_usage = BufferUsageBit::kAllStorage;
		inf.m_size = sizeof(Model) * U32(GeomWhat::kCount);

		modelBuffer = g_gr->newBuffer(inf);
		WeakArray<Model, PtrSize> models = modelBuffer->map<Model>(0, U32(GeomWhat::kCount), BufferMapAccessBit::kWrite);
		memset(&models[0], 0, inf.m_size);

		for(GeomWhat i : EnumIterable<GeomWhat>())
		{
			const Geom& g = geometries[i];
			models[U32(i)].m_mtl.m_diffuseColor = g.m_diffuseColor;
			models[U32(i)].m_mtl.m_emissiveColor = g.m_emissiveColor;
			models[U32(i)].m_mesh.m_indexBufferPtr = g.m_indexBuffer->getGpuAddress();
			models[U32(i)].m_mesh.m_positionBufferPtr = g.m_vertexBuffer->getGpuAddress();
			memcpy(&models[U32(i)].m_worldTransform, &g.m_worldTransform, sizeof(Mat3x4));
			models[U32(i)].m_worldRotation = g.m_worldRotation;
		}

		modelBuffer->unmap();
	}

	// Create the ppline
	ShaderProgramPtr rtProg;
	constexpr U32 rayGenGroupIdx = 1;
	constexpr U32 missGroupIdx = 2;
	constexpr U32 shadowMissGroupIdx = 3;
	constexpr U32 lambertianChitGroupIdx = 4;
	constexpr U32 lambertianRoomChitGroupIdx = 5;
	constexpr U32 emissiveChitGroupIdx = 6;
	constexpr U32 shadowAhitGroupIdx = 7;
	constexpr U32 hitgroupCount = 7;
	{
		const CString commonSrcPart = R"(
#define Mat3x4Scalar Mat3x4

%s

const F32 PI = 3.14159265358979323846;

struct PayLoad
{
	Vec3 m_total;
	Vec3 m_weight;
	Vec3 m_scatteredDir;
	F32 m_hitT;
};

struct ShadowPayLoad
{
	F32 m_shadow;
};

layout(set = 0, binding = 0, scalar) buffer b_00
{
	Model u_models[];
};

layout(set = 0, binding = 1, scalar) buffer b_01
{
	Light u_lights[];
};

layout(push_constant, scalar) uniform b_pc
{
	PushConstants u_regs;
};

#define PAYLOAD_LOCATION 0
#define SHADOW_PAYLOAD_LOCATION 1

ANKI_REF(U16Vec3, ANKI_SIZEOF(U16));
ANKI_REF(Vec3, ANKI_SIZEOF(F32));

Vec3 computePrimitiveNormal(Model model, U32 primitiveId)
{
	const Mesh mesh = model.m_mesh;
	const U32 offset = primitiveId * 6;
	const U16Vec3 indices = U16Vec3Ref(nonuniformEXT(mesh.m_indexBufferPtr + offset)).m_value;

	const Vec3 pos0 = Vec3Ref(nonuniformEXT(mesh.m_positionBufferPtr + indices[0] * ANKI_SIZEOF(Vec3))).m_value;
	const Vec3 pos1 = Vec3Ref(nonuniformEXT(mesh.m_positionBufferPtr + indices[1] * ANKI_SIZEOF(Vec3))).m_value;
	const Vec3 pos2 = Vec3Ref(nonuniformEXT(mesh.m_positionBufferPtr + indices[2] * ANKI_SIZEOF(Vec3))).m_value;

	const Vec3 normal = normalize(cross(pos1 - pos0, pos2 - pos0));
	return model.m_worldRotation * normal;
}

UVec3 rand3DPCG16(UVec3 v)
{
	v = v * 1664525u + 1013904223u;

	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;

	return v >> 16u;
}

Vec2 hammersleyRandom16(U32 sampleIdx, U32 sampleCount, UVec2 random)
{
	const F32 e1 = fract(F32(sampleIdx) / sampleCount + F32(random.x) * (1.0 / 65536.0));
	const F32 e2 = F32((bitfieldReverse(sampleIdx) >> 16) ^ random.y) * (1.0 / 65536.0);
	return Vec2(e1, e2);
}

Vec3 hemisphereSampleUniform(Vec2 uv)
{
	const F32 phi = uv.y * 2.0 * PI;
	const F32 cosTheta = 1.0 - uv.x;
	const F32 sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

Mat3 rotationFromDirection(Vec3 zAxis)
{
	Vec3 z = zAxis;
	F32 sign = (z.z >= 0.0) ? 1.0 : -1.0;
	F32 a = -1.0 / (sign + z.z);
	F32 b = z.x * z.y * a;

	Vec3 x = Vec3(1.0 + sign * a * pow(z.x, 2.0), sign * b, -sign * z.x);
	Vec3 y = Vec3(b, sign + a * pow(z.y, 2.0), -z.y);

	return Mat3(x, y, z);
}

Vec3 randomDirectionInHemisphere(Vec3 normal)
{
	const UVec2 random = rand3DPCG16(UVec3(gl_LaunchIDEXT.xy, u_regs.m_frame)).xy;
	const Vec2 uniformRandom = hammersleyRandom16(0, 0xFFFFu, random);
	return normalize(rotationFromDirection(normal) * hemisphereSampleUniform(uniformRandom));
}

void scatterLambertian(Vec3 normal, out Vec3 scatterDir, out F32 pdf)
{
	scatterDir = randomDirectionInHemisphere(normal);
	pdf = dot(normal, scatterDir) / PI;
}

F32 scatteringPdfLambertian(Vec3 normal, Vec3 scatteredDir)
{
	F32 cosine = dot(normal, scatteredDir);
	return max(cosine / PI, 0.0);
})";

#	define MAGIC_MACRO ANKI_STRINGIZE
		const CString rtTypesStr =
#	include <Tests/Gr/RtTypes.h>
			;
#	undef MAGIC_MACRO

		String commonSrc;
		commonSrc.sprintf(commonSrcPart.cstr(), rtTypesStr.cstr());

		const CString lambertianSrc = R"(
layout(location = PAYLOAD_LOCATION) rayPayloadInEXT PayLoad s_payLoad;

hitAttributeEXT vec2 g_attribs;

void main()
{
	const Model model = u_models[nonuniformEXT(gl_InstanceID)];

	const Vec3 normal = computePrimitiveNormal(model, gl_PrimitiveID);

	Vec3 scatteredDir;
	F32 pdf;
	scatterLambertian(normal, scatteredDir, pdf);

	const F32 scatteringPdf = scatteringPdfLambertian(normal, scatteredDir);

	s_payLoad.m_total += model.m_mtl.m_emissiveColor * s_payLoad.m_weight;
	s_payLoad.m_weight *= model.m_mtl.m_diffuseColor * scatteringPdf / pdf;
	s_payLoad.m_scatteredDir = scatteredDir;
	s_payLoad.m_hitT = gl_HitTEXT;
})";

		const CString lambertianRoomSrc = R"(
layout(location = PAYLOAD_LOCATION) rayPayloadInEXT PayLoad s_payLoad;

void main()
{
	Vec3 col;
	U32 quad = gl_PrimitiveID / 2;

	if(quad == 2)
	{
		col = Vec3(0.65, 0.05, 0.05);
	}
	else if(quad == 3)
	{
		col = Vec3(0.12, 0.45, 0.15);
	}
	else
	{
		col = Vec3(0.73f);
	}

	const Model model = u_models[nonuniformEXT(gl_InstanceID)];

	const Vec3 normal = computePrimitiveNormal(model, gl_PrimitiveID);

	Vec3 scatteredDir;
	F32 pdf;
	scatterLambertian(normal, scatteredDir, pdf);

	const F32 scatteringPdf = scatteringPdfLambertian(normal, scatteredDir);

	// Color = diff * scatteringPdf / pdf * trace(depth - 1)
	s_payLoad.m_total += model.m_mtl.m_emissiveColor * s_payLoad.m_weight;
	s_payLoad.m_weight *= col * scatteringPdf / pdf;
	s_payLoad.m_scatteredDir = scatteredDir;
	s_payLoad.m_hitT = gl_HitTEXT;
})";

		const CString emissiveSrc = R"(
layout(location = PAYLOAD_LOCATION) rayPayloadInEXT PayLoad s_payLoad;

void main()
{
	const Model model = u_models[nonuniformEXT(gl_InstanceID)];

	s_payLoad.m_total += model.m_mtl.m_emissiveColor * s_payLoad.m_weight;
	s_payLoad.m_weight = Vec3(0.0);
	s_payLoad.m_scatteredDir = Vec3(1.0, 0.0, 0.0);
	s_payLoad.m_hitT = -1.0;
})";

		const CString missSrc = R"(
layout(location = PAYLOAD_LOCATION) rayPayloadInEXT PayLoad s_payLoad;

void main()
{
	//s_payLoad.m_color =
		//mix(Vec3(0.3, 0.5, 0.3), Vec3(0.1, 0.6, 0.1), F32(gl_LaunchIDEXT.y) / F32(gl_LaunchSizeEXT.y));
		//Vec3(0.0);
	s_payLoad.m_weight = Vec3(0.0);
	s_payLoad.m_scatteredDir = Vec3(1.0, 0.0, 0.0);
	s_payLoad.m_hitT = -1.0;
})";

		const CString shadowAhitSrc = R"(
layout(location = SHADOW_PAYLOAD_LOCATION) rayPayloadInEXT ShadowPayLoad s_payLoad;

void main()
{
	s_payLoad.m_shadow += 0.25;
	//terminateRayEXT();
})";

		const CString shadowChitSrc = R"(
void main()
{
})";

		const CString shadowMissSrc = R"(
layout(location = SHADOW_PAYLOAD_LOCATION) rayPayloadInEXT ShadowPayLoad s_payLoad;

void main()
{
	s_payLoad.m_shadow = 1.0;
})";

		const CString rayGenSrc = R"(
layout(set = 1, binding = 0) uniform accelerationStructureEXT u_tlas;
layout(set = 1, binding = 1, rgba8) uniform readonly image2D u_inImg;
layout(set = 1, binding = 2, rgba8) uniform writeonly image2D u_outImg;

layout(location = PAYLOAD_LOCATION) rayPayloadEXT PayLoad s_payLoad;
layout(location = SHADOW_PAYLOAD_LOCATION) rayPayloadEXT ShadowPayLoad s_shadowPayLoad;

void main()
{
	Vec2 uv = (Vec2(gl_LaunchIDEXT.xy) + 0.5) / Vec2(gl_LaunchSizeEXT.xy);
	uv.y = 1.0 - uv.y;
	const Vec2 ndc = uv * 2.0 - 1.0;
	const Vec4 p4 = inverse(u_regs.m_vp) * Vec4(ndc, 1.0, 1.0);
	const Vec3 p3 = p4.xyz / p4.w;

	const UVec2 random = rand3DPCG16(UVec3(gl_LaunchIDEXT.xy, u_regs.m_frame)).xy;
	const Vec2 randomCircle = hammersleyRandom16(0, 0xFFFFu, random);

	Vec3 outColor = Vec3(0.0);

	const U32 sampleCount = 8;
	const U32 maxRecursionDepth = 2;
	for(U32 s = 0; s < sampleCount; ++s)
	{
		Vec3 rayOrigin = u_regs.m_cameraPos;
		Vec3 rayDir = normalize(p3 - u_regs.m_cameraPos);
		s_payLoad.m_total = Vec3(0.0);
		s_payLoad.m_weight = Vec3(1.0);

		for(U32 depth = 0; depth < maxRecursionDepth; ++depth)
		{
			const U32 cullMask = 0xFF;
			const U32 sbtRecordOffset = 0;
			const U32 sbtRecordStride = 0;
			const U32 missIndex = 0;
			const F32 tMin = 0.1;
			const F32 tMax = 10000.0;
			traceRayEXT(u_tlas, gl_RayFlagsOpaqueEXT, cullMask, sbtRecordOffset, sbtRecordStride, missIndex,
						rayOrigin, tMin, rayDir, tMax, PAYLOAD_LOCATION);

			if(s_payLoad.m_hitT > 0.0)
			{
				rayOrigin = rayOrigin + rayDir * s_payLoad.m_hitT;
				rayDir = s_payLoad.m_scatteredDir;
			}
			else
			{
				break;
			}
		}

		outColor += s_payLoad.m_total + s_payLoad.m_weight;
		//outColor += s_payLoad.m_scatteredDir * 0.5 + 0.5;
	}

	outColor /= F32(sampleCount);

#if 0
	const Vec3 diffuseColor = Vec3(s_payLoad.m_diffuseColor);
	const Vec3 normal = s_payLoad.m_normal;
	if(s_payLoad.m_hitT > 0.0)
	{
		const Vec3 rayOrigin = u_regs.m_cameraPos + normalize(p3 - u_regs.m_cameraPos) * s_payLoad.m_hitT;

		for(U32 i = 0; i < u_regs.m_lightCount; ++i)
		{
			s_shadowPayLoad.m_shadow = 0.0;
			const Light light = u_lights[i];
			const Vec3 randomPointInLight = mix(light.m_min, light.m_max, randomCircle.xyx);

			const Vec3 rayDir = normalize(randomPointInLight - rayOrigin);
			const U32 cullMask = 0x2;
			const U32 sbtRecordOffset = 1;
			const U32 sbtRecordStride = 0;
			const U32 missIndex = 1;
			const F32 tMin = 0.1;
			const F32 tMax = length(randomPointInLight - rayOrigin);
			const U32 flags = gl_RayFlagsOpaqueEXT;
			traceRayEXT(u_tlas, flags, cullMask, sbtRecordOffset, sbtRecordStride, missIndex, rayOrigin,
						tMin, rayDir, tMax, SHADOW_PAYLOAD_LOCATION);

			F32 shadow = clamp(s_shadowPayLoad.m_shadow, 0.0, 1.0);
			outColor += normal * light.m_intensity * shadow;
		}
	}
	else
	{
		outColor = diffuseColor;
	}
#endif

	const Vec3 history = imageLoad(u_inImg, IVec2(gl_LaunchIDEXT.xy)).rgb;
	outColor = mix(outColor, history, (u_regs.m_frame != 0) ?  0.99 : 0.0);
	imageStore(u_outImg, IVec2(gl_LaunchIDEXT.xy), Vec4(outColor, 0.0));
})";

		ShaderPtr lambertianShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), lambertianSrc.cstr()), ShaderType::kClosestHit, *g_gr);
		ShaderPtr lambertianRoomShader =
			createShader(String().sprintf("%s\n%s", commonSrc.cstr(), lambertianRoomSrc.cstr()), ShaderType::kClosestHit, *g_gr);
		ShaderPtr emissiveShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), emissiveSrc.cstr()), ShaderType::kClosestHit, *g_gr);

		ShaderPtr shadowAhitShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), shadowAhitSrc.cstr()), ShaderType::kAnyHit, *g_gr);
		ShaderPtr shadowChitShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), shadowChitSrc.cstr()), ShaderType::kClosestHit, *g_gr);
		ShaderPtr missShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), missSrc.cstr()), ShaderType::kMiss, *g_gr);

		ShaderPtr shadowMissShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), shadowMissSrc.cstr()), ShaderType::kMiss, *g_gr);

		ShaderPtr rayGenShader = createShader(String().sprintf("%s\n%s", commonSrc.cstr(), rayGenSrc.cstr()), ShaderType::kRayGen, *g_gr);

		Array<RayTracingHitGroup, 4> hitGroups;
		hitGroups[0].m_closestHitShader = lambertianShader.get();
		hitGroups[1].m_closestHitShader = lambertianRoomShader.get();
		hitGroups[2].m_closestHitShader = emissiveShader.get();
		hitGroups[3].m_closestHitShader = shadowChitShader.get();
		hitGroups[3].m_anyHitShader = shadowAhitShader.get();

		Array<Shader*, 2> missShaders = {missShader.get(), shadowMissShader.get()};

		// Add the same 2 times to test multiple ray gen shaders
		Array<Shader*, 2> rayGenShaders = {rayGenShader.get(), rayGenShader.get()};

		ShaderProgramInitInfo inf;
		inf.m_rayTracingShaders.m_hitGroups = hitGroups;
		inf.m_rayTracingShaders.m_rayGenShaders = rayGenShaders;
		inf.m_rayTracingShaders.m_missShaders = missShaders;

		rtProg = g_gr->newShaderProgram(inf);
	}

	// Create the SBT
	BufferPtr sbt;
	{
		const U32 recordCount = 1 + 2 + U32(GeomWhat::kCount) * 2;
		const U32 sbtRecordSize = g_gr->getDeviceCapabilities().m_sbtRecordAlignment;

		BufferInitInfo inf;
		inf.m_mapAccess = BufferMapAccessBit::kWrite;
		inf.m_usage = BufferUsageBit::kShaderBindingTable;
		inf.m_size = sbtRecordSize * recordCount;

		sbt = g_gr->newBuffer(inf);
		WeakArray<U8, PtrSize> mapped = sbt->map<U8>(0, inf.m_size, BufferMapAccessBit::kWrite);
		memset(&mapped[0], 0, inf.m_size);

		ConstWeakArray<U8> handles = rtProg->getShaderGroupHandles();
		ANKI_TEST_EXPECT_EQ(handles.getSize(), g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * (hitgroupCount + 1));

		// Ray gen
		U32 record = 0;
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * rayGenGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// 2xMiss
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * missGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowMissGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Small box
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * lambertianChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Big box
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * lambertianChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Room
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * lambertianRoomChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Light
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * emissiveChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++], &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		sbt->unmap();
	}

	// Create lights
	BufferPtr lightBuffer;
	constexpr U32 lightCount = 1;
	{
		BufferInitInfo inf;
		inf.m_mapAccess = BufferMapAccessBit::kWrite;
		inf.m_usage = BufferUsageBit::kAllStorage;
		inf.m_size = sizeof(Light) * lightCount;

		lightBuffer = g_gr->newBuffer(inf);
		WeakArray<Light, PtrSize> lights = lightBuffer->map<Light>(0, lightCount, BufferMapAccessBit::kWrite);

		lights[0].m_min = geometries[GeomWhat::LIGHT].m_aabb.getMin().xyz();
		lights[0].m_max = geometries[GeomWhat::LIGHT].m_aabb.getMax().xyz();
		lights[0].m_intensity = Vec3(1.0f);

		lightBuffer->unmap();
	}

	// Draw
	constexpr U32 ITERATIONS = 100 * 8;
	for(U32 i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		const Mat4 viewMat = Mat4::lookAt(Vec3(278.0f, 278.0f, -800.0f), Vec3(278.0f, 278.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f)).getInverse();
		const Mat4 projMat = Mat4::calculatePerspectiveProjectionMatrix(toRad(40.0f) * WIDTH / HEIGHT, toRad(40.0f), 0.01f, 2000.0f);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		if(i == 0)
		{
			for(const Geom& g : geometries)
			{
				setAccelerationStructureBarrier(cmdb, g.m_blas, AccelerationStructureUsageBit::kNone, AccelerationStructureUsageBit::kBuild);
			}

			for(const Geom& g : geometries)
			{
				BufferInitInfo scratchInit;
				scratchInit.m_size = g.m_blas->getBuildScratchBufferSize();
				scratchInit.m_usage = BufferUsageBit::kAccelerationStructureBuildScratch;
				BufferPtr scratchBuff = GrManager::getSingleton().newBuffer(scratchInit);
				cmdb->buildAccelerationStructure(g.m_blas.get(), BufferView(scratchBuff.get()));
			}

			for(const Geom& g : geometries)
			{
				setAccelerationStructureBarrier(cmdb, g.m_blas, AccelerationStructureUsageBit::kBuild, AccelerationStructureUsageBit::kAttach);
			}

			setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kNone, AccelerationStructureUsageBit::kBuild);
			BufferInitInfo scratchInit;
			scratchInit.m_size = tlas->getBuildScratchBufferSize();
			scratchInit.m_usage = BufferUsageBit::kAccelerationStructureBuildScratch;
			BufferPtr scratchBuff = GrManager::getSingleton().newBuffer(scratchInit);
			cmdb->buildAccelerationStructure(tlas.get(), BufferView(scratchBuff.get()));
			setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kBuild, AccelerationStructureUsageBit::kTraceRaysRead);
		}

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		TextureViewPtr presentView;
		{

			TextureViewInitInfo inf;
			inf.m_texture = presentTex.get();
			presentView = g_gr->newTextureView(inf);
		}

		TextureViewPtr offscreenView, offscreenHistoryView;
		{

			TextureViewInitInfo inf;
			inf.m_texture = offscreenRts[i & 1].get();
			offscreenView = g_gr->newTextureView(inf);

			inf.m_texture = offscreenRts[(i + 1) & 1].get();
			offscreenHistoryView = g_gr->newTextureView(inf);
		}

		setTextureBarrier(cmdb, offscreenRts[i & 1], TextureUsageBit::kNone, TextureUsageBit::kUavTraceRaysWrite, TextureSubresourceInfo());
		setTextureBarrier(cmdb, offscreenRts[(i + 1) & 1], TextureUsageBit::kUavComputeRead, TextureUsageBit::kUavTraceRaysRead,
						  TextureSubresourceInfo());

		cmdb->bindStorageBuffer(0, 0, BufferView(modelBuffer.get()));
		cmdb->bindStorageBuffer(0, 1, BufferView(lightBuffer.get()));
		cmdb->bindAccelerationStructure(1, 0, tlas.get());
		cmdb->bindStorageTexture(1, 1, offscreenHistoryView.get());
		cmdb->bindStorageTexture(1, 2, offscreenView.get());

		cmdb->bindShaderProgram(rtProg.get());

		PushConstants pc;
		pc.m_vp = projMat * viewMat;
		pc.m_cameraPos = Vec3(278.0f, 278.0f, -800.0f);
		pc.m_lightCount = lightCount;
		pc.m_frame = i;

		cmdb->setFastConstants(&pc, sizeof(pc));

		const U32 sbtRecordSize = g_gr->getDeviceCapabilities().m_sbtRecordAlignment;
		cmdb->traceRays(BufferView(sbt.get()), sbtRecordSize, U32(GeomWhat::kCount) * 2, 2, WIDTH, HEIGHT, 1);

		// Copy to present
		setTextureBarrier(cmdb, offscreenRts[i & 1], TextureUsageBit::kUavTraceRaysWrite, TextureUsageBit::kUavComputeRead,
						  TextureSubresourceInfo());
		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kNone, TextureUsageBit::kUavCompute, TextureSubresourceInfo());

		cmdb->bindStorageTexture(0, 0, offscreenView.get());
		cmdb->bindStorageTexture(0, 1, presentView.get());

		cmdb->bindShaderProgram(copyToPresentProg.get());
		const U32 sizeX = (WIDTH + 8 - 1) / 8;
		const U32 sizeY = (HEIGHT + 8 - 1) / 8;
		cmdb->dispatchCompute(sizeX, sizeY, 1);

		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kUavCompute, TextureUsageBit::kPresent, TextureSubresourceInfo());

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 60.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END();
#endif
}

ANKI_TEST(Gr, AsyncCompute)
{
#if 0
	COMMON_BEGIN()

	constexpr U32 ARRAY_SIZE = 1000 * 1024 * 8;

	// Create the counting program
	static const char* PROG_SRC = R"(
layout(local_size_x = 8) in;

layout(binding = 0, std430) buffer b_buff
{
	U32 u_counters[];
};

void main()
{
	for(U32 i = 0u; i < gl_LocalInvocationID.x * 20u; ++i)
	{
		atomicAdd(u_counters[gl_GlobalInvocationID.x], i + 1u);
	}
})";

	ShaderPtr shader = createShader(PROG_SRC, ShaderType::kCompute, *g_gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_computeShader = shader.get();
	ShaderProgramPtr incrementProg = g_gr->newShaderProgram(sprogInit);

	// Create the check program
	static const char* CHECK_SRC = R"(
layout(local_size_x = 8) in;

layout(binding = 0, std430) buffer b_buff
{
	U32 u_counters[];
};

void main()
{
	// Walk the atomics in reverse to make sure that this dispatch won't overlap with the previous one
	const U32 newGlobalInvocationID = gl_NumWorkGroups.x * gl_WorkGroupSize.x - gl_GlobalInvocationID.x - 1u;

	U32 expectedVal = 0u;
	for(U32 i = 0u; i < (newGlobalInvocationID % gl_WorkGroupSize.x) * 20u; ++i)
	{
		expectedVal += i + 1u;
	}

	atomicCompSwap(u_counters[newGlobalInvocationID], expectedVal, 4u);
})";

	shader = createShader(CHECK_SRC, ShaderType::kCompute, *g_gr);
	sprogInit.m_computeShader = shader.get();
	ShaderProgramPtr checkProg = g_gr->newShaderProgram(sprogInit);

	// Create buffers
	BufferInitInfo info;
	info.m_size = sizeof(U32) * ARRAY_SIZE;
	info.m_usage = BufferUsageBit::kAllCompute;
	info.m_mapAccess = BufferMapAccessBit::kWrite | BufferMapAccessBit::kRead;
	BufferPtr atomicsBuffer = g_gr->newBuffer(info);
	U32* values = static_cast<U32*>(atomicsBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kRead | BufferMapAccessBit::kWrite));
	memset(values, 0, info.m_size);

	// Pre-create some CPU result buffers
	DynamicArray<U32> atomicsBufferCpu;
	atomicsBufferCpu.resize(ARRAY_SIZE);
	DynamicArray<U32> expectedResultsBufferCpu;
	expectedResultsBufferCpu.resize(ARRAY_SIZE);
	for(U32 i = 0; i < ARRAY_SIZE; ++i)
	{
		const U32 localInvocation = i % 8;
		U32 expectedVal = 4;
		for(U32 j = 0; j < localInvocation * 20; ++j)
		{
			expectedVal += j + 1;
		}

		expectedResultsBufferCpu[i] = expectedVal;
	}

	// Create the 1st command buffer
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr incrementCmdb = g_gr->newCommandBuffer(cinit);
	incrementCmdb->bindShaderProgram(incrementProg.get());
	incrementCmdb->bindStorageBuffer(0, 0, BufferView(atomicsBuffer.get()));
	incrementCmdb->dispatchCompute(ARRAY_SIZE / 8, 1, 1);

	// Create the 2nd command buffer
	cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr checkCmdb = g_gr->newCommandBuffer(cinit);
	checkCmdb->bindShaderProgram(checkProg.get());
	checkCmdb->bindStorageBuffer(0, 0, BufferView(atomicsBuffer.get()));
	checkCmdb->dispatchCompute(ARRAY_SIZE / 8, 1, 1);

	// Create the 3rd command buffer
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr incrementCmdb2 = g_gr->newCommandBuffer(cinit);
	incrementCmdb2->bindShaderProgram(incrementProg.get());
	incrementCmdb2->bindStorageBuffer(0, 0, BufferView(atomicsBuffer.get()));
	incrementCmdb2->dispatchCompute(ARRAY_SIZE / 8, 1, 1);

	// Submit
#	if 1
	FencePtr fence;
	incrementCmdb->endRecording();
	GrManager::getSingleton().submit(incrementCmdb.get(), {}, &fence);
	checkCmdb->endRecording();
	Fence* pFence = fence.get();
	GrManager::getSingleton().submit(checkCmdb.get(), {&pFence, 1}, &fence);
	incrementCmdb2->endRecording();
	pFence = fence.get();
	GrManager::getSingleton().submit(incrementCmdb2.get(), {&pFence, 1}, &fence);
	fence->clientWait(kMaxSecond);
#	else
	incrementCmdb->flush();
	gr->finish();
	checkCmdb->flush();
	gr->finish();
	incrementCmdb2->flush();
	gr->finish();
#	endif

	// Verify
	memcpy(atomicsBufferCpu.getBegin(), values, atomicsBufferCpu.getSizeInBytes());
	Bool correct = true;
	for(U32 i = 0; i < ARRAY_SIZE; ++i)
	{
		correct = correct && atomicsBufferCpu[i] == expectedResultsBufferCpu[i];
		if(!correct)
		{
			printf("%u!=%u %u\n", atomicsBufferCpu[i], expectedResultsBufferCpu[i], i);
			break;
		}
	}
	atomicsBuffer->unmap();
	ANKI_TEST_EXPECT_EQ(correct, true);

	COMMON_END()
#endif
}
