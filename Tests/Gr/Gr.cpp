
// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>
#include <AnKi/Core/NativeWindow.h>
#include <AnKi/Input/Input.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Resource/TransferGpuAllocator.h>
#include <AnKi/ShaderCompiler/Glslang.h>
#include <AnKi/ShaderCompiler/ShaderProgramParser.h>
#include <AnKi/Collision/Aabb.h>
#include <ctime>

using namespace anki;

const U WIDTH = 1024;
const U HEIGHT = 768;

static const char* VERT_QUAD_STRIP_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out Vec2 out_uv;

void main()
{
	const vec2 POSITIONS[4] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));
	gl_Position = vec4(POSITIONS[gl_VertexID % 4], 0.0, 1.0);
	out_uv = gl_Position.xy / 2.0 + 0.5;
})";

static const char* VERT_UBO_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(set = 0, binding = 0) uniform u0_
{
	vec4 u_color[3];
};

layout(set = 0, binding = 1) uniform u1_
{
	vec4 u_rotation2d;
};

layout(location = 0) out vec3 out_color;

void main()
{
	out_color = u_color[gl_VertexID].rgb;

	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, 1.0), vec2(0.0, -1.0), vec2(1.0, 1.0));

	mat2 rot = mat2(
		u_rotation2d.x, u_rotation2d.y, u_rotation2d.z, u_rotation2d.w);
	vec2 pos = rot * POSITIONS[gl_VertexID % 3];

	gl_Position = vec4(pos, 0.0, 1.0);
})";

static const char* VERT_INP_SRC = R"(
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color0;
layout(location = 2) in vec3 in_color1;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec3 out_color0;
layout(location = 1) out vec3 out_color1;

void main()
{
	gl_Position = vec4(in_position, 1.0);

	out_color0 = in_color0;
	out_color1 = in_color1;
})";

static const char* VERT_QUAD_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec2 out_uv;

void main()
{
	const vec2 POSITIONS[6] =
		vec2[](vec2(-1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, -1.0),
		vec2(1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0));

	gl_Position = vec4(POSITIONS[gl_VertexID], 0.0, 1.0);
	out_uv = POSITIONS[gl_VertexID] / 2.0 + 0.5;
})";

static const char* VERT_MRT_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) in vec3 in_pos;

layout(set = 0, binding = 0, std140, row_major) uniform u0_
{
	mat4 u_mvp;
};

void main()
{
	gl_Position = u_mvp * vec4(in_pos, 1.0);
})";

static const char* FRAG_UBO_SRC = R"(layout (location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_color;

void main()
{
	out_color = vec4(in_color, 1.0);
})";

static const char* FRAG_INP_SRC = R"(layout (location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_color0;
layout(location = 1) in vec3 in_color1;

void main()
{
	out_color = vec4(in_color0 + in_color1, 1.0);
})";

static const char* FRAG_TEX_SRC = R"(layout (location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(set = 0, binding = 0) uniform sampler2D u_tex0;

void main()
{
	out_color = texture(u_tex0, in_uv);
})";

static const char* FRAG_TEX3D_SRC = R"(layout (location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform u0_
{
	vec4 u_uv;
};

layout(set = 0, binding = 1) uniform sampler3D u_tex;

void main()
{
	out_color = textureLod(u_tex, u_uv.xyz, u_uv.w);
})";

static const char* FRAG_MRT_SRC = R"(layout (location = 0) out vec4 out_color0;
layout (location = 1) out vec4 out_color1;

layout(set = 0, binding = 1, std140) uniform u1_
{
	vec4 u_color0;
	vec4 u_color1;
};

void main()
{
	out_color0 = u_color0;
	out_color1 = u_color1;
})";

static const char* FRAG_MRT2_SRC = R"(layout (location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(set = 0, binding = 0) uniform sampler2D u_tex0;
layout(set = 0, binding = 2) uniform sampler2D u_tex1;

void main()
{
	vec2 uv = in_uv;
#ifdef ANKI_VK
	uv.y = 1.0 - uv.y;
#endif
	float factor = uv.x;
	vec3 col0 = texture(u_tex0, uv).rgb;
	vec3 col1 = texture(u_tex1, uv).rgb;

	out_color = vec4(col1 + col0, 1.0);
})";

static const char* FRAG_SIMPLE_TEX_SRC = R"(
layout (location = 0) out vec4 out_color;
layout(location = 0) in vec2 in_uv;
layout(set = 0, binding = 0) uniform sampler2D u_tex0;

void main()
{
	out_color = textureLod(u_tex0, in_uv, 1.0);
})";

static const char* COMP_WRITE_IMAGE_SRC = R"(
layout(set = 0, binding = 0, rgba8) writeonly uniform image2D u_img;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(set = 1, binding = 0) buffer ss1_
{
	vec4 u_color;
};

void main()
{
	imageStore(u_img, ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y), u_color);
})";

static NativeWindow* g_win = nullptr;
static GrManager* g_gr = nullptr;
static StagingGpuMemoryPool* stagingMem = nullptr;
static Input* input = nullptr;

#define COMMON_BEGIN() \
	stagingMem = new StagingGpuMemoryPool(); \
	ConfigSet cfg(allocAligned, nullptr); \
	cfg.setWidth(WIDTH); \
	cfg.setHeight(HEIGHT); \
	cfg.setGrValidation(true); \
	cfg.setGrVsync(false); \
	cfg.setGrRayTracing(true); \
	cfg.setGrDebugMarkers(true); \
	g_win = createWindow(cfg); \
	ANKI_TEST_EXPECT_NO_ERR(Input::newInstance(allocAligned, nullptr, g_win, input)); \
	g_gr = createGrManager(&cfg, g_win); \
	ANKI_TEST_EXPECT_NO_ERR(stagingMem->init(g_gr, cfg)); \
	TransferGpuAllocator* transfAlloc = new TransferGpuAllocator(); \
	ANKI_TEST_EXPECT_NO_ERR(transfAlloc->init(128_MB, g_gr, &g_gr->getMemoryPool())); \
	while(true) \
	{

#define COMMON_END() \
	break; \
	} \
	g_gr->finish(); \
	delete transfAlloc; \
	delete stagingMem; \
	GrManager::deleteInstance(g_gr); \
	Input::deleteInstance(input); \
	NativeWindow::deleteInstance(g_win); \
	g_win = nullptr; \
	g_gr = nullptr; \
	stagingMem = nullptr;

static void* setUniforms(PtrSize size, CommandBufferPtr& cmdb, U32 set, U32 binding)
{
	StagingGpuMemoryToken token;
	void* ptr = stagingMem->allocateFrame(size, StagingGpuMemoryType::kUniform, token);
	cmdb->bindUniformBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	return ptr;
}

static void* setStorage(PtrSize size, CommandBufferPtr& cmdb, U32 set, U32 binding)
{
	StagingGpuMemoryToken token;
	void* ptr = stagingMem->allocateFrame(size, StagingGpuMemoryType::kStorage, token);
	cmdb->bindStorageBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	return ptr;
}

#define SET_UNIFORMS(type_, size_, cmdb_, set_, binding_) static_cast<type_>(setUniforms(size_, cmdb_, set_, binding_))
#define SET_STORAGE(type_, size_, cmdb_, set_, binding_) static_cast<type_>(setStorage(size_, cmdb_, set_, binding_))

#define UPLOAD_TEX_SURFACE(cmdb_, tex_, surf_, ptr_, size_, handle_) \
	do \
	{ \
		ANKI_TEST_EXPECT_NO_ERR(transfAlloc->allocate(size_, handle_)); \
		void* f = handle_.getMappedMemory(); \
		memcpy(f, ptr_, size_); \
		TextureViewPtr view = g_gr->newTextureView(TextureViewInitInfo(tex_, surf_)); \
		cmdb_->copyBufferToTextureView(handle_.getBuffer(), handle_.getOffset(), handle_.getRange(), view); \
	} while(0)

#define UPLOAD_TEX_VOL(cmdb_, tex_, vol_, ptr_, size_, handle_) \
	do \
	{ \
		ANKI_TEST_EXPECT_NO_ERR(transfAlloc->allocate(size_, handle_)); \
		void* f = handle_.getMappedMemory(); \
		memcpy(f, ptr_, size_); \
		TextureViewPtr view = g_gr->newTextureView(TextureViewInitInfo(tex_, vol_)); \
		cmdb_->copyBufferToTextureView(handle_.getBuffer(), handle_.getOffset(), handle_.getRange(), view); \
	} while(0)

constexpr Format kDsFormat = Format::kD24_Unorm_S8_Uint;

static ShaderProgramPtr createProgram(CString vertSrc, CString fragSrc, GrManager& gr)
{
	ShaderPtr vert = createShader(vertSrc, ShaderType::kVertex, gr);
	ShaderPtr frag = createShader(fragSrc, ShaderType::kFragment, gr);
	ShaderProgramInitInfo inf;
	inf.m_graphicsShaders[ShaderType::kVertex] = vert;
	inf.m_graphicsShaders[ShaderType::kFragment] = frag;
	return gr.newShaderProgram(inf);
}

static FramebufferPtr createColorFb(GrManager& gr, TexturePtr tex)
{
	TextureViewInitInfo init;
	init.m_texture = tex;
	TextureViewPtr view = gr.newTextureView(init);

	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 1;
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 1.0, 1.0}};
	fbinit.m_colorAttachments[0].m_textureView = view;

	return gr.newFramebuffer(fbinit);
}

