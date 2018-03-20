// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/Gr.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/Config.h>
#include <anki/util/HighRezTimer.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/resource/TransferGpuAllocator.h>
#include <ctime>

namespace anki
{

const U WIDTH = 1024;
const U HEIGHT = 768;

static const char* VERT_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, 1.0), vec2(0.0, -1.0), vec2(1.0, 1.0));

	gl_Position = vec4(POSITIONS[gl_VertexID % 3], 0.0, 1.0);
})";

static const char* VERT_QUAD_STRIP_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[4] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

	gl_Position = vec4(POSITIONS[gl_VertexID % 4], 0.0, 1.0);
})";

static const char* VERT_UBO_SRC = R"(
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	vec4 u_color[3];
};

layout(ANKI_UBO_BINDING(0, 1)) uniform u1_
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

layout(ANKI_UBO_BINDING(0, 0), std140, row_major) uniform u0_
{
	mat4 u_mvp;
};

void main()
{
	gl_Position = u_mvp * vec4(in_pos, 1.0);
})";

static const char* FRAG_SRC = R"(layout (location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(0.5);
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

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex0;

void main()
{
	out_color = texture(u_tex0, in_uv);
})";

static const char* FRAG_2TEX_SRC = R"(layout (location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex0;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_tex1;

void main()
{
	if(anki_fragCoord.x < 1024 / 2)
	{
		if(anki_fragCoord.y < 768 / 2)
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
		if(anki_fragCoord.y < 768 / 2)
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

static const char* FRAG_TEX3D_SRC = R"(layout (location = 0) out vec4 out_color;

layout(ANKI_UBO_BINDING(0, 0)) uniform u0_
{
	vec4 u_uv;
};

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler3D u_tex;

void main()
{
	out_color = textureLod(u_tex, u_uv.xyz, u_uv.w);
})";

static const char* FRAG_MRT_SRC = R"(layout (location = 0) out vec4 out_color0;
layout (location = 1) out vec4 out_color1;

layout(ANKI_UBO_BINDING(0, 1), std140) uniform u1_
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

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex0;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_tex1;

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
layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex0;

void main()
{
	out_color = textureLod(u_tex0, in_uv, 1.0);
})";

static const char* COMP_WRITE_IMAGE_SRC = R"(
layout(ANKI_IMAGE_BINDING(0, 0), rgba8) writeonly uniform image2D u_img;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
	
layout(ANKI_SS_BINDING(1, 0)) buffer ss1_
{
	vec4 u_color;
};
	
void main()
{
	imageStore(u_img, ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y), u_color);
})";

static NativeWindow* win = nullptr;
static GrManager* gr = nullptr;
static StagingGpuMemoryManager* stagingMem = nullptr;

#define COMMON_BEGIN() \
	stagingMem = new StagingGpuMemoryManager(); \
	Config cfg; \
	cfg.set("width", WIDTH); \
	cfg.set("height", HEIGHT); \
	cfg.set("window.debugContext", true); \
	cfg.set("window.vsync", false); \
	win = createWindow(cfg); \
	gr = createGrManager(cfg, win); \
	ANKI_TEST_EXPECT_NO_ERR(stagingMem->init(gr, Config())); \
	TransferGpuAllocator* transfAlloc = new TransferGpuAllocator(); \
	ANKI_TEST_EXPECT_NO_ERR(transfAlloc->init(128_MB, gr, gr->getAllocator())); \
	{

#define COMMON_END() \
	} \
	gr->finish(); \
	delete transfAlloc; \
	delete stagingMem; \
	GrManager::deleteInstance(gr); \
	delete win; \
	win = nullptr; \
	gr = nullptr; \
	stagingMem = nullptr;

static void* setUniforms(PtrSize size, CommandBufferPtr& cmdb, U set, U binding)
{
	StagingGpuMemoryToken token;
	void* ptr = stagingMem->allocateFrame(size, StagingGpuMemoryType::UNIFORM, token);
	cmdb->bindUniformBuffer(set, binding, token.m_buffer, token.m_offset, token.m_range);
	return ptr;
}

static void* setStorage(PtrSize size, CommandBufferPtr& cmdb, U set, U binding)
{
	StagingGpuMemoryToken token;
	void* ptr = stagingMem->allocateFrame(size, StagingGpuMemoryType::STORAGE, token);
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
		TextureViewPtr view = gr->newTextureView(TextureViewInitInfo(tex_, surf_)); \
		cmdb_->copyBufferToTextureView(handle_.getBuffer(), handle_.getOffset(), handle_.getRange(), view); \
	} while(0)

#define UPLOAD_TEX_VOL(cmdb_, tex_, vol_, ptr_, size_, handle_) \
	do \
	{ \
		ANKI_TEST_EXPECT_NO_ERR(transfAlloc->allocate(size_, handle_)); \
		void* f = handle_.getMappedMemory(); \
		memcpy(f, ptr_, size_); \
		TextureViewPtr view = gr->newTextureView(TextureViewInitInfo(tex_, vol_)); \
		cmdb_->copyBufferToTextureView(handle_.getBuffer(), handle_.getOffset(), handle_.getRange(), view); \
	} while(0)

const Format DS_FORMAT = Format::D24_UNORM_S8_UINT;

static ShaderPtr createShader(
	CString src, ShaderType type, GrManager& gr, ConstWeakArray<ShaderSpecializationConstValue> specVals = {})
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ShaderCompiler comp(alloc);

	ShaderCompilerOptions options;
	options.setFromGrManager(gr);
	options.m_shaderType = type;

	DynamicArrayAuto<U8> bin(alloc);
	ANKI_TEST_EXPECT_NO_ERR(comp.compile(src, options, bin));

	ShaderInitInfo initInf(type, WeakArray<U8>(&bin[0], bin.getSize()));
	initInf.m_constValues = specVals;

	return gr.newShader(initInf);
}

static ShaderProgramPtr createProgram(CString vertSrc, CString fragSrc, GrManager& gr)
{
	ShaderPtr vert = createShader(vertSrc, ShaderType::VERTEX, gr);
	ShaderPtr frag = createShader(fragSrc, ShaderType::FRAGMENT, gr);
	return gr.newShaderProgram(ShaderProgramInitInfo(vert, frag));
}