static void createCube(GrManager& gr, BufferPtr& verts, BufferPtr& indices)
{
	static const Array<F32, 8 * 3> pos = {
		{1, 1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1}};

	static const Array<U16, 6 * 2 * 3> idx = {
		{0, 1, 3, 3, 1, 2, 1, 5, 6, 1, 6, 2, 7, 4, 0, 7, 0, 3, 6, 5, 7, 7, 5, 4, 0, 4, 5, 0, 5, 1, 3, 2, 6, 3, 6, 7}};

	verts = gr.newBuffer(BufferInitInfo(sizeof(pos), BufferUsageBit::kVertex, BufferMapAccessBit::kWrite));

	void* mapped = verts->map(0, sizeof(pos), BufferMapAccessBit::kWrite);
	memcpy(mapped, &pos[0], sizeof(pos));
	verts->unmap();

	indices = gr.newBuffer(BufferInitInfo(sizeof(idx), BufferUsageBit::kIndex, BufferMapAccessBit::kWrite));
	mapped = indices->map(0, sizeof(idx), BufferMapAccessBit::kWrite);
	memcpy(mapped, &idx[0], sizeof(idx));
	indices->unmap();
}

static void presentBarrierA(CommandBufferPtr cmdb, TexturePtr presentTex)
{
	TextureBarrierInfo barrier;
	barrier.m_previousUsage = TextureUsageBit::kNone;
	barrier.m_nextUsage = TextureUsageBit::kFramebufferWrite;
	barrier.m_texture = presentTex.get();
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
}

static void presentBarrierB(CommandBufferPtr cmdb, TexturePtr presentTex)
{
	TextureBarrierInfo barrier;
	barrier.m_previousUsage = TextureUsageBit::kFramebufferWrite;
	barrier.m_nextUsage = TextureUsageBit::kPresent;
	barrier.m_texture = presentTex.get();
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
}

static void setTextureSurfaceBarrier(CommandBufferPtr& cmdb, TexturePtr tex, TextureUsageBit before,
									 TextureUsageBit after, const TextureSurfaceInfo& surf)
{
	TextureBarrierInfo barrier;
	barrier.m_previousUsage = before;
	barrier.m_nextUsage = after;
	barrier.m_texture = tex.get();
	barrier.m_subresource = surf;

	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
}

static void setTextureBarrier(CommandBufferPtr& cmdb, TexturePtr tex, TextureUsageBit before, TextureUsageBit after,
							  const TextureSubresourceInfo& subresource)
{
	TextureBarrierInfo barrier;
	barrier.m_previousUsage = before;
	barrier.m_nextUsage = after;
	barrier.m_texture = tex.get();
	barrier.m_subresource = subresource;

	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
}

static void setTextureVolumeBarrier(CommandBufferPtr& cmdb, TexturePtr tex, TextureUsageBit before,
									TextureUsageBit after, const TextureVolumeInfo& vol)
{
	TextureBarrierInfo barrier;
	barrier.m_previousUsage = before;
	barrier.m_nextUsage = after;
	barrier.m_texture = tex.get();
	barrier.m_subresource = vol;

	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});
}

static void setBufferBarrier(CommandBufferPtr cmdb, BufferPtr buffer, BufferUsageBit before, BufferUsageBit after,
							 PtrSize offset, PtrSize range)
{
	BufferBarrierInfo barrier;
	barrier.m_previousUsage = before;
	barrier.m_nextUsage = after;
	barrier.m_offset = offset;
	barrier.m_size = range;
	barrier.m_buffer = buffer.get();

	cmdb->setPipelineBarrier({}, {&barrier, 1}, {});
}

static void setAccelerationStructureBarrier(CommandBufferPtr cmdb, AccelerationStructurePtr as,
											AccelerationStructureUsageBit before, AccelerationStructureUsageBit after)
{
	AccelerationStructureBarrierInfo barrier;
	barrier.m_previousUsage = before;
	barrier.m_nextUsage = after;
	barrier.m_as = as.get();

	cmdb->setPipelineBarrier({}, {}, {&barrier, 1});
}

ANKI_TEST(Gr, GrManager){COMMON_BEGIN() COMMON_END()}

ANKI_TEST(Gr, Shader)
{
	COMMON_BEGIN()

	ShaderPtr shader = createShader(FRAG_MRT_SRC, ShaderType::kFragment, *g_gr);

	COMMON_END()
}

ANKI_TEST(Gr, ShaderProgram)
{
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
}

ANKI_TEST(Gr, ClearScreen)
{
	COMMON_BEGIN()
	ANKI_TEST_LOGI("Expect to see a magenta background");

	U iterations = 100;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb, {TextureUsageBit::kFramebufferWrite}, {});
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

ANKI_TEST(Gr, SimpleDrawcall)
{
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

	ANKI_TEST_LOGI("Expect to see a grey triangle");
	ShaderProgramPtr prog = createProgram(kVertSrc, kFragSrc, *g_gr);

	const U ITERATIONS = 200;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->bindShaderProgram(prog);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb, {{TextureUsageBit::kFramebufferWrite}}, {});
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
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
		cmdb->drawArrays(PrimitiveTopology::TRIANGLE_STRIP, 4);
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
	init.m_usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kAllFramebuffer;
	init.m_height = RT_HEIGHT;
	init.m_width = RT_WIDTH;
	init.m_mipmapCount = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_type = TextureType::k2D;
	TexturePtr rt = g_gr->newTexture(init);

	TextureViewInitInfo viewInit(rt);
	TextureViewPtr texView = g_gr->newTextureView(viewInit);

	Array<FramebufferPtr, 4> fb;
	for(FramebufferPtr& f : fb)
	{
		TextureViewInitInfo viewInf(rt);
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
			setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kNone, TextureUsageBit::kFramebufferWrite,
									 TextureSurfaceInfo(0, 0, 0, 0));
			cmdb->beginRenderPass(fb[0], {{TextureUsageBit::kFramebufferWrite}}, {});
			cmdb->endRenderPass();
			setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kFramebufferWrite, TextureUsageBit::kSampledFragment,
									 TextureSurfaceInfo(0, 0, 0, 0));
			cmdb->flush();
		}

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		// Draw offscreen
		setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kSampledFragment, TextureUsageBit::kFramebufferWrite,
								 TextureSurfaceInfo(0, 0, 0, 0));
		auto vp = VIEWPORTS[(i / 30) % 4];
		cmdb->setViewport(vp[0], vp[1], vp[2], vp[3]);
		cmdb->setScissor(vp[0] + SCISSOR_MARGIN, vp[1] + SCISSOR_MARGIN, vp[2] - SCISSOR_MARGIN * 2,
						 vp[3] - SCISSOR_MARGIN * 2);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb[i % 4], {{TextureUsageBit::kFramebufferWrite}}, {}, vp[0] + RENDER_AREA_MARGIN,
							  vp[1] + RENDER_AREA_MARGIN, vp[2] - RENDER_AREA_MARGIN * 2,
							  vp[3] - RENDER_AREA_MARGIN * 2);
		cmdb->drawArrays(PrimitiveTopology::kTriangleStrip, 4);
		cmdb->endRenderPass();

		// Draw onscreen
		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->setScissor(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->bindShaderProgram(blitProg);
		setTextureSurfaceBarrier(cmdb, rt, TextureUsageBit::kFramebufferWrite, TextureUsageBit::kSampledFragment,
								 TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->bindTextureAndSampler(0, 0, texView, sampler);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb, {TextureUsageBit::kFramebufferWrite}, {});
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

ANKI_TEST(Gr, Buffer)
{
	COMMON_BEGIN()

	BufferInitInfo buffInit("a");
	buffInit.m_size = 512;
	buffInit.m_usage = BufferUsageBit::kAllUniform;
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
}

ANKI_TEST(Gr, DrawWithUniforms)
{
	COMMON_BEGIN()

	// A non-uploaded buffer
	BufferPtr b =
		g_gr->newBuffer(BufferInitInfo(sizeof(Vec4) * 3, BufferUsageBit::kAllUniform, BufferMapAccessBit::kWrite));

	Vec4* ptr = static_cast<Vec4*>(b->map(0, sizeof(Vec4) * 3, BufferMapAccessBit::kWrite));
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
	ptr[0] = Vec4(1.0, 0.0, 0.0, 0.0);
	ptr[1] = Vec4(0.0, 1.0, 0.0, 0.0);
	ptr[2] = Vec4(0.0, 0.0, 1.0, 0.0);
	b->unmap();

	// Progm
	ShaderProgramPtr prog = createProgram(VERT_UBO_SRC, FRAG_UBO_SRC, *g_gr);

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();
		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->bindShaderProgram(prog);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb, {TextureUsageBit::kFramebufferWrite}, {});

		cmdb->bindUniformBuffer(0, 0, b, 0, kMaxPtrSize);

		// Uploaded buffer
		Vec4* rotMat = SET_UNIFORMS(Vec4*, sizeof(Vec4), cmdb, 0, 1);
		F32 angle = toRad(360.0f / F32(ITERATION_COUNT) * F32(iterations));
		(*rotMat)[0] = cos(angle);
		(*rotMat)[1] = -sin(angle);
		(*rotMat)[2] = sin(angle);
		(*rotMat)[3] = cos(angle);

		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

ANKI_TEST(Gr, DrawWithVertex)
{
	COMMON_BEGIN()

	// The buffers
	struct Vert
	{
		Vec3 m_pos;
		Array<U8, 4> m_color;
	};
	static_assert(sizeof(Vert) == sizeof(Vec4), "See file");

	BufferPtr b =
		g_gr->newBuffer(BufferInitInfo(sizeof(Vert) * 3, BufferUsageBit::kVertex, BufferMapAccessBit::kWrite));

	Vert* ptr = static_cast<Vert*>(b->map(0, sizeof(Vert) * 3, BufferMapAccessBit::kWrite));
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

	ptr[0].m_pos = Vec3(-1.0, 1.0, 0.0);
	ptr[1].m_pos = Vec3(0.0, -1.0, 0.0);
	ptr[2].m_pos = Vec3(1.0, 1.0, 0.0);

	ptr[0].m_color = {{255, 0, 0}};
	ptr[1].m_color = {{0, 255, 0}};
	ptr[2].m_color = {{0, 0, 255}};
	b->unmap();

	BufferPtr c =
		g_gr->newBuffer(BufferInitInfo(sizeof(Vec3) * 3, BufferUsageBit::kVertex, BufferMapAccessBit::kWrite));

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

		cmdb->bindVertexBuffer(0, b, 0, sizeof(Vert));
		cmdb->bindVertexBuffer(1, c, 0, sizeof(Vec3));
		cmdb->setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);
		cmdb->setVertexAttribute(1, 0, Format::kR8G8B8_Unorm, sizeof(Vec3));
		cmdb->setVertexAttribute(2, 1, Format::kR32G32B32_Sfloat, 0);

		cmdb->setViewport(0, 0, g_win->getWidth(), g_win->getHeight());
		cmdb->setPolygonOffset(0.0, 0.0);
		cmdb->bindShaderProgram(prog);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb, {TextureUsageBit::kFramebufferWrite}, {});
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

ANKI_TEST(Gr, Sampler)
{
	COMMON_BEGIN()

	SamplerInitInfo init;

	SamplerPtr b = g_gr->newSampler(init);

	COMMON_END()
}

ANKI_TEST(Gr, Texture)
{
	COMMON_BEGIN()

	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::kR8G8B8_Unorm;
	init.m_usage = TextureUsageBit::kSampledFragment;
	init.m_height = 4;
	init.m_width = 4;
	init.m_mipmapCount = 2;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_type = TextureType::k2D;

	TexturePtr b = g_gr->newTexture(init);

	TextureViewInitInfo view(b);
	TextureViewPtr v = g_gr->newTextureView(view);

	COMMON_END()
}

ANKI_TEST(Gr, DrawWithTexture)
{
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
	init.m_usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kTransferDestination;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapCount = 2;
	init.m_samples = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::k2D;

	TexturePtr a = g_gr->newTexture(init);

	TextureViewPtr aView = g_gr->newTextureView(TextureViewInitInfo(a));

	//
	// Create texture B
	//
	init.m_width = 4;
	init.m_height = 4;
	init.m_mipmapCount = 3;
	init.m_usage =
		TextureUsageBit::kSampledFragment | TextureUsageBit::kTransferDestination | TextureUsageBit::kGenerateMipmaps;

	TexturePtr b = g_gr->newTexture(init);

	TextureViewPtr bView = g_gr->newTextureView(TextureViewInitInfo(b));

	//
	// Upload all textures
	//
	Array<U8, 2 * 2 * 3> mip0 = {{255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255}};

	Array<U8, 3> mip1 = {{128, 128, 128}};

	Array<U8, 4 * 4 * 3> bmip0 = {{255, 0,   0,   0,   255, 0,   0,   0,   255, 255, 255, 0,   255, 0,   255, 0,
								   255, 255, 255, 255, 255, 128, 0,   0,   0,   128, 0,   0,   0,   128, 128, 128,
								   0,   128, 0,   128, 0,   128, 128, 128, 128, 128, 255, 128, 0,   0,   128, 255}};

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbinit);

	// Set barriers
	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kSampledFragment, TextureUsageBit::kTransferDestination,
							 TextureSurfaceInfo(0, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kSampledFragment, TextureUsageBit::kTransferDestination,
							 TextureSurfaceInfo(1, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, b, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
							 TextureSurfaceInfo(0, 0, 0, 0));

	TransferGpuAllocatorHandle handle0, handle1, handle2;
	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceInfo(0, 0, 0, 0), &mip0[0], sizeof(mip0), handle0);

	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceInfo(1, 0, 0, 0), &mip1[0], sizeof(mip1), handle1);

	UPLOAD_TEX_SURFACE(cmdb, b, TextureSurfaceInfo(0, 0, 0, 0), &bmip0[0], sizeof(bmip0), handle2);

	// Gen mips
	setTextureSurfaceBarrier(cmdb, b, TextureUsageBit::kTransferDestination, TextureUsageBit::kGenerateMipmaps,
							 TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->generateMipmaps2d(g_gr->newTextureView(TextureViewInitInfo(b)));

	// Set barriers
	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledFragment,
							 TextureSurfaceInfo(0, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, a, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledFragment,
							 TextureSurfaceInfo(1, 0, 0, 0));

	for(U32 i = 0; i < 3; ++i)
	{
		setTextureSurfaceBarrier(cmdb, b, TextureUsageBit::kGenerateMipmaps, TextureUsageBit::kSampledFragment,
								 TextureSurfaceInfo(i, 0, 0, 0));
	}

	FencePtr fence;
	cmdb->flush({}, &fence);
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
		cmdb->bindShaderProgram(prog);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(fb, {TextureUsageBit::kFramebufferWrite}, {});

		Vec4 pc(F32(g_win->getWidth()), F32(g_win->getHeight()), 0.0f, 0.0f);
		cmdb->setPushConstants(&pc, sizeof(pc));

		cmdb->bindTextureAndSampler(0, 0, aView, sampler);
		cmdb->bindTextureAndSampler(0, 1, bView, sampler);
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);
		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

static void drawOffscreenDrawcalls([[maybe_unused]] GrManager& gr, ShaderProgramPtr prog, CommandBufferPtr cmdb,
								   U32 viewPortSize, BufferPtr indexBuff, BufferPtr vertBuff)
{
	static F32 ang = -2.5f;
	ang += toRad(2.5f);

	Mat4 viewMat(Vec4(0.0, 0.0, 5.0, 1.0), Mat3::getIdentity(), 1.0f);
	viewMat.invert();

	Mat4 projMat = Mat4::calculatePerspectiveProjectionMatrix(toRad(60.0f), toRad(60.0f), 0.1f, 100.0f);

	Mat4 modelMat(Vec4(-0.5, -0.5, 0.0, 1.0), Mat3(Euler(ang, ang / 2.0f, ang / 3.0f)), 1.0f);

	Mat4* mvp = SET_UNIFORMS(Mat4*, sizeof(*mvp), cmdb, 0, 0);
	*mvp = projMat * viewMat * modelMat;

	Vec4* color = SET_UNIFORMS(Vec4*, sizeof(*color) * 2, cmdb, 0, 1);
	*color++ = Vec4(1.0, 0.0, 0.0, 0.0);
	*color = Vec4(0.0, 1.0, 0.0, 0.0);

	cmdb->bindVertexBuffer(0, vertBuff, 0, sizeof(Vec3));
	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);
	cmdb->bindShaderProgram(prog);
	cmdb->bindIndexBuffer(indexBuff, 0, IndexType::kU16);
	cmdb->setViewport(0, 0, viewPortSize, viewPortSize);
	cmdb->drawElements(PrimitiveTopology::kTriangles, 6 * 2 * 3);

	// 2nd draw
	modelMat = Mat4(Vec4(0.5, 0.5, 0.0, 1.0), Mat3(Euler(ang * 2.0f, ang, ang / 3.0f * 2.0f)), 1.0f);

	mvp = SET_UNIFORMS(Mat4*, sizeof(*mvp), cmdb, 0, 0);
	*mvp = projMat * viewMat * modelMat;

	color = SET_UNIFORMS(Vec4*, sizeof(*color) * 2, cmdb, 0, 1);
	*color++ = Vec4(0.0, 0.0, 1.0, 0.0);
	*color = Vec4(0.0, 1.0, 1.0, 0.0);

	cmdb->drawElements(PrimitiveTopology::kTriangles, 6 * 2 * 3);
}