static FramebufferPtr createDefaultFb(GrManager& gr)
{
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 1;
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 1.0, 1.0}};

	return gr.newFramebuffer(fbinit);
}

static void createCube(GrManager& gr, BufferPtr& verts, BufferPtr& indices)
{
	static const Array<F32, 8 * 3> pos = {
		{1, 1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1}};

	static const Array<U16, 6 * 2 * 3> idx = {
		{0, 1, 3, 3, 1, 2, 1, 5, 6, 1, 6, 2, 7, 4, 0, 7, 0, 3, 6, 5, 7, 7, 5, 4, 0, 4, 5, 0, 5, 1, 3, 2, 6, 3, 6, 7}};

	verts = gr.newBuffer(BufferInitInfo(sizeof(pos), BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE));

	void* mapped = verts->map(0, sizeof(pos), BufferMapAccessBit::WRITE);
	memcpy(mapped, &pos[0], sizeof(pos));
	verts->unmap();

	indices = gr.newBuffer(BufferInitInfo(sizeof(idx), BufferUsageBit::INDEX, BufferMapAccessBit::WRITE));
	mapped = indices->map(0, sizeof(idx), BufferMapAccessBit::WRITE);
	memcpy(mapped, &idx[0], sizeof(idx));
	indices->unmap();
}

ANKI_TEST(Gr, GrManager){COMMON_BEGIN() COMMON_END()}

ANKI_TEST(Gr, Shader)
{
	COMMON_BEGIN()

	ShaderPtr shader = createShader(FRAG_MRT_SRC, ShaderType::FRAGMENT, *gr);

	COMMON_END()
}

ANKI_TEST(Gr, ShaderProgram)
{
	COMMON_BEGIN()

	ShaderProgramPtr ppline = createProgram(VERT_SRC, FRAG_SRC, *gr);

	COMMON_END()
}

ANKI_TEST(Gr, ClearScreen)
{
	COMMON_BEGIN()
	ANKI_TEST_LOGI("Expect to see a magenta background");

	FramebufferPtr fb = createDefaultFb(*gr);

	U iterations = 100;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		cmdb->beginRenderPass(fb, {}, {});
		cmdb->endRenderPass();
		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
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

	ANKI_TEST_LOGI("Expect to see a grey triangle");
	ShaderProgramPtr prog = createProgram(VERT_SRC, FRAG_SRC, *gr);
	FramebufferPtr fb = createDefaultFb(*gr);

	const U ITERATIONS = 200;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb, {}, {});
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
		cmdb->endRenderPass();
		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