static void drawOffscreen(GrManager& gr, Bool useSecondLevel)
{
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
	init.m_usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kAllFramebuffer;
	init.m_height = TEX_SIZE;
	init.m_width = TEX_SIZE;
	init.m_type = TextureType::k2D;

	TexturePtr col0 = gr.newTexture(init);
	TexturePtr col1 = gr.newTexture(init);

	TextureViewPtr col0View = gr.newTextureView(TextureViewInitInfo(col0));
	TextureViewPtr col1View = gr.newTextureView(TextureViewInitInfo(col1));

	init.m_format = kDsFormat;
	TexturePtr dp = gr.newTexture(init);

	//
	// Create FB
	//
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 2;
	fbinit.m_colorAttachments[0].m_textureView = gr.newTextureView(TextureViewInitInfo(col0));
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{0.1f, 0.0f, 0.0f, 0.0f}};
	fbinit.m_colorAttachments[1].m_textureView = gr.newTextureView(TextureViewInitInfo(col1));
	fbinit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0f, 0.1f, 0.0f, 0.0f}};
	TextureViewInitInfo viewInit(dp);
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

		setTextureSurfaceBarrier(cmdb, col0, TextureUsageBit::kNone, TextureUsageBit::kFramebufferWrite,
								 TextureSurfaceInfo(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, col1, TextureUsageBit::kNone, TextureUsageBit::kFramebufferWrite,
								 TextureSurfaceInfo(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, dp, TextureUsageBit::kNone, TextureUsageBit::kAllFramebuffer,
								 TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->beginRenderPass(fb, {{TextureUsageBit::kFramebufferWrite, TextureUsageBit::kFramebufferWrite}},
							  TextureUsageBit::kAllFramebuffer);

		if(!useSecondLevel)
		{
			drawOffscreenDrawcalls(gr, prog, cmdb, TEX_SIZE, indices, verts);
		}
		else
		{
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kSecondLevel | CommandBufferFlag::kGeneralWork;
			cinit.m_framebuffer = fb;
			CommandBufferPtr cmdb2 = gr.newCommandBuffer(cinit);

			drawOffscreenDrawcalls(gr, prog, cmdb2, TEX_SIZE, indices, verts);

			cmdb2->flush();

			CommandBuffer* pCmdb = cmdb2.get();
			cmdb->pushSecondLevelCommandBuffers({&pCmdb, 1});
		}

		cmdb->endRenderPass();

		setTextureSurfaceBarrier(cmdb, col0, TextureUsageBit::kFramebufferWrite, TextureUsageBit::kSampledFragment,
								 TextureSurfaceInfo(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, col1, TextureUsageBit::kFramebufferWrite, TextureUsageBit::kSampledFragment,
								 TextureSurfaceInfo(0, 0, 0, 0));
		setTextureSurfaceBarrier(cmdb, dp, TextureUsageBit::kAllFramebuffer, TextureUsageBit::kSampledFragment,
								 TextureSurfaceInfo(0, 0, 0, 0));

		// Draw quad
		TexturePtr presentTex = gr.acquireNextPresentableTexture();
		FramebufferPtr dfb = createColorFb(gr, presentTex);

		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb, {TextureUsageBit::kFramebufferWrite}, {});
		cmdb->bindShaderProgram(resolveProg);
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindTextureAndSampler(0, 0, col0View, sampler);
		cmdb->bindTextureAndSampler(0, 2, col1View, sampler);
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->flush();

		// End
		gr.swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}
}

ANKI_TEST(Gr, DrawOffscreen)
{
	COMMON_BEGIN()

	drawOffscreen(*g_gr, false);

	COMMON_END()
}

ANKI_TEST(Gr, DrawWithSecondLevel)
{
	COMMON_BEGIN()

	drawOffscreen(*g_gr, true);

	COMMON_END()
}

ANKI_TEST(Gr, ImageLoadStore)
{
	COMMON_BEGIN()

	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kBase;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	TextureInitInfo init;
	init.m_width = init.m_height = 4;
	init.m_mipmapCount = 2;
	init.m_usage =
		TextureUsageBit::kTransferDestination | TextureUsageBit::kAllSampled | TextureUsageBit::kImageComputeWrite;
	init.m_type = TextureType::k2D;
	init.m_format = Format::kR8G8B8A8_Unorm;

	TexturePtr tex = g_gr->newTexture(init);

	TextureViewInitInfo viewInit(tex);
	viewInit.m_firstMipmap = 1;
	viewInit.m_mipmapCount = 1;
	TextureViewPtr view = g_gr->newTextureView(viewInit);

	// Prog
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_SIMPLE_TEX_SRC, *g_gr);

	// Create shader & compute prog
	ShaderPtr shader = createShader(COMP_WRITE_IMAGE_SRC, ShaderType::kCompute, *g_gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_computeShader = shader;
	ShaderProgramPtr compProg = g_gr->newShaderProgram(sprogInit);

	// Write texture data
	CommandBufferInitInfo cmdbinit;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbinit);

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
							 TextureSurfaceInfo(0, 0, 0, 0));

	ClearValue clear;
	clear.m_colorf = {{0.0, 1.0, 0.0, 1.0}};
	TextureViewInitInfo viewInit2(tex, TextureSurfaceInfo(0, 0, 0, 0));
	cmdb->clearTextureView(g_gr->newTextureView(viewInit2), clear);

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledFragment,
							 TextureSurfaceInfo(0, 0, 0, 0));

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
							 TextureSurfaceInfo(1, 0, 0, 0));

	clear.m_colorf = {{0.0, 0.0, 1.0, 1.0}};
	TextureViewInitInfo viewInit3(tex, TextureSurfaceInfo(1, 0, 0, 0));
	cmdb->clearTextureView(g_gr->newTextureView(viewInit3), clear);

	setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kTransferDestination, TextureUsageBit::kImageComputeWrite,
							 TextureSurfaceInfo(1, 0, 0, 0));

	cmdb->flush();

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		CommandBufferInitInfo cinit;
		cinit.m_flags =
			CommandBufferFlag::kGeneralWork | CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		// Write image
		Vec4* col = SET_STORAGE(Vec4*, sizeof(*col), cmdb, 1, 0);
		*col = Vec4(F32(iterations) / F32(ITERATION_COUNT));

		setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kImageComputeWrite,
								 TextureSurfaceInfo(1, 0, 0, 0));
		cmdb->bindShaderProgram(compProg);
		cmdb->bindImage(0, 0, view);
		cmdb->dispatchCompute(WIDTH / 2, HEIGHT / 2, 1);
		setTextureSurfaceBarrier(cmdb, tex, TextureUsageBit::kImageComputeWrite, TextureUsageBit::kSampledFragment,
								 TextureSurfaceInfo(1, 0, 0, 0));

		// Present image
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);

		cmdb->bindShaderProgram(prog);
		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr dfb = createColorFb(*g_gr, presentTex);
		presentBarrierA(cmdb, presentTex);
		cmdb->beginRenderPass(dfb, {TextureUsageBit::kFramebufferWrite}, {});
		cmdb->bindTextureAndSampler(0, 0, g_gr->newTextureView(TextureViewInitInfo(tex)), sampler);
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 6);
		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->flush();

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
}

ANKI_TEST(Gr, 3DTextures)
{
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
	init.m_usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kTransferDestination;
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
	Array<U8, 2 * 2 * 2 * 4> mip0 = {{255, 0, 0,   0, 0, 255, 0,   0, 0,   0,   255, 0, 255, 255, 0, 0,
									  255, 0, 255, 0, 0, 255, 255, 0, 255, 255, 255, 0, 0,   0,   0, 0}};

	Array<U8, 4> mip1 = {{128, 128, 128, 0}};

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbinit);

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
							TextureVolumeInfo(0));

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
							TextureVolumeInfo(1));

	TransferGpuAllocatorHandle handle0, handle1;
	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeInfo(0), &mip0[0], sizeof(mip0), handle0);

	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeInfo(1), &mip1[0], sizeof(mip1), handle1);

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledFragment,
							TextureVolumeInfo(0));

	setTextureVolumeBarrier(cmdb, a, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledFragment,
							TextureVolumeInfo(1));

	FencePtr fence;
	cmdb->flush({}, &fence);
	transfAlloc->release(handle0, fence);
	transfAlloc->release(handle1, fence);

	//
	// Rest
	//
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_TEX3D_SRC, *g_gr);

	static Array<Vec4, 9> TEX_COORDS_LOD = {{Vec4(0, 0, 0, 0), Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(1, 1, 0, 0),
											 Vec4(0, 0, 1, 0), Vec4(1, 0, 1, 0), Vec4(0, 1, 1, 0), Vec4(1, 1, 1, 0),
											 Vec4(0, 0, 0, 1)}};

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
		cmdb->beginRenderPass(dfb, {TextureUsageBit::kFramebufferWrite}, {});

		cmdb->bindShaderProgram(prog);

		Vec4* uv = SET_UNIFORMS(Vec4*, sizeof(Vec4), cmdb, 0, 0);

		U32 idx = U32((F32(ITERATION_COUNT - iterations - 1) / F32(ITERATION_COUNT)) * F32(TEX_COORDS_LOD.getSize()));
		*uv = TEX_COORDS_LOD[idx];

		cmdb->bindTextureAndSampler(0, 1, g_gr->newTextureView(TextureViewInitInfo(a)), sampler);
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 6);

		cmdb->endRenderPass();
		presentBarrierB(cmdb, presentTex);

		cmdb->flush();

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
}

static RenderTargetDescription newRTDescr(CString name)
{
	RenderTargetDescription texInf(name);
	texInf.m_width = texInf.m_height = 16;
	texInf.m_usage = TextureUsageBit::kFramebufferWrite | TextureUsageBit::kSampledFragment;
	texInf.m_format = Format::kR8G8B8A8_Unorm;
	texInf.bake();
	return texInf;
}