ANKI_TEST(Gr, ViewportAndScissor)
{
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
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
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
		const F32 TICK = 1.0 / 30.0;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

ANKI_TEST(Gr, ViewportAndScissorOffscreen)
{
	srand(time(nullptr));
	COMMON_BEGIN()

	ANKI_TEST_LOGI("Expect to see a grey quad appearing in the 4 corners. "
				   "Around that quad is a border that changes color. "
				   "The quads appear counter-clockwise");
	ShaderProgramPtr prog = createProgram(VERT_QUAD_STRIP_SRC, FRAG_SRC, *gr);
	ShaderProgramPtr blitProg = createProgram(VERT_QUAD_SRC, FRAG_TEX_SRC, *gr);

	const Format COL_FORMAT = Format::R8G8B8A8_UNORM;
	const U RT_WIDTH = 32;
	const U RT_HEIGHT = 16;
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = COL_FORMAT;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	init.m_height = RT_HEIGHT;
	init.m_width = RT_WIDTH;
	init.m_mipmapCount = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_type = TextureType::_2D;
	TexturePtr rt = gr->newTexture(init);

	TextureViewInitInfo viewInit(rt);
	TextureViewPtr texView = gr->newTextureView(viewInit);

	Array<FramebufferPtr, 4> fb;
	for(FramebufferPtr& f : fb)
	{
		TextureViewInitInfo viewInf(rt);
		TextureViewPtr view = gr->newTextureView(viewInf);

		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentCount = 1;
		fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{randFloat(1.0), randFloat(1.0), randFloat(1.0), 1.0}};
		fbinit.m_colorAttachments[0].m_textureView = view;

		f = gr->newFramebuffer(fbinit);
	}

	FramebufferPtr defaultFb = createDefaultFb(*gr);

	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::NEAREST;
	samplerInit.m_mipmapFilter = SamplingFilter::BASE;
	SamplerPtr sampler = gr->newSampler(samplerInit);

	static const Array2d<U, 4, 4> VIEWPORTS = {{{{0, 0, RT_WIDTH / 2, RT_HEIGHT / 2}},
		{{RT_WIDTH / 2, 0, RT_WIDTH / 2, RT_HEIGHT / 2}},
		{{RT_WIDTH / 2, RT_HEIGHT / 2, RT_WIDTH / 2, RT_HEIGHT / 2}},
		{{0, RT_HEIGHT / 2, RT_WIDTH / 2, RT_HEIGHT / 2}}}};

	const U ITERATIONS = 400;
	const U SCISSOR_MARGIN = 2;
	const U RENDER_AREA_MARGIN = 1;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		if(i == 0)
		{
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
			CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

			cmdb->setViewport(0, 0, RT_WIDTH, RT_HEIGHT);
			cmdb->setTextureSurfaceBarrier(rt,
				TextureUsageBit::NONE,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureSurfaceInfo(0, 0, 0, 0));
			cmdb->beginRenderPass(fb[0], {{TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE}}, {});
			cmdb->endRenderPass();
			cmdb->setTextureSurfaceBarrier(rt,
				TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
				TextureUsageBit::SAMPLED_FRAGMENT,
				TextureSurfaceInfo(0, 0, 0, 0));
			cmdb->flush();
		}

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		// Draw offscreen
		cmdb->setTextureSurfaceBarrier(rt,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
		auto vp = VIEWPORTS[(i / 30) % 4];
		cmdb->setViewport(vp[0], vp[1], vp[2], vp[3]);
		cmdb->setScissor(
			vp[0] + SCISSOR_MARGIN, vp[1] + SCISSOR_MARGIN, vp[2] - SCISSOR_MARGIN * 2, vp[3] - SCISSOR_MARGIN * 2);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb[i % 4],
			{{TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE}},
			{},
			vp[0] + RENDER_AREA_MARGIN,
			vp[1] + RENDER_AREA_MARGIN,
			vp[2] - RENDER_AREA_MARGIN * 2,
			vp[3] - RENDER_AREA_MARGIN * 2);
		cmdb->drawArrays(PrimitiveTopology::TRIANGLE_STRIP, 4);
		cmdb->endRenderPass();

		// Draw onscreen
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->setScissor(0, 0, WIDTH, HEIGHT);
		cmdb->bindShaderProgram(blitProg);
		cmdb->setTextureSurfaceBarrier(rt,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->bindTextureAndSampler(0, 0, texView, sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->beginRenderPass(defaultFb, {}, {});
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 6);
		cmdb->endRenderPass();

		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
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

	BufferPtr a = gr->newBuffer(BufferInitInfo(512, BufferUsageBit::UNIFORM_ALL, BufferMapAccessBit::NONE));

	BufferPtr b = gr->newBuffer(
		BufferInitInfo(64, BufferUsageBit::STORAGE_ALL, BufferMapAccessBit::WRITE | BufferMapAccessBit::READ));

	void* ptr = b->map(0, 64, BufferMapAccessBit::WRITE);
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
	U8 ptr2[64];
	memset(ptr, 0xCC, 64);
	memset(ptr2, 0xCC, 64);
	b->unmap();

	ptr = b->map(0, 64, BufferMapAccessBit::READ);
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
		gr->newBuffer(BufferInitInfo(sizeof(Vec4) * 3, BufferUsageBit::UNIFORM_ALL, BufferMapAccessBit::WRITE));

	Vec4* ptr = static_cast<Vec4*>(b->map(0, sizeof(Vec4) * 3, BufferMapAccessBit::WRITE));
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
	ptr[0] = Vec4(1.0, 0.0, 0.0, 0.0);
	ptr[1] = Vec4(0.0, 1.0, 0.0, 0.0);
	ptr[2] = Vec4(0.0, 0.0, 1.0, 0.0);
	b->unmap();

	// Progm
	ShaderProgramPtr prog = createProgram(VERT_UBO_SRC, FRAG_UBO_SRC, *gr);

	// FB
	FramebufferPtr fb = createDefaultFb(*gr);

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();
		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb, {}, {});

		cmdb->bindUniformBuffer(0, 0, b, 0, MAX_PTR_SIZE);

		// Uploaded buffer
		Vec4* rotMat = SET_UNIFORMS(Vec4*, sizeof(Vec4), cmdb, 0, 1);
		F32 angle = toRad(360.0f / ITERATION_COUNT * iterations);
		(*rotMat)[0] = cos(angle);
		(*rotMat)[1] = -sin(angle);
		(*rotMat)[2] = sin(angle);
		(*rotMat)[3] = cos(angle);

		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
		cmdb->endRenderPass();
		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
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

	BufferPtr b = gr->newBuffer(BufferInitInfo(sizeof(Vert) * 3, BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE));

	Vert* ptr = static_cast<Vert*>(b->map(0, sizeof(Vert) * 3, BufferMapAccessBit::WRITE));
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

	ptr[0].m_pos = Vec3(-1.0, 1.0, 0.0);
	ptr[1].m_pos = Vec3(0.0, -1.0, 0.0);
	ptr[2].m_pos = Vec3(1.0, 1.0, 0.0);

	ptr[0].m_color = {{255, 0, 0}};
	ptr[1].m_color = {{0, 255, 0}};
	ptr[2].m_color = {{0, 0, 255}};
	b->unmap();

	BufferPtr c = gr->newBuffer(BufferInitInfo(sizeof(Vec3) * 3, BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE));

	Vec3* otherColor = static_cast<Vec3*>(c->map(0, sizeof(Vec3) * 3, BufferMapAccessBit::WRITE));

	otherColor[0] = Vec3(0.0, 1.0, 1.0);
	otherColor[1] = Vec3(1.0, 0.0, 1.0);
	otherColor[2] = Vec3(1.0, 1.0, 0.0);
	c->unmap();

	// Prog
	ShaderProgramPtr prog = createProgram(VERT_INP_SRC, FRAG_INP_SRC, *gr);

	// FB
	FramebufferPtr fb = createDefaultFb(*gr);

	U iterations = 100;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		cmdb->bindVertexBuffer(0, b, 0, sizeof(Vert));
		cmdb->bindVertexBuffer(1, c, 0, sizeof(Vec3));
		cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
		cmdb->setVertexAttribute(1, 0, Format::R8G8B8_UNORM, sizeof(Vec3));
		cmdb->setVertexAttribute(2, 1, Format::R32G32B32_SFLOAT, 0);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->setPolygonOffset(0.0, 0.0);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb, {}, {});
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
		cmdb->endRenderPass();
		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
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

	SamplerPtr b = gr->newSampler(init);

	COMMON_END()
}

ANKI_TEST(Gr, Texture)
{
	COMMON_BEGIN()

	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::R8G8B8_UNORM;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT;
	init.m_height = 4;
	init.m_width = 4;
	init.m_mipmapCount = 2;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_type = TextureType::_2D;

	TexturePtr b = gr->newTexture(init);

	TextureViewInitInfo view(b);
	TextureViewPtr v = gr->newTextureView(view);

	COMMON_END()
}

ANKI_TEST(Gr, DrawWithTexture)
{
	COMMON_BEGIN()

	//
	// Create sampler
	//
	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::NEAREST;
	samplerInit.m_mipmapFilter = SamplingFilter::LINEAR;
	samplerInit.m_repeat = false;
	SamplerPtr sampler = gr->newSampler(samplerInit);

	//
	// Create texture A
	//
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::R8G8B8_UNORM;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::TRANSFER_DESTINATION;
	init.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapCount = 2;
	init.m_samples = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::_2D;

	TexturePtr a = gr->newTexture(init);

	TextureViewPtr aView = gr->newTextureView(TextureViewInitInfo(a));

	//
	// Create texture B
	//
	init.m_width = 4;
	init.m_height = 4;
	init.m_mipmapCount = 3;
	init.m_usage =
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::TRANSFER_DESTINATION | TextureUsageBit::GENERATE_MIPMAPS;
	init.m_initialUsage = TextureUsageBit::NONE;

	TexturePtr b = gr->newTexture(init);

	TextureViewPtr bView = gr->newTextureView(TextureViewInitInfo(b));

	//
	// Upload all textures
	//
	Array<U8, 2 * 2 * 3> mip0 = {{255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255}};

	Array<U8, 3> mip1 = {{128, 128, 128}};

	Array<U8, 4 * 4 * 3> bmip0 = {{255,
		0,
		0,
		0,
		255,
		0,
		0,
		0,
		255,
		255,
		255,
		0,
		255,
		0,
		255,
		0,
		255,
		255,
		255,
		255,
		255,
		128,
		0,
		0,
		0,
		128,
		0,
		0,
		0,
		128,
		128,
		128,
		0,
		128,
		0,
		128,
		0,
		128,
		128,
		128,
		128,
		128,
		255,
		128,
		0,
		0,
		128,
		255}};

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cmdbinit);

	// Set barriers
	cmdb->setTextureSurfaceBarrier(
		a, TextureUsageBit::SAMPLED_FRAGMENT, TextureUsageBit::TRANSFER_DESTINATION, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(
		a, TextureUsageBit::SAMPLED_FRAGMENT, TextureUsageBit::TRANSFER_DESTINATION, TextureSurfaceInfo(1, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(
		b, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, TextureSurfaceInfo(0, 0, 0, 0));

	TransferGpuAllocatorHandle handle0, handle1, handle2;
	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceInfo(0, 0, 0, 0), &mip0[0], sizeof(mip0), handle0);

	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceInfo(1, 0, 0, 0), &mip1[0], sizeof(mip1), handle1);

	UPLOAD_TEX_SURFACE(cmdb, b, TextureSurfaceInfo(0, 0, 0, 0), &bmip0[0], sizeof(bmip0), handle2);

	// Gen mips
	cmdb->setTextureSurfaceBarrier(
		b, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::GENERATE_MIPMAPS, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->generateMipmaps2d(gr->newTextureView(TextureViewInitInfo(b)));

	// Set barriers
	cmdb->setTextureSurfaceBarrier(
		a, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(
		a, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, TextureSurfaceInfo(1, 0, 0, 0));

	for(U i = 0; i < 3; ++i)
	{
		cmdb->setTextureSurfaceBarrier(
			b, TextureUsageBit::GENERATE_MIPMAPS, TextureUsageBit::SAMPLED_FRAGMENT, TextureSurfaceInfo(i, 0, 0, 0));
	}

	FencePtr fence;
	cmdb->flush(&fence);
	transfAlloc->release(handle0, fence);
	transfAlloc->release(handle1, fence);
	transfAlloc->release(handle2, fence);

	//
	// Create prog
	//
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_2TEX_SRC, *gr);

	//
	// Create FB
	//
	FramebufferPtr fb = createDefaultFb(*gr);

	//
	// Draw
	//
	const U ITERATION_COUNT = 200;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb, {}, {});

		cmdb->bindTextureAndSampler(0, 0, aView, sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->bindTextureAndSampler(0, 1, bView, sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 6);
		cmdb->endRenderPass();
		cmdb->flush();

		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}

	COMMON_END()
}

static void drawOffscreenDrawcalls(GrManager& gr,
	ShaderProgramPtr prog,
	CommandBufferPtr cmdb,
	U viewPortSize,
	BufferPtr indexBuff,
	BufferPtr vertBuff)
{
	static F32 ang = -2.5f;
	ang += toRad(2.5f);

	Mat4 viewMat(Vec4(0.0, 0.0, 5.0, 1.0), Mat3::getIdentity(), 1.0f);
	viewMat.invert();

	Mat4 projMat = Mat4::calculatePerspectiveProjectionMatrix(toRad(60.0), toRad(60.0), 0.1f, 100.0f);

	Mat4 modelMat(Vec4(-0.5, -0.5, 0.0, 1.0), Mat3(Euler(ang, ang / 2.0f, ang / 3.0f)), 1.0f);

	Mat4* mvp = SET_UNIFORMS(Mat4*, sizeof(*mvp), cmdb, 0, 0);
	*mvp = projMat * viewMat * modelMat;

	Vec4* color = SET_UNIFORMS(Vec4*, sizeof(*color) * 2, cmdb, 0, 1);
	*color++ = Vec4(1.0, 0.0, 0.0, 0.0);
	*color = Vec4(0.0, 1.0, 0.0, 0.0);

	cmdb->bindVertexBuffer(0, vertBuff, 0, sizeof(Vec3));
	cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
	cmdb->bindShaderProgram(prog);
	cmdb->bindIndexBuffer(indexBuff, 0, IndexType::U16);
	cmdb->setViewport(0, 0, viewPortSize, viewPortSize);
	cmdb->drawElements(PrimitiveTopology::TRIANGLES, 6 * 2 * 3);

	// 2nd draw
	modelMat = Mat4(Vec4(0.5, 0.5, 0.0, 1.0), Mat3(Euler(ang * 2.0, ang, ang / 3.0f * 2.0)), 1.0f);

	mvp = SET_UNIFORMS(Mat4*, sizeof(*mvp), cmdb, 0, 0);
	*mvp = projMat * viewMat * modelMat;

	color = SET_UNIFORMS(Vec4*, sizeof(*color) * 2, cmdb, 0, 1);
	*color++ = Vec4(0.0, 0.0, 1.0, 0.0);
	*color = Vec4(0.0, 1.0, 1.0, 0.0);

	cmdb->drawElements(PrimitiveTopology::TRIANGLES, 6 * 2 * 3);
}