ANKI_TEST(Gr, RenderGraph)
{
	COMMON_BEGIN()

	StackMemoryPool pool(allocAligned, nullptr, 2_MB);
	RenderGraphDescription descr(&pool);
	RenderGraphPtr rgraph = g_gr->newRenderGraph();

	const U GI_MIP_COUNT = 4;

	TextureInitInfo texI("dummy");
	texI.m_width = texI.m_height = 16;
	texI.m_usage = TextureUsageBit::kFramebufferWrite | TextureUsageBit::kSampledFragment;
	texI.m_format = Format::kR8G8B8A8_Unorm;
	TexturePtr dummyTex = g_gr->newTexture(texI);

	// SM
	RenderTargetHandle smScratchRt = descr.newRenderTarget(newRTDescr("SM scratch"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("SM");
		pass.newTextureDependency(smScratchRt, TextureUsageBit::kAllFramebuffer);
	}

	// SM to exponential SM
	RenderTargetHandle smExpRt = descr.importRenderTarget(dummyTex, TextureUsageBit::kSampledFragment);
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("ESM");
		pass.newTextureDependency(smScratchRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(smExpRt, TextureUsageBit::kFramebufferWrite);
	}

	// GI gbuff
	RenderTargetHandle giGbuffNormRt = descr.newRenderTarget(newRTDescr("GI GBuff norm"));
	RenderTargetHandle giGbuffDiffRt = descr.newRenderTarget(newRTDescr("GI GBuff diff"));
	RenderTargetHandle giGbuffDepthRt = descr.newRenderTarget(newRTDescr("GI GBuff depth"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("GI gbuff");
		pass.newTextureDependency(giGbuffNormRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(giGbuffDepthRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(giGbuffDiffRt, TextureUsageBit::kFramebufferWrite);
	}

	// GI light
	RenderTargetHandle giGiLightRt = descr.importRenderTarget(dummyTex, TextureUsageBit::kSampledFragment);
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, 0));

		GraphicsRenderPassDescription& pass =
			descr.newGraphicsRenderPass(StringRaii(&pool).sprintf("GI lp%u", faceIdx).toCString());
		pass.newTextureDependency(giGiLightRt, TextureUsageBit::kFramebufferWrite, subresource);
		pass.newTextureDependency(giGbuffNormRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(giGbuffDepthRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(giGbuffDiffRt, TextureUsageBit::kSampledFragment);
	}

	// GI light mips
	{
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass =
				descr.newGraphicsRenderPass(StringRaii(&pool).sprintf("GI mip%u", faceIdx).toCString());

			for(U32 mip = 0; mip < GI_MIP_COUNT; ++mip)
			{
				TextureSurfaceInfo surf(mip, 0, faceIdx, 0);
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
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("G-Buffer");
		pass.newTextureDependency(gbuffRt0, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(gbuffRt1, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(gbuffRt2, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(gbuffDepth, TextureUsageBit::kFramebufferWrite);
	}

	// Half depth
	RenderTargetHandle halfDepthRt = descr.newRenderTarget(newRTDescr("Depth/2"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("HalfDepth");
		pass.newTextureDependency(gbuffDepth, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(halfDepthRt, TextureUsageBit::kFramebufferWrite);
	}

	// Quarter depth
	RenderTargetHandle quarterDepthRt = descr.newRenderTarget(newRTDescr("Depth/4"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("QuarterDepth");
		pass.newTextureDependency(quarterDepthRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(halfDepthRt, TextureUsageBit::kSampledFragment);
	}

	// SSAO
	RenderTargetHandle ssaoRt = descr.newRenderTarget(newRTDescr("SSAO"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("SSAO main");
		pass.newTextureDependency(ssaoRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(quarterDepthRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(gbuffRt2, TextureUsageBit::kSampledFragment);

		RenderTargetHandle ssaoVBlurRt = descr.newRenderTarget(newRTDescr("SSAO tmp"));
		GraphicsRenderPassDescription& pass2 = descr.newGraphicsRenderPass("SSAO vblur");
		pass2.newTextureDependency(ssaoRt, TextureUsageBit::kSampledFragment);
		pass2.newTextureDependency(ssaoVBlurRt, TextureUsageBit::kFramebufferWrite);

		GraphicsRenderPassDescription& pass3 = descr.newGraphicsRenderPass("SSAO hblur");
		pass3.newTextureDependency(ssaoRt, TextureUsageBit::kFramebufferWrite);
		pass3.newTextureDependency(ssaoVBlurRt, TextureUsageBit::kSampledFragment);
	}

	// Volumetric
	RenderTargetHandle volRt = descr.newRenderTarget(newRTDescr("Vol"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Vol main");
		pass.newTextureDependency(volRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(quarterDepthRt, TextureUsageBit::kSampledFragment);

		RenderTargetHandle volVBlurRt = descr.newRenderTarget(newRTDescr("Vol tmp"));
		GraphicsRenderPassDescription& pass2 = descr.newGraphicsRenderPass("Vol vblur");
		pass2.newTextureDependency(volRt, TextureUsageBit::kSampledFragment);
		pass2.newTextureDependency(volVBlurRt, TextureUsageBit::kFramebufferWrite);

		GraphicsRenderPassDescription& pass3 = descr.newGraphicsRenderPass("Vol hblur");
		pass3.newTextureDependency(volRt, TextureUsageBit::kFramebufferWrite);
		pass3.newTextureDependency(volVBlurRt, TextureUsageBit::kSampledFragment);
	}

	// Forward shading
	RenderTargetHandle fsRt = descr.newRenderTarget(newRTDescr("FS"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Forward shading");
		pass.newTextureDependency(fsRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(halfDepthRt, TextureUsageBit::kSampledFragment | TextureUsageBit::kFramebufferRead);
		pass.newTextureDependency(volRt, TextureUsageBit::kSampledFragment);
	}

	// Light shading
	RenderTargetHandle lightRt = descr.importRenderTarget(dummyTex, TextureUsageBit::kNone);
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Light shading");

		pass.newTextureDependency(lightRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(gbuffRt0, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(gbuffRt1, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(gbuffRt2, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(gbuffDepth, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(smExpRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(giGiLightRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(ssaoRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(fsRt, TextureUsageBit::kSampledFragment);
	}

	// TAA
	RenderTargetHandle taaHistoryRt = descr.importRenderTarget(dummyTex, TextureUsageBit::kSampledFragment);
	RenderTargetHandle taaRt = descr.importRenderTarget(dummyTex, TextureUsageBit::kNone);
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Temporal AA");

		pass.newTextureDependency(lightRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(taaRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(taaHistoryRt, TextureUsageBit::kSampledFragment);
	}

	rgraph->compileNewGraph(descr, pool);
	COMMON_END()
}

/// Test workarounds for some unsupported formats
ANKI_TEST(Gr, VkWorkarounds)
{
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
	sinf.m_computeShader = comp;
	ShaderProgramPtr prog = g_gr->newShaderProgram(sinf);

	// Create the texture
	TextureInitInfo texInit;
	texInit.m_width = texInit.m_height = 8;
	texInit.m_format = Format::kR8G8B8_Uint;
	texInit.m_type = TextureType::k2D;
	texInit.m_usage = TextureUsageBit::kTransferDestination | TextureUsageBit::kAllSampled;
	texInit.m_mipmapCount = 2;
	TexturePtr tex = g_gr->newTexture(texInit);
	TextureViewPtr texView = g_gr->newTextureView(TextureViewInitInfo(tex));

	SamplerInitInfo samplerInit;
	SamplerPtr sampler = g_gr->newSampler(samplerInit);

	// Create the buffer to copy to the texture
	BufferPtr uploadBuff = g_gr->newBuffer(BufferInitInfo(PtrSize(texInit.m_width) * texInit.m_height * 3,
														  BufferUsageBit::kAllTransfer, BufferMapAccessBit::kWrite));
	U8* data = static_cast<U8*>(uploadBuff->map(0, uploadBuff->getSize(), BufferMapAccessBit::kWrite));
	for(U32 i = 0; i < texInit.m_width * texInit.m_height; ++i)
	{
		data[0] = U8(i);
		data[1] = U8(i + 1);
		data[2] = U8(i + 2);
		data += 3;
	}
	uploadBuff->unmap();

	BufferPtr uploadBuff2 = g_gr->newBuffer(BufferInitInfo(PtrSize(texInit.m_width >> 1) * (texInit.m_height >> 1) * 3,
														   BufferUsageBit::kAllTransfer, BufferMapAccessBit::kWrite));
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
	BufferPtr resultBuff =
		g_gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::kStorageComputeWrite, BufferMapAccessBit::kRead));

	// Upload data and test them
	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cmdbInit);

	TextureSubresourceInfo subresource;
	subresource.m_mipmapCount = texInit.m_mipmapCount;
	setTextureBarrier(cmdb, tex, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination, subresource);
	cmdb->copyBufferToTextureView(uploadBuff, 0, uploadBuff->getSize(),
								  g_gr->newTextureView(TextureViewInitInfo(tex, TextureSurfaceInfo(0, 0, 0, 0))));
	cmdb->copyBufferToTextureView(uploadBuff2, 0, uploadBuff2->getSize(),
								  g_gr->newTextureView(TextureViewInitInfo(tex, TextureSurfaceInfo(1, 0, 0, 0))));

	setTextureBarrier(cmdb, tex, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledCompute, subresource);
	cmdb->bindShaderProgram(prog);
	cmdb->bindTextureAndSampler(0, 0, texView, sampler);
	cmdb->bindStorageBuffer(0, 1, resultBuff, 0, resultBuff->getSize());
	cmdb->dispatchCompute(1, 1, 1);

	setBufferBarrier(cmdb, resultBuff, BufferUsageBit::kStorageComputeWrite, BufferUsageBit::kStorageComputeWrite, 0,
					 resultBuff->getSize());

	cmdb->flush();
	g_gr->finish();

	// Get the result
	UVec4* result = static_cast<UVec4*>(resultBuff->map(0, resultBuff->getSize(), BufferMapAccessBit::kRead));
	ANKI_TEST_EXPECT_EQ(result->x(), 2);
	ANKI_TEST_EXPECT_EQ(result->y(), 2);
	ANKI_TEST_EXPECT_EQ(result->z(), 2);
	ANKI_TEST_EXPECT_EQ(result->w(), 2);
	resultBuff->unmap();

	COMMON_END()
}

ANKI_TEST(Gr, SpecConsts)
{
	COMMON_BEGIN()

	static const char* VERT_SRC = R"(
layout(constant_id = 0) const int const0 = 0;
layout(constant_id = 2) const float const1 = 0.0;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) flat out int out_const0;
layout(location = 1) flat out float out_const1;

void main()
{
	vec2 uv = vec2(gl_VertexID & 1, gl_VertexID >> 1) * 2.0;
	vec2 pos = uv * 2.0 - 1.0;

	gl_Position = vec4(pos, 0.0, 1.0);

	out_const0 = const0;
	out_const1 = const1;
}
)";

	static const char* FRAG_SRC = R"(
layout(constant_id = 0) const int const0 = 0;
layout(constant_id = 1) const float const1 = 0.0;

layout(location = 0) flat in int in_const0;
layout(location = 1) flat in float in_const1;

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
		if(in_const0 != 2147483647 || in_const1 != 1234.5678 || const0 != -2147483647 || const1 != -1.0)
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

	ShaderPtr vert =
		createShader(VERT_SRC, ShaderType::kVertex, *g_gr,
					 Array<ShaderSpecializationConstValue, 3>{{ShaderSpecializationConstValue(2147483647),
															   ShaderSpecializationConstValue(-1.0f),
															   ShaderSpecializationConstValue(1234.5678f)}});
	ShaderPtr frag = createShader(FRAG_SRC, ShaderType::kFragment, *g_gr,
								  Array<ShaderSpecializationConstValue, 2>{{ShaderSpecializationConstValue(-2147483647),
																			ShaderSpecializationConstValue(-1.0f)}});
	ShaderProgramInitInfo sinf;
	sinf.m_graphicsShaders[ShaderType::kVertex] = vert;
	sinf.m_graphicsShaders[ShaderType::kFragment] = frag;
	ShaderProgramPtr prog = g_gr->newShaderProgram(sinf);

	// Create the result buffer
	BufferPtr resultBuff =
		g_gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::kStorageComputeWrite, BufferMapAccessBit::kRead));

	// Draw

	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

	cmdb->setViewport(0, 0, WIDTH, HEIGHT);
	cmdb->bindShaderProgram(prog);
	cmdb->bindStorageBuffer(0, 0, resultBuff, 0, resultBuff->getSize());
	TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
	FramebufferPtr dfb = createColorFb(*g_gr, presentTex);
	presentBarrierA(cmdb, presentTex);
	cmdb->beginRenderPass(dfb, {TextureUsageBit::kFramebufferWrite}, {});
	cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
	cmdb->endRenderPass();
	presentBarrierB(cmdb, presentTex);
	cmdb->flush();

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
}

ANKI_TEST(Gr, PushConsts)
{
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
	BufferPtr resultBuff = g_gr->newBuffer(BufferInitInfo(
		sizeof(UVec4), BufferUsageBit::kAllStorage | BufferUsageBit::kTransferDestination, BufferMapAccessBit::kRead));

	// Draw
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

	cmdb->fillBuffer(resultBuff, 0, resultBuff->getSize(), 0);
	setBufferBarrier(cmdb, resultBuff, BufferUsageBit::kTransferDestination, BufferUsageBit::kStorageFragmentWrite, 0,
					 resultBuff->getSize());

	cmdb->setViewport(0, 0, WIDTH, HEIGHT);
	cmdb->bindShaderProgram(prog);

	struct PushConstants
	{
		Vec4 m_color = Vec4(1.0, 0.0, 1.0, 0.0);
		IVec4 m_icolor = IVec4(-1, 1, 2147483647, -2147483647);
		Vec4 m_arr[2] = {Vec4(1, 2, 3, 4), Vec4(10, 20, 30, 40)};
		Mat4 m_mat = Mat4(0.0f);
	} pc;
	pc.m_mat(0, 1) = 0.5f;
	cmdb->setPushConstants(&pc, sizeof(pc));

	cmdb->bindStorageBuffer(0, 0, resultBuff, 0, resultBuff->getSize());
	TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
	FramebufferPtr dfb = createColorFb(*g_gr, presentTex);
	presentBarrierA(cmdb, presentTex);
	cmdb->beginRenderPass(dfb, {TextureUsageBit::kFramebufferWrite}, {});
	cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
	cmdb->endRenderPass();
	presentBarrierB(cmdb, presentTex);
	cmdb->flush();

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
}

ANKI_TEST(Gr, BindingWithArray)
{
	COMMON_BEGIN()

	// Create result buffer
	BufferPtr resBuff =
		g_gr->newBuffer(BufferInitInfo(sizeof(Vec4), BufferUsageBit::kAllCompute, BufferMapAccessBit::kRead));

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
	sprogInit.m_computeShader = shader;
	ShaderProgramPtr prog = g_gr->newShaderProgram(sprogInit);

	// Run
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

	for(U32 i = 0; i < uniformBuffers.getSize(); ++i)
	{
		cmdb->bindUniformBuffer(0, 0, uniformBuffers[i], 0, kMaxPtrSize, i);
	}

	cmdb->bindStorageBuffer(0, 1, resBuff, 0, kMaxPtrSize);

	cmdb->bindShaderProgram(prog);
	cmdb->dispatchCompute(1, 1, 1);

	cmdb->flush();
	g_gr->finish();

	// Check result
	Vec4* res = static_cast<Vec4*>(resBuff->map(0, sizeof(Vec4), BufferMapAccessBit::kRead));

	ANKI_TEST_EXPECT_EQ(res->x(), 28.0f);
	ANKI_TEST_EXPECT_EQ(res->y(), 32.0f);
	ANKI_TEST_EXPECT_EQ(res->z(), 36.0f);
	ANKI_TEST_EXPECT_EQ(res->w(), 40.0f);

	resBuff->unmap();

	COMMON_END();
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
	texInit.m_usage = TextureUsageBit::kAllImage | TextureUsageBit::ALL_TRANSFER | TextureUsageBit::kAllSampled;
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
	TextureViewPtr viewA = gr->newTextureView(TextureViewInitInfo(texA, TextureSurfaceInfo()));
	TextureViewPtr viewB = gr->newTextureView(TextureViewInitInfo(texB, TextureSurfaceInfo()));
	TextureViewPtr viewC = gr->newTextureView(TextureViewInitInfo(texC, TextureSurfaceInfo()));

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

	setTextureSurfaceBarrier(cmdb, texA, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
								   TextureSurfaceInfo());
	setTextureSurfaceBarrier(cmdb, texB, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
								   TextureSurfaceInfo());
	setTextureSurfaceBarrier(cmdb, texC, TextureUsageBit::kNone, TextureUsageBit::kTransferDestination,
								   TextureSurfaceInfo());

	TransferGpuAllocatorHandle handle0, handle1, handle2;
	const UVec4 mip0 = UVec4(1, 2, 3, 4);
	UPLOAD_TEX_SURFACE(cmdb, texA, TextureSurfaceInfo(0, 0, 0, 0), &mip0[0], sizeof(mip0), handle0);
	const UVec4 mip1 = UVec4(10, 20, 30, 40);
	UPLOAD_TEX_SURFACE(cmdb, texB, TextureSurfaceInfo(0, 0, 0, 0), &mip1[0], sizeof(mip1), handle1);
	const Vec4 mip2 = Vec4(2.2f, 3.3f, 4.4f, 5.5f);
	UPLOAD_TEX_SURFACE(cmdb, texC, TextureSurfaceInfo(0, 0, 0, 0), &mip2[0], sizeof(mip2), handle2);

	setTextureSurfaceBarrier(cmdb, texA, TextureUsageBit::kTransferDestination, TextureUsageBit::kImageComputeRead,
								   TextureSurfaceInfo());
	setTextureSurfaceBarrier(cmdb, texB, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledCompute,
								   TextureSurfaceInfo());
	setTextureSurfaceBarrier(cmdb, texC, TextureUsageBit::kTransferDestination, TextureUsageBit::kSampledCompute,
								   TextureSurfaceInfo());

	cmdb->bindStorageBuffer(1, 0, resBuff, 0, kMaxPtrSize);
	cmdb->bindSampler(1, 1, sampler);
	cmdb->bindShaderProgram(prog);

	const U32 idx0 = viewA->getOrCreateBindlessImageIndex();
	const U32 idx1 = viewB->getOrCreateBindlessTextureIndex();
	const U32 idx2 = viewC->getOrCreateBindlessTextureIndex();
	UVec4 pc(idx0, idx1, idx2, 0);
	cmdb->setPushConstants(&pc, sizeof(pc));

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

ANKI_TEST(Gr, BufferAddress)
{
	COMMON_BEGIN()

	// Create program
	static const char* PROG_SRC = R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

ANKI_DEFINE_LOAD_STORE(Vec4, 4)

layout(push_constant) uniform u_
{
	U64 u_bufferAddressRead;
	U64 u_bufferAddressWrite;
};

void main()
{
	Vec4 a;
	load(u_bufferAddressRead, a);
	Vec4 b;
	load(u_bufferAddressRead + 16ul, b);

	store(u_bufferAddressWrite, a + b);
})";

	ShaderPtr shader = createShader(PROG_SRC, ShaderType::kCompute, *g_gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_computeShader = shader;
	ShaderProgramPtr prog = g_gr->newShaderProgram(sprogInit);

	// Create buffers
	BufferInitInfo info;
	info.m_size = sizeof(Vec4) * 2;
	info.m_usage = BufferUsageBit::kAllCompute;
	info.m_mapAccess = BufferMapAccessBit::kWrite;
	BufferPtr ptrBuff = g_gr->newBuffer(info);

	Vec4* mapped = static_cast<Vec4*>(ptrBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
	const Vec4 VEC(123.456f, -1.1f, 100.0f, -666.0f);
	*mapped = VEC;
	++mapped;
	*mapped = VEC * 10.0f;
	ptrBuff->unmap();

	BufferPtr resBuff =
		g_gr->newBuffer(BufferInitInfo(sizeof(Vec4), BufferUsageBit::kAllCompute, BufferMapAccessBit::kRead));

	// Run
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

	cmdb->bindShaderProgram(prog);

	struct Address
	{
		PtrSize m_addressRead;
		PtrSize m_addressWrite;
	} address;
	address.m_addressRead = ptrBuff->getGpuAddress();
	address.m_addressWrite = resBuff->getGpuAddress();
	cmdb->setPushConstants(&address, sizeof(address));

	cmdb->dispatchCompute(1, 1, 1);

	cmdb->flush();
	g_gr->finish();

	// Check
	mapped = static_cast<Vec4*>(resBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kRead));
	ANKI_TEST_EXPECT_EQ(*mapped, VEC + VEC * 10.0f);
	resBuff->unmap();

	COMMON_END();
}

ANKI_TEST(Gr, RayQuery)
{
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
		init.m_bottomLevel.m_indexBuffer = idxBuffer;
		init.m_bottomLevel.m_indexCount = 3;
		init.m_bottomLevel.m_indexType = IndexType::kU16;
		init.m_bottomLevel.m_positionBuffer = vertBuffer;
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
		Array<AccelerationStructureInstance, 1> instances = {{{blas, Mat3x4::getIdentity()}}};
		init.m_topLevel.m_instances = instances;

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

		HeapMemoryPool pool(allocAligned, nullptr);
		StringRaii fragSrc(&pool);
		if(useRayTracing)
		{
			fragSrc.append("#define USE_RAY_TRACING 1\n");
		}
		else
		{
			fragSrc.append("#define USE_RAY_TRACING 0\n");
		}
		fragSrc.append(src);
		prog = createProgram(VERT_QUAD_STRIP_SRC, fragSrc, *g_gr);
	}

	// Build AS
	if(useRayTracing)
	{
		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		setAccelerationStructureBarrier(cmdb, blas, AccelerationStructureUsageBit::kNone,
										AccelerationStructureUsageBit::kBuild);
		cmdb->buildAccelerationStructure(blas);
		setAccelerationStructureBarrier(cmdb, blas, AccelerationStructureUsageBit::kBuild,
										AccelerationStructureUsageBit::kAttach);

		setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kNone,
										AccelerationStructureUsageBit::kBuild);
		cmdb->buildAccelerationStructure(tlas);
		setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kBuild,
										AccelerationStructureUsageBit::kFragmentRead);

		cmdb->flush();
	}

	// Draw
	constexpr U32 ITERATIONS = 200;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		const Vec4 cameraPos = {0.0f, 0.0f, 3.0f, 0.0f};
		const Mat4 viewMat = Mat4{Transform{cameraPos, Mat3x4::getIdentity(), 1.0f}}.getInverse();
		const Mat4 projMat = Mat4::calculatePerspectiveProjectionMatrix(toRad(90.0f), toRad(90.0f), 0.01f, 1000.0f);

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);

		cmdb->bindShaderProgram(prog);
		struct PC
		{
			Mat4 m_vp;
			Vec4 m_cameraPos;
		} pc;
		pc.m_vp = projMat * viewMat;
		pc.m_cameraPos = cameraPos;
		cmdb->setPushConstants(&pc, sizeof(pc));

		if(useRayTracing)
		{
			cmdb->bindAccelerationStructure(0, 0, tlas);
		}

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		FramebufferPtr fb = createColorFb(*g_gr, presentTex);

		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kNone, TextureUsageBit::kFramebufferWrite,
						  TextureSubresourceInfo{});

		cmdb->beginRenderPass(fb, {TextureUsageBit::kFramebufferWrite}, {});
		cmdb->drawArrays(PrimitiveTopology::kTriangleStrip, 4);
		cmdb->endRenderPass();

		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kFramebufferWrite, TextureUsageBit::kPresent,
						  TextureSubresourceInfo{});

		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 30.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END();
}

static void createCubeBuffers(GrManager& gr, Vec3 min, Vec3 max, BufferPtr& indexBuffer, BufferPtr& vertBuffer,
							  Bool turnInsideOut = false)
{
	BufferInitInfo inf;
	inf.m_mapAccess = BufferMapAccessBit::kWrite;
	inf.m_usage = BufferUsageBit::kIndex | BufferUsageBit::kVertex | BufferUsageBit::kStorageTraceRaysRead;
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
	COMMON_BEGIN();

	const Bool useRayTracing = g_gr->getDeviceCapabilities().m_rayTracingEnabled;
	if(!useRayTracing)
	{
		ANKI_TEST_LOGW("Ray tracing not supported");
		break;
	}

	using Mat3x4Scalar = Array2d<F32, 3, 4>;
#define MAGIC_MACRO(x) x
#include <Tests/Gr/RtTypes.h>
#undef MAGIC_MACRO

	HeapMemoryPool pool(allocAligned, nullptr);

	// Create the offscreen RTs
	Array<TexturePtr, 2> offscreenRts;
	{
		TextureInitInfo inf("T_offscreen#1");
		inf.m_width = WIDTH;
		inf.m_height = HEIGHT;
		inf.m_format = Format::kR8G8B8A8_Unorm;
		inf.m_usage = TextureUsageBit::kImageTraceRaysRead | TextureUsageBit::kImageTraceRaysWrite
					  | TextureUsageBit::kImageComputeRead;

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
		sprogInit.m_computeShader = shader;
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
	geometries[GeomWhat::SMALL_BOX].m_worldTransform = Mat3x4(
		Vec3((geometries[GeomWhat::SMALL_BOX].m_aabb.getMin() + geometries[GeomWhat::SMALL_BOX].m_aabb.getMax()).xyz()
			 / 2.0f),
		geometries[GeomWhat::SMALL_BOX].m_worldRotation);
	geometries[GeomWhat::SMALL_BOX].m_diffuseColor = Vec3(0.75f);

	geometries[GeomWhat::BIG_BOX].m_aabb = Aabb(Vec3(265.0f, 0.0f, 295.0f), Vec3(430.0f, 330.0f, 460.0f));
	geometries[GeomWhat::BIG_BOX].m_worldRotation = Mat3(Axisang(toRad(15.0f), Vec3(0.0f, 1.0f, 0.0f)));
	geometries[GeomWhat::BIG_BOX].m_worldTransform = Mat3x4(
		Vec3((geometries[GeomWhat::BIG_BOX].m_aabb.getMin() + geometries[GeomWhat::BIG_BOX].m_aabb.getMax()).xyz()
			 / 2.0f),
		geometries[GeomWhat::BIG_BOX].m_worldRotation);
	geometries[GeomWhat::BIG_BOX].m_diffuseColor = Vec3(0.75f);

	geometries[GeomWhat::ROOM].m_aabb = Aabb(Vec3(0.0f), Vec3(555.0f));
	geometries[GeomWhat::ROOM].m_worldRotation = Mat3::getIdentity();
	geometries[GeomWhat::ROOM].m_worldTransform = Mat3x4(
		Vec3((geometries[GeomWhat::ROOM].m_aabb.getMin() + geometries[GeomWhat::ROOM].m_aabb.getMax()).xyz() / 2.0f),
		geometries[GeomWhat::ROOM].m_worldRotation);
	geometries[GeomWhat::ROOM].m_insideOut = true;
	geometries[GeomWhat::ROOM].m_indexCount = 30;

	geometries[GeomWhat::LIGHT].m_aabb =
		Aabb(Vec3(213.0f + 1.0f, 554.0f, 227.0f + 1.0f), Vec3(343.0f - 1.0f, 554.0f + 0.001f, 332.0f - 1.0f));
	geometries[GeomWhat::LIGHT].m_worldRotation = Mat3::getIdentity();
	geometries[GeomWhat::LIGHT].m_worldTransform = Mat3x4(
		Vec3((geometries[GeomWhat::LIGHT].m_aabb.getMin() + geometries[GeomWhat::LIGHT].m_aabb.getMax()).xyz() / 2.0f),
		geometries[GeomWhat::LIGHT].m_worldRotation);
	geometries[GeomWhat::LIGHT].m_asMask = 0b01;
	geometries[GeomWhat::LIGHT].m_emissiveColor = Vec3(15.0f);

	// Create Buffers
	for(Geom& g : geometries)
	{
		createCubeBuffers(*g_gr, -(g.m_aabb.getMax().xyz() - g.m_aabb.getMin().xyz()) / 2.0f,
						  (g.m_aabb.getMax().xyz() - g.m_aabb.getMin().xyz()) / 2.0f, g.m_indexBuffer, g.m_vertexBuffer,
						  g.m_insideOut);
	}

	// Create AS
	AccelerationStructurePtr tlas;
	{
		for(Geom& g : geometries)
		{
			AccelerationStructureInitInfo inf;
			inf.m_type = AccelerationStructureType::kBottomLevel;
			inf.m_bottomLevel.m_indexBuffer = g.m_indexBuffer;
			inf.m_bottomLevel.m_indexType = IndexType::kU16;
			inf.m_bottomLevel.m_indexCount = g.m_indexCount;
			inf.m_bottomLevel.m_positionBuffer = g.m_vertexBuffer;
			inf.m_bottomLevel.m_positionCount = 8;
			inf.m_bottomLevel.m_positionsFormat = Format::kR32G32B32_Sfloat;
			inf.m_bottomLevel.m_positionStride = sizeof(Vec3);

			g.m_blas = g_gr->newAccelerationStructure(inf);
		}

		// TLAS
		Array<AccelerationStructureInstance, U32(GeomWhat::kCount)> instances;
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
		inf.m_topLevel.m_instances = instances;

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
		WeakArray<Model, PtrSize> models =
			modelBuffer->map<Model>(0, U32(GeomWhat::kCount), BufferMapAccessBit::kWrite);
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

#define MAGIC_MACRO ANKI_STRINGIZE
		const CString rtTypesStr =
#include <Tests/Gr/RtTypes.h>
			;
#undef MAGIC_MACRO

		StringRaii commonSrc(&pool);
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

		ShaderPtr lambertianShader =
			createShader(StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), lambertianSrc.cstr()),
						 ShaderType::kClosestHit, *g_gr);
		ShaderPtr lambertianRoomShader =
			createShader(StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), lambertianRoomSrc.cstr()),
						 ShaderType::kClosestHit, *g_gr);
		ShaderPtr emissiveShader = createShader(
			StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), emissiveSrc.cstr()), ShaderType::kClosestHit, *g_gr);

		ShaderPtr shadowAhitShader = createShader(
			StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), shadowAhitSrc.cstr()), ShaderType::kAnyHit, *g_gr);
		ShaderPtr shadowChitShader =
			createShader(StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), shadowChitSrc.cstr()),
						 ShaderType::kClosestHit, *g_gr);
		ShaderPtr missShader = createShader(StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), missSrc.cstr()),
											ShaderType::kMiss, *g_gr);

		ShaderPtr shadowMissShader = createShader(
			StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), shadowMissSrc.cstr()), ShaderType::kMiss, *g_gr);

		ShaderPtr rayGenShader = createShader(StringRaii(&pool).sprintf("%s\n%s", commonSrc.cstr(), rayGenSrc.cstr()),
											  ShaderType::kRayGen, *g_gr);

		Array<RayTracingHitGroup, 4> hitGroups;
		hitGroups[0].m_closestHitShader = lambertianShader;
		hitGroups[1].m_closestHitShader = lambertianRoomShader;
		hitGroups[2].m_closestHitShader = emissiveShader;
		hitGroups[3].m_closestHitShader = shadowChitShader;
		hitGroups[3].m_anyHitShader = shadowAhitShader;

		Array<ShaderPtr, 2> missShaders = {missShader, shadowMissShader};

		// Add the same 2 times to test multiple ray gen shaders
		Array<ShaderPtr, 2> rayGenShaders = {rayGenShader, rayGenShader};

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
		inf.m_usage = BufferUsageBit::kSBT;
		inf.m_size = sbtRecordSize * recordCount;

		sbt = g_gr->newBuffer(inf);
		WeakArray<U8, PtrSize> mapped = sbt->map<U8>(0, inf.m_size, BufferMapAccessBit::kWrite);
		memset(&mapped[0], 0, inf.m_size);

		ConstWeakArray<U8> handles = rtProg->getShaderGroupHandles();
		ANKI_TEST_EXPECT_EQ(handles.getSize(),
							g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * (hitgroupCount + 1));

		// Ray gen
		U32 record = 0;
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * rayGenGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// 2xMiss
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * missGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowMissGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Small box
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * lambertianChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Big box
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * lambertianChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Room
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * lambertianRoomChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);

		// Light
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * emissiveChitGroupIdx],
			   g_gr->getDeviceCapabilities().m_shaderGroupHandleSize);
		memcpy(&mapped[sbtRecordSize * record++],
			   &handles[g_gr->getDeviceCapabilities().m_shaderGroupHandleSize * shadowAhitGroupIdx],
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

		const Mat4 viewMat =
			Mat4::lookAt(Vec3(278.0f, 278.0f, -800.0f), Vec3(278.0f, 278.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f))
				.getInverse();
		const Mat4 projMat =
			Mat4::calculatePerspectiveProjectionMatrix(toRad(40.0f) * WIDTH / HEIGHT, toRad(40.0f), 0.01f, 2000.0f);

		CommandBufferInitInfo cinit;
		cinit.m_flags =
			CommandBufferFlag::kGeneralWork | CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = g_gr->newCommandBuffer(cinit);

		if(i == 0)
		{
			for(const Geom& g : geometries)
			{
				setAccelerationStructureBarrier(cmdb, g.m_blas, AccelerationStructureUsageBit::kNone,
												AccelerationStructureUsageBit::kBuild);
			}

			for(const Geom& g : geometries)
			{
				cmdb->buildAccelerationStructure(g.m_blas);
			}

			for(const Geom& g : geometries)
			{
				setAccelerationStructureBarrier(cmdb, g.m_blas, AccelerationStructureUsageBit::kBuild,
												AccelerationStructureUsageBit::kAttach);
			}

			setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kNone,
											AccelerationStructureUsageBit::kBuild);
			cmdb->buildAccelerationStructure(tlas);
			setAccelerationStructureBarrier(cmdb, tlas, AccelerationStructureUsageBit::kBuild,
											AccelerationStructureUsageBit::kTraceRaysRead);
		}

		TexturePtr presentTex = g_gr->acquireNextPresentableTexture();
		TextureViewPtr presentView;
		{

			TextureViewInitInfo inf;
			inf.m_texture = presentTex;
			presentView = g_gr->newTextureView(inf);
		}

		TextureViewPtr offscreenView, offscreenHistoryView;
		{

			TextureViewInitInfo inf;
			inf.m_texture = offscreenRts[i & 1];
			offscreenView = g_gr->newTextureView(inf);

			inf.m_texture = offscreenRts[(i + 1) & 1];
			offscreenHistoryView = g_gr->newTextureView(inf);
		}

		setTextureBarrier(cmdb, offscreenRts[i & 1], TextureUsageBit::kNone, TextureUsageBit::kImageTraceRaysWrite,
						  TextureSubresourceInfo());
		setTextureBarrier(cmdb, offscreenRts[(i + 1) & 1], TextureUsageBit::kImageComputeRead,
						  TextureUsageBit::kImageTraceRaysRead, TextureSubresourceInfo());

		cmdb->bindStorageBuffer(0, 0, modelBuffer, 0, kMaxPtrSize);
		cmdb->bindStorageBuffer(0, 1, lightBuffer, 0, kMaxPtrSize);
		cmdb->bindAccelerationStructure(1, 0, tlas);
		cmdb->bindImage(1, 1, offscreenHistoryView);
		cmdb->bindImage(1, 2, offscreenView);

		cmdb->bindShaderProgram(rtProg);

		PushConstants pc;
		pc.m_vp = projMat * viewMat;
		pc.m_cameraPos = Vec3(278.0f, 278.0f, -800.0f);
		pc.m_lightCount = lightCount;
		pc.m_frame = i;

		cmdb->setPushConstants(&pc, sizeof(pc));

		const U32 sbtRecordSize = g_gr->getDeviceCapabilities().m_sbtRecordAlignment;
		cmdb->traceRays(sbt, 0, sbtRecordSize, U32(GeomWhat::kCount) * 2, 2, WIDTH, HEIGHT, 1);

		// Copy to present
		setTextureBarrier(cmdb, offscreenRts[i & 1], TextureUsageBit::kImageTraceRaysWrite,
						  TextureUsageBit::kImageComputeRead, TextureSubresourceInfo());
		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kNone, TextureUsageBit::kImageComputeWrite,
						  TextureSubresourceInfo());

		cmdb->bindImage(0, 0, offscreenView);
		cmdb->bindImage(0, 1, presentView);

		cmdb->bindShaderProgram(copyToPresentProg);
		const U32 sizeX = (WIDTH + 8 - 1) / 8;
		const U32 sizeY = (HEIGHT + 8 - 1) / 8;
		cmdb->dispatchCompute(sizeX, sizeY, 1);

		setTextureBarrier(cmdb, presentTex, TextureUsageBit::kImageComputeWrite, TextureUsageBit::kPresent,
						  TextureSubresourceInfo());

		cmdb->flush();

		g_gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0f / 60.0f;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END();
}