static void drawOffscreen(GrManager& gr, Bool useSecondLevel)
{
	//
	// Create textures
	//
	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::LINEAR;
	samplerInit.m_mipmapFilter = SamplingFilter::LINEAR;
	SamplerPtr sampler = gr.newSampler(samplerInit);

	const Format COL_FORMAT = Format::R8G8B8A8_UNORM;
	const U TEX_SIZE = 256;

	TextureInitInfo init;
	init.m_format = COL_FORMAT;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	init.m_height = TEX_SIZE;
	init.m_width = TEX_SIZE;
	init.m_type = TextureType::_2D;

	TexturePtr col0 = gr.newTexture(init);
	TexturePtr col1 = gr.newTexture(init);

	TextureViewPtr col0View = gr.newTextureView(TextureViewInitInfo(col0));
	TextureViewPtr col1View = gr.newTextureView(TextureViewInitInfo(col1));

	init.m_format = DS_FORMAT;
	TexturePtr dp = gr.newTexture(init);

	//
	// Create FB
	//
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 2;
	fbinit.m_colorAttachments[0].m_textureView = gr.newTextureView(TextureViewInitInfo(col0));
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{0.1, 0.0, 0.0, 0.0}};
	fbinit.m_colorAttachments[1].m_textureView = gr.newTextureView(TextureViewInitInfo(col1));
	fbinit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0, 0.1, 0.0, 0.0}};
	TextureViewInitInfo viewInit(dp);
	viewInit.m_depthStencilAspect = DepthStencilAspectBit::DEPTH;
	fbinit.m_depthStencilAttachment.m_textureView = gr.newTextureView(viewInit);
	fbinit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	FramebufferPtr fb = gr.newFramebuffer(fbinit);

	//
	// Create default FB
	//
	FramebufferPtr dfb = createDefaultFb(gr);

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
		gr.beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr.newCommandBuffer(cinit);

		cmdb->setPolygonOffset(0.0, 0.0);

		cmdb->setTextureSurfaceBarrier(
			col0, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(
			col1, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(dp,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->beginRenderPass(fb,
			{{TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE}},
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

		if(!useSecondLevel)
		{
			drawOffscreenDrawcalls(gr, prog, cmdb, TEX_SIZE, indices, verts);
		}
		else
		{
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
			cinit.m_framebuffer = fb;
			CommandBufferPtr cmdb2 = gr.newCommandBuffer(cinit);

			drawOffscreenDrawcalls(gr, prog, cmdb2, TEX_SIZE, indices, verts);

			cmdb2->flush();

			cmdb->pushSecondLevelCommandBuffer(cmdb2);
		}

		cmdb->endRenderPass();

		cmdb->setTextureSurfaceBarrier(col0,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(col1,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(dp,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));

		// Draw quad
		cmdb->beginRenderPass(dfb, {}, {});
		cmdb->bindShaderProgram(resolveProg);
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindTextureAndSampler(0, 0, col0View, sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->bindTextureAndSampler(0, 1, col1View, sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 6);
		cmdb->endRenderPass();

		cmdb->flush();

		// End
		gr.swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
		if(timer.getElapsedTime() < TICK)
		{
			HighRezTimer::sleep(TICK - timer.getElapsedTime());
		}
	}
}

ANKI_TEST(Gr, DrawOffscreen)
{
	COMMON_BEGIN()

	drawOffscreen(*gr, false);

	COMMON_END()
}

ANKI_TEST(Gr, DrawWithSecondLevel)
{
	COMMON_BEGIN()

	drawOffscreen(*gr, true);

	COMMON_END()
}

ANKI_TEST(Gr, ImageLoadStore)
{
	COMMON_BEGIN()

	SamplerInitInfo samplerInit;
	samplerInit.m_minMagFilter = SamplingFilter::NEAREST;
	samplerInit.m_mipmapFilter = SamplingFilter::BASE;
	SamplerPtr sampler = gr->newSampler(samplerInit);

	TextureInitInfo init;
	init.m_width = init.m_height = 4;
	init.m_mipmapCount = 2;
	init.m_usage = TextureUsageBit::CLEAR | TextureUsageBit::SAMPLED_ALL | TextureUsageBit::IMAGE_COMPUTE_WRITE;
	init.m_type = TextureType::_2D;
	init.m_format = Format::R8G8B8A8_UNORM;

	TexturePtr tex = gr->newTexture(init);

	TextureViewInitInfo viewInit(tex);
	viewInit.m_firstMipmap = 1;
	viewInit.m_mipmapCount = 1;
	TextureViewPtr view = gr->newTextureView(viewInit);

	// Prog
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_SIMPLE_TEX_SRC, *gr);

	// Create shader & compute prog
	ShaderPtr shader = createShader(COMP_WRITE_IMAGE_SRC, ShaderType::COMPUTE, *gr);
	ShaderProgramInitInfo sprogInit;
	sprogInit.m_shaders[ShaderType::COMPUTE] = shader;
	ShaderProgramPtr compProg = gr->newShaderProgram(sprogInit);

	// FB
	FramebufferPtr dfb = createDefaultFb(*gr);

	// Write texture data
	CommandBufferInitInfo cmdbinit;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cmdbinit);

	cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::NONE, TextureUsageBit::CLEAR, TextureSurfaceInfo(0, 0, 0, 0));

	ClearValue clear;
	clear.m_colorf = {{0.0, 1.0, 0.0, 1.0}};
	TextureViewInitInfo viewInit2(tex, TextureSurfaceInfo(0, 0, 0, 0));
	cmdb->clearTextureView(gr->newTextureView(viewInit2), clear);

	cmdb->setTextureSurfaceBarrier(
		tex, TextureUsageBit::CLEAR, TextureUsageBit::SAMPLED_FRAGMENT, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::NONE, TextureUsageBit::CLEAR, TextureSurfaceInfo(1, 0, 0, 0));

	clear.m_colorf = {{0.0, 0.0, 1.0, 1.0}};
	TextureViewInitInfo viewInit3(tex, TextureSurfaceInfo(1, 0, 0, 0));
	cmdb->clearTextureView(gr->newTextureView(viewInit3), clear);

	cmdb->setTextureSurfaceBarrier(
		tex, TextureUsageBit::CLEAR, TextureUsageBit::IMAGE_COMPUTE_WRITE, TextureSurfaceInfo(1, 0, 0, 0));

	cmdb->flush();

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();
		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags =
			CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::COMPUTE_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		// Write image
		Vec4* col = SET_STORAGE(Vec4*, sizeof(*col), cmdb, 1, 0);
		*col = Vec4(iterations / F32(ITERATION_COUNT));

		cmdb->setTextureSurfaceBarrier(
			tex, TextureUsageBit::NONE, TextureUsageBit::IMAGE_COMPUTE_WRITE, TextureSurfaceInfo(1, 0, 0, 0));
		cmdb->bindShaderProgram(compProg);
		cmdb->bindImage(0, 0, view);
		cmdb->dispatchCompute(WIDTH / 2, HEIGHT / 2, 1);
		cmdb->setTextureSurfaceBarrier(tex,
			TextureUsageBit::IMAGE_COMPUTE_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(1, 0, 0, 0));

		// Present image
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);

		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(dfb, {}, {});
		cmdb->bindTextureAndSampler(
			0, 0, gr->newTextureView(TextureViewInitInfo(tex)), sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 6);
		cmdb->endRenderPass();

		cmdb->flush();

		// End
		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 30.0;
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
	samplerInit.m_minMagFilter = SamplingFilter::NEAREST;
	samplerInit.m_mipmapFilter = SamplingFilter::BASE;
	samplerInit.m_repeat = false;
	SamplerPtr sampler = gr->newSampler(samplerInit);

	//
	// Create texture A
	//
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = Format::R8G8B8A8_UNORM;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::TRANSFER_DESTINATION;
	init.m_initialUsage = TextureUsageBit::TRANSFER_DESTINATION;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapCount = 2;
	init.m_samples = 1;
	init.m_depth = 2;
	init.m_layerCount = 1;
	init.m_type = TextureType::_3D;

	TexturePtr a = gr->newTexture(init);

	//
	// Upload all textures
	//
	Array<U8, 2 * 2 * 2 * 4> mip0 = {{255,
		0,
		0,
		0,
		0,
		255,
		0,
		0,
		0,
		0,
		255,
		0,
		255,
		255,
		0,
		0,
		255,
		0,
		255,
		0,
		0,
		255,
		255,
		0,
		255,
		255,
		255,
		0,
		0,
		0,
		0,
		0}};

	Array<U8, 4> mip1 = {{128, 128, 128, 0}};

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::TRANSFER_WORK | CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cmdbinit);

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, TextureVolumeInfo(0));

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, TextureVolumeInfo(1));

	TransferGpuAllocatorHandle handle0, handle1;
	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeInfo(0), &mip0[0], sizeof(mip0), handle0);

	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeInfo(1), &mip1[0], sizeof(mip1), handle1);

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, TextureVolumeInfo(0));

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, TextureVolumeInfo(1));

	FencePtr fence;
	cmdb->flush(&fence);
	transfAlloc->release(handle0, fence);
	transfAlloc->release(handle1, fence);

	//
	// Rest
	//
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_TEX3D_SRC, *gr);

	FramebufferPtr dfb = createDefaultFb(*gr);

	static Array<Vec4, 9> TEX_COORDS_LOD = {{Vec4(0, 0, 0, 0),
		Vec4(1, 0, 0, 0),
		Vec4(0, 1, 0, 0),
		Vec4(1, 1, 0, 0),
		Vec4(0, 0, 1, 0),
		Vec4(1, 0, 1, 0),
		Vec4(0, 1, 1, 0),
		Vec4(1, 1, 1, 0),
		Vec4(0, 0, 0, 1)}};

	const U ITERATION_COUNT = 100;
	U iterations = ITERATION_COUNT;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();
		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->beginRenderPass(dfb, {}, {});

		cmdb->bindShaderProgram(prog);

		Vec4* uv = SET_UNIFORMS(Vec4*, sizeof(Vec4), cmdb, 0, 0);

		U idx = (F32(ITERATION_COUNT - iterations - 1) / ITERATION_COUNT) * TEX_COORDS_LOD.getSize();
		*uv = TEX_COORDS_LOD[idx];

		cmdb->bindTextureAndSampler(
			0, 0, gr->newTextureView(TextureViewInitInfo(a)), sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 6);

		cmdb->endRenderPass();

		cmdb->flush();

		// End
		gr->swapBuffers();

		timer.stop();
		const F32 TICK = 1.0 / 15.0;
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
	texInf.m_usage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT;
	texInf.m_format = Format::R8G8B8A8_UNORM;
	texInf.bake();
	return texInf;
}