ANKI_TEST(Gr, AsyncCompute)
{
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
	sprogInit.m_computeShader = shader;
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
	sprogInit.m_computeShader = shader;
	ShaderProgramPtr checkProg = g_gr->newShaderProgram(sprogInit);

	// Create buffers
	BufferInitInfo info;
	info.m_size = sizeof(U32) * ARRAY_SIZE;
	info.m_usage = BufferUsageBit::kAllCompute;
	info.m_mapAccess = BufferMapAccessBit::kWrite | BufferMapAccessBit::kRead;
	BufferPtr atomicsBuffer = g_gr->newBuffer(info);
	U32* values =
		static_cast<U32*>(atomicsBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kRead | BufferMapAccessBit::kWrite));
	memset(values, 0, info.m_size);

	// Pre-create some CPU result buffers
	HeapMemoryPool pool(allocAligned, nullptr);
	DynamicArrayRaii<U32> atomicsBufferCpu(&pool);
	atomicsBufferCpu.create(ARRAY_SIZE);
	DynamicArrayRaii<U32> expectedResultsBufferCpu(&pool);
	expectedResultsBufferCpu.create(ARRAY_SIZE);
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
	incrementCmdb->bindShaderProgram(incrementProg);
	incrementCmdb->bindStorageBuffer(0, 0, atomicsBuffer, 0, kMaxPtrSize);
	incrementCmdb->dispatchCompute(ARRAY_SIZE / 8, 1, 1);

	// Create the 2nd command buffer
	cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr checkCmdb = g_gr->newCommandBuffer(cinit);
	checkCmdb->bindShaderProgram(checkProg);
	checkCmdb->bindStorageBuffer(0, 0, atomicsBuffer, 0, kMaxPtrSize);
	checkCmdb->dispatchCompute(ARRAY_SIZE / 8, 1, 1);

	// Create the 3rd command buffer
	cinit.m_flags = CommandBufferFlag::kComputeWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr incrementCmdb2 = g_gr->newCommandBuffer(cinit);
	incrementCmdb2->bindShaderProgram(incrementProg);
	incrementCmdb2->bindStorageBuffer(0, 0, atomicsBuffer, 0, kMaxPtrSize);
	incrementCmdb2->dispatchCompute(ARRAY_SIZE / 8, 1, 1);

	// Submit
#if 1
	FencePtr fence;
	incrementCmdb->flush({}, &fence);
	checkCmdb->flush(Array<FencePtr, 1>{fence}, &fence);
	incrementCmdb2->flush(Array<FencePtr, 1>{fence}, &fence);
	fence->clientWait(kMaxSecond);
#else
	incrementCmdb->flush();
	gr->finish();
	checkCmdb->flush();
	gr->finish();
	incrementCmdb2->flush();
	gr->finish();
#endif

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
}