ANKI_TEST(Gr, RenderGraph)
{
	COMMON_BEGIN()

	StackAllocator<U8> alloc(allocAligned, nullptr, 2_MB);
	RenderGraphDescription descr(alloc);
	RenderGraphPtr rgraph = gr->newRenderGraph();

	const U GI_MIP_COUNT = 4;

	TextureInitInfo texI("dummy");
	texI.m_width = texI.m_height = 16;
	texI.m_usage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT;
	texI.m_format = Format::R8G8B8A8_UNORM;
	TexturePtr dummyTex = gr->newTexture(texI);

	// SM
	RenderTargetHandle smScratchRt = descr.newRenderTarget(newRTDescr("SM scratch"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("SM");
		pass.newConsumer({smScratchRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
		pass.newProducer({smScratchRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE});
	}

	// SM to exponential SM
	RenderTargetHandle smExpRt = descr.importRenderTarget("ESM", dummyTex, TextureUsageBit::SAMPLED_FRAGMENT);
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("ESM");
		pass.newConsumer({smScratchRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({smExpRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({smExpRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// GI gbuff
	RenderTargetHandle giGbuffNormRt = descr.newRenderTarget(newRTDescr("GI GBuff norm"));
	RenderTargetHandle giGbuffDiffRt = descr.newRenderTarget(newRTDescr("GI GBuff diff"));
	RenderTargetHandle giGbuffDepthRt = descr.newRenderTarget(newRTDescr("GI GBuff depth"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("GI gbuff");
		pass.newConsumer({giGbuffNormRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({giGbuffDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({giGbuffDiffRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		pass.newProducer({giGbuffNormRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({giGbuffDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({giGbuffDiffRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// GI light
	RenderTargetHandle giGiLightRt = descr.importRenderTarget("GI light", dummyTex, TextureUsageBit::SAMPLED_FRAGMENT);
	for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, 0));

		GraphicsRenderPassDescription& pass =
			descr.newGraphicsRenderPass(StringAuto(alloc).sprintf("GI lp%u", faceIdx).toCString());
		pass.newConsumer({giGiLightRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});
		pass.newConsumer({giGbuffNormRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({giGbuffDepthRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({giGbuffDiffRt, TextureUsageBit::SAMPLED_FRAGMENT});

		pass.newProducer({giGiLightRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});
	}

	// GI light mips
	{
		for(U faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass =
				descr.newGraphicsRenderPass(StringAuto(alloc).sprintf("GI mip%u", faceIdx).toCString());

			for(U mip = 0; mip < GI_MIP_COUNT; ++mip)
			{
				TextureSurfaceInfo surf(mip, 0, faceIdx, 0);
				pass.newConsumer({giGiLightRt, TextureUsageBit::GENERATE_MIPMAPS, surf});
				pass.newProducer({giGiLightRt, TextureUsageBit::GENERATE_MIPMAPS, surf});
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
		pass.newConsumer({gbuffRt0, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({gbuffRt1, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({gbuffRt2, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({gbuffDepth, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		pass.newProducer({gbuffRt0, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({gbuffRt1, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({gbuffRt2, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({gbuffDepth, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Half depth
	RenderTargetHandle halfDepthRt = descr.newRenderTarget(newRTDescr("Depth/2"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("HalfDepth");
		pass.newConsumer({gbuffDepth, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Quarter depth
	RenderTargetHandle quarterDepthRt = descr.newRenderTarget(newRTDescr("Depth/4"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("QuarterDepth");
		pass.newConsumer({quarterDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({halfDepthRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newProducer({quarterDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// SSAO
	RenderTargetHandle ssaoRt = descr.newRenderTarget(newRTDescr("SSAO"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("SSAO main");
		pass.newConsumer({ssaoRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({quarterDepthRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({gbuffRt2, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newProducer({ssaoRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		RenderTargetHandle ssaoVBlurRt = descr.newRenderTarget(newRTDescr("SSAO tmp"));
		GraphicsRenderPassDescription& pass2 = descr.newGraphicsRenderPass("SSAO vblur");
		pass2.newConsumer({ssaoRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass2.newConsumer({ssaoVBlurRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass2.newProducer({ssaoVBlurRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		GraphicsRenderPassDescription& pass3 = descr.newGraphicsRenderPass("SSAO hblur");
		pass3.newConsumer({ssaoRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass3.newProducer({ssaoRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass3.newConsumer({ssaoVBlurRt, TextureUsageBit::SAMPLED_FRAGMENT});
	}

	// Volumetric
	RenderTargetHandle volRt = descr.newRenderTarget(newRTDescr("Vol"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Vol main");
		pass.newConsumer({volRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({quarterDepthRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newProducer({volRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		RenderTargetHandle volVBlurRt = descr.newRenderTarget(newRTDescr("Vol tmp"));
		GraphicsRenderPassDescription& pass2 = descr.newGraphicsRenderPass("Vol vblur");
		pass2.newConsumer({volRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass2.newConsumer({volVBlurRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass2.newProducer({volVBlurRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

		GraphicsRenderPassDescription& pass3 = descr.newGraphicsRenderPass("Vol hblur");
		pass3.newConsumer({volRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass3.newProducer({volRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass3.newConsumer({volVBlurRt, TextureUsageBit::SAMPLED_FRAGMENT});
	}

	// Forward shading
	RenderTargetHandle fsRt = descr.newRenderTarget(newRTDescr("FS"));
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Forward shading");
		pass.newConsumer({fsRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({fsRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer(
			{halfDepthRt, TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ});
		pass.newConsumer({volRt, TextureUsageBit::SAMPLED_FRAGMENT});
	}

	// Light shading
	RenderTargetHandle lightRt = descr.importRenderTarget("Light", dummyTex, TextureUsageBit::NONE);
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Light shading");

		pass.newConsumer({lightRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({gbuffRt0, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({gbuffRt1, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({gbuffRt2, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({gbuffDepth, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({smExpRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({giGiLightRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({ssaoRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({fsRt, TextureUsageBit::SAMPLED_FRAGMENT});

		pass.newProducer({lightRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// TAA
	RenderTargetHandle taaHistoryRt = descr.importRenderTarget("TAA hist", dummyTex, TextureUsageBit::SAMPLED_FRAGMENT);
	RenderTargetHandle taaRt = descr.importRenderTarget("TAA", dummyTex, TextureUsageBit::NONE);
	{
		GraphicsRenderPassDescription& pass = descr.newGraphicsRenderPass("Temporal AA");

		pass.newConsumer({lightRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({taaRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({taaRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({taaHistoryRt, TextureUsageBit::SAMPLED_FRAGMENT});
	}

	rgraph->compileNewGraph(descr, alloc);
	COMMON_END()
}

/// Test workarounds for some unsupported formats
ANKI_TEST(Gr, VkWorkarounds)
{
	COMMON_BEGIN()

	// Create program
	static const char* COMP_SRC = R"(
layout(local_size_x = 8, local_size_y = 8, local_size_z = 2) in;

layout(ANKI_TEX_BINDING(0, 0)) uniform usampler2D u_tex;
	
layout(ANKI_SS_BINDING(0, 0)) buffer s_
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

	ShaderPtr comp = createShader(COMP_SRC, ShaderType::COMPUTE, *gr);
	ShaderProgramPtr prog = gr->newShaderProgram(ShaderProgramInitInfo(comp));

	// Create the texture
	TextureInitInfo texInit;
	texInit.m_width = texInit.m_height = 8;
	texInit.m_format = Format::R8G8B8_UINT;
	texInit.m_type = TextureType::_2D;
	texInit.m_usage = TextureUsageBit::TRANSFER_DESTINATION | TextureUsageBit::SAMPLED_ALL;
	texInit.m_mipmapCount = 2;
	TexturePtr tex = gr->newTexture(texInit);
	TextureViewPtr texView = gr->newTextureView(TextureViewInitInfo(tex));

	SamplerInitInfo samplerInit;
	SamplerPtr sampler = gr->newSampler(samplerInit);

	// Create the buffer to copy to the texture
	BufferPtr uploadBuff = gr->newBuffer(BufferInitInfo(
		texInit.m_width * texInit.m_height * 3, BufferUsageBit::TRANSFER_ALL, BufferMapAccessBit::WRITE));
	U8* data = static_cast<U8*>(uploadBuff->map(0, uploadBuff->getSize(), BufferMapAccessBit::WRITE));
	for(U i = 0; i < texInit.m_width * texInit.m_height; ++i)
	{
		data[0] = i;
		data[1] = i + 1;
		data[2] = i + 2;
		data += 3;
	}
	uploadBuff->unmap();

	BufferPtr uploadBuff2 = gr->newBuffer(BufferInitInfo(
		(texInit.m_width >> 1) * (texInit.m_height >> 1) * 3, BufferUsageBit::TRANSFER_ALL, BufferMapAccessBit::WRITE));
	data = static_cast<U8*>(uploadBuff2->map(0, uploadBuff2->getSize(), BufferMapAccessBit::WRITE));
	for(U i = 0; i < (texInit.m_width >> 1) * (texInit.m_height >> 1); ++i)
	{
		data[0] = i;
		data[1] = i + 1;
		data[2] = i + 2;
		data += 3;
	}
	uploadBuff2->unmap();

	// Create the result buffer
	BufferPtr resultBuff =
		gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::STORAGE_COMPUTE_WRITE, BufferMapAccessBit::READ));

	// Upload data and test them
	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags =
		CommandBufferFlag::TRANSFER_WORK | CommandBufferFlag::COMPUTE_WORK | CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cmdbInit);

	TextureSubresourceInfo subresource;
	subresource.m_mipmapCount = texInit.m_mipmapCount;
	cmdb->setTextureBarrier(tex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, subresource);
	cmdb->copyBufferToTextureView(uploadBuff,
		0,
		uploadBuff->getSize(),
		gr->newTextureView(TextureViewInitInfo(tex, TextureSurfaceInfo(0, 0, 0, 0))));
	cmdb->copyBufferToTextureView(uploadBuff2,
		0,
		uploadBuff2->getSize(),
		gr->newTextureView(TextureViewInitInfo(tex, TextureSurfaceInfo(1, 0, 0, 0))));

	cmdb->setTextureBarrier(tex, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_COMPUTE, subresource);
	cmdb->bindShaderProgram(prog);
	cmdb->bindTextureAndSampler(0, 0, texView, sampler, TextureUsageBit::SAMPLED_COMPUTE);
	cmdb->bindStorageBuffer(0, 0, resultBuff, 0, resultBuff->getSize());
	cmdb->dispatchCompute(1, 1, 1);

	cmdb->setBufferBarrier(resultBuff,
		BufferUsageBit::STORAGE_COMPUTE_WRITE,
		BufferUsageBit::STORAGE_COMPUTE_WRITE,
		0,
		resultBuff->getSize());

	cmdb->flush();
	gr->finish();

	// Get the result
	UVec4* result = static_cast<UVec4*>(resultBuff->map(0, resultBuff->getSize(), BufferMapAccessBit::READ));
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
ANKI_SPEC_CONST(0, int, const0);
ANKI_SPEC_CONST(2, float, const1);

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
ANKI_SPEC_CONST(0, int, const0);
ANKI_SPEC_CONST(1, float, const1);

layout(location = 0) flat in int in_const0;
layout(location = 1) flat in float in_const1;

layout(location = 0) out vec4 out_color;

layout(ANKI_SS_BINDING(0, 0)) buffer s_
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

	ShaderPtr vert = createShader(VERT_SRC,
		ShaderType::VERTEX,
		*gr,
		Array<ShaderSpecializationConstValue, 3>{{ShaderSpecializationConstValue(2147483647),
			ShaderSpecializationConstValue(-1.0f),
			ShaderSpecializationConstValue(1234.5678f)}});
	ShaderPtr frag = createShader(FRAG_SRC,
		ShaderType::FRAGMENT,
		*gr,
		Array<ShaderSpecializationConstValue, 2>{
			{ShaderSpecializationConstValue(-2147483647), ShaderSpecializationConstValue(-1.0f)}});
	ShaderProgramPtr prog = gr->newShaderProgram(ShaderProgramInitInfo(vert, frag));

	// Create the result buffer
	BufferPtr resultBuff =
		gr->newBuffer(BufferInitInfo(sizeof(UVec4), BufferUsageBit::STORAGE_COMPUTE_WRITE, BufferMapAccessBit::READ));

	// Draw
	gr->beginFrame();

	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

	cmdb->setViewport(0, 0, WIDTH, HEIGHT);
	cmdb->bindShaderProgram(prog);
	cmdb->bindStorageBuffer(0, 0, resultBuff, 0, resultBuff->getSize());
	cmdb->beginRenderPass(createDefaultFb(*gr), {}, {});
	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	cmdb->endRenderPass();
	cmdb->flush();

	gr->swapBuffers();
	gr->finish();

	// Get the result
	UVec4* result = static_cast<UVec4*>(resultBuff->map(0, resultBuff->getSize(), BufferMapAccessBit::READ));
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
ANKI_PUSH_CONSTANTS(PC, regs);
	
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
ANKI_PUSH_CONSTANTS(PC, regs);
	
layout(location = 0) in vec4 in_color;
layout(location = 0) out vec4 out_color;

layout(ANKI_SS_BINDING(0, 0)) buffer s_
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

	ShaderProgramPtr prog = createProgram(VERT_SRC, FRAG_SRC, *gr);

	// Create the result buffer
	BufferPtr resultBuff = gr->newBuffer(
		BufferInitInfo(sizeof(UVec4), BufferUsageBit::STORAGE_ALL | BufferUsageBit::FILL, BufferMapAccessBit::READ));

	// Draw
	gr->beginFrame();

	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
	CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

	cmdb->fillBuffer(resultBuff, 0, resultBuff->getSize(), 0);
	cmdb->setBufferBarrier(
		resultBuff, BufferUsageBit::FILL, BufferUsageBit::STORAGE_FRAGMENT_WRITE, 0, resultBuff->getSize());

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
	cmdb->beginRenderPass(createDefaultFb(*gr), {}, {});
	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	cmdb->endRenderPass();
	cmdb->flush();

	gr->swapBuffers();
	gr->finish();

	// Get the result
	UVec4* result = static_cast<UVec4*>(resultBuff->map(0, resultBuff->getSize(), BufferMapAccessBit::READ));
	ANKI_TEST_EXPECT_EQ(result->x(), 2);
	ANKI_TEST_EXPECT_EQ(result->y(), 2);
	ANKI_TEST_EXPECT_EQ(result->z(), 2);
	ANKI_TEST_EXPECT_EQ(result->w(), 2);
	resultBuff->unmap();

	COMMON_END()
}

} // end namespace anki
