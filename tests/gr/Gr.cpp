// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/Gr.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/Config.h>
#include <anki/util/HighRezTimer.h>
#include <anki/core/StagingGpuMemoryManager.h>

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

	ANKI_WRITE_POSITION(vec4(POSITIONS[gl_VertexID % 3], 0.0, 1.0));
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

	ANKI_WRITE_POSITION(vec4(POSITIONS[gl_VertexID], 0.0, 1.0));
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
	ANKI_WRITE_POSITION(u_mvp * vec4(in_pos, 1.0));
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
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_tex1;

ANKI_USING_FRAG_COORD(768)

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

#define COMMON_BEGIN()                                                                                                 \
	stagingMem = new StagingGpuMemoryManager();                                                                        \
	createGrManager(win, gr);                                                                                          \
	ANKI_TEST_EXPECT_NO_ERR(stagingMem->init(gr, Config()));                                                           \
	{

#define COMMON_END()                                                                                                   \
	}                                                                                                                  \
	delete stagingMem;                                                                                                 \
	delete gr;                                                                                                         \
	delete win;                                                                                                        \
	win = nullptr;                                                                                                     \
	gr = nullptr;                                                                                                      \
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

#define UPLOAD_TEX_SURFACE(cmdb_, tex_, surf_, ptr_, size_)                                                            \
	do                                                                                                                 \
	{                                                                                                                  \
		StagingGpuMemoryToken token;                                                                                   \
		void* f = stagingMem->allocateFrame(size_, StagingGpuMemoryType::TRANSFER, token);                             \
		memcpy(f, ptr_, size_);                                                                                        \
		cmdb_->copyBufferToTextureSurface(token.m_buffer, token.m_offset, token.m_range, tex_, surf_);                 \
	} while(0)

#define UPLOAD_TEX_VOL(cmdb_, tex_, vol_, ptr_, size_)                                                                 \
	do                                                                                                                 \
	{                                                                                                                  \
		StagingGpuMemoryToken token;                                                                                   \
		void* f = stagingMem->allocateFrame(size_, StagingGpuMemoryType::TRANSFER, token);                             \
		memcpy(f, ptr_, size_);                                                                                        \
		cmdb_->copyBufferToTextureVolume(token.m_buffer, token.m_offset, token.m_range, tex_, vol_);                   \
	} while(0)

const PixelFormat DS_FORMAT = PixelFormat(ComponentFormat::D24S8, TransformFormat::UNORM);

static NativeWindow* createWindow()
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	NativeWindowInitInfo inf;
	inf.m_width = WIDTH;
	inf.m_height = HEIGHT;
	NativeWindow* win = new NativeWindow();

	ANKI_TEST_EXPECT_NO_ERR(win->init(inf, alloc));

	return win;
}

static void createGrManager(NativeWindow*& win, GrManager*& gr)
{
	win = createWindow();
	gr = new GrManager();

	Config cfg;
	cfg.set("debugContext", true);
	cfg.set("vsync", false);
	GrManagerInitInfo inf;
	inf.m_allocCallback = allocAligned;
	inf.m_cacheDirectory = "./";
	inf.m_config = &cfg;
	inf.m_window = win;
	ANKI_TEST_EXPECT_NO_ERR(gr->init(inf));
}

static ShaderProgramPtr createProgram(CString vertSrc, CString fragSrc, GrManager& gr)
{
	ShaderPtr vert = gr.newInstance<Shader>(ShaderType::VERTEX, vertSrc);
	ShaderPtr frag = gr.newInstance<Shader>(ShaderType::FRAGMENT, fragSrc);
	return gr.newInstance<ShaderProgram>(vert, frag);
}

static FramebufferPtr createDefaultFb(GrManager& gr)
{
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 1;
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 1.0, 1.0}};

	return gr.newInstance<Framebuffer>(fbinit);
}

static void createCube(GrManager& gr, BufferPtr& verts, BufferPtr& indices)
{
	static const Array<F32, 8 * 3> pos = {
		{1, 1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, 1, 1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1}};

	static const Array<U16, 6 * 2 * 3> idx = {
		{0, 1, 3, 3, 1, 2, 1, 5, 6, 1, 6, 2, 7, 4, 0, 7, 0, 3, 6, 5, 7, 7, 5, 4, 0, 4, 5, 0, 5, 1, 3, 2, 6, 3, 6, 7}};

	verts = gr.newInstance<Buffer>(sizeof(pos), BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE);

	void* mapped = verts->map(0, sizeof(pos), BufferMapAccessBit::WRITE);
	memcpy(mapped, &pos[0], sizeof(pos));
	verts->unmap();

	indices = gr.newInstance<Buffer>(sizeof(idx), BufferUsageBit::INDEX, BufferMapAccessBit::WRITE);
	mapped = indices->map(0, sizeof(idx), BufferMapAccessBit::WRITE);
	memcpy(mapped, &idx[0], sizeof(idx));
	indices->unmap();
}

ANKI_TEST(Gr, GrManager)
{
	COMMON_BEGIN()
	COMMON_END()
}

ANKI_TEST(Gr, Shader)
{
	COMMON_BEGIN()

	ShaderPtr shader = gr->newInstance<Shader>(ShaderType::FRAGMENT, FRAG_MRT_SRC);

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

	FramebufferPtr fb = createDefaultFb(*gr);

	U iterations = 100;
	while(iterations--)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		cmdb->beginRenderPass(fb);
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

	ANKI_TEST_LOGI("Expect to see a grey triangle appearing in the 4 corners");
	ShaderProgramPtr prog = createProgram(VERT_SRC, FRAG_SRC, *gr);
	FramebufferPtr fb = createDefaultFb(*gr);

	static const Array2d<U, 4, 4> VIEWPORTS = {{{{0, 0, WIDTH / 2, HEIGHT / 2}},
		{{WIDTH / 2, 0, WIDTH, HEIGHT / 2}},
		{{WIDTH / 2, HEIGHT / 2, WIDTH, HEIGHT}},
		{{0, HEIGHT / 2, WIDTH / 2, HEIGHT}}}};

	const U ITERATIONS = 200;
	for(U i = 0; i < ITERATIONS; ++i)
	{
		HighRezTimer timer;
		timer.start();

		gr->beginFrame();

		CommandBufferInitInfo cinit;
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		auto vp = VIEWPORTS[(i / 30) % 4];
		cmdb->setViewport(vp[0], vp[1], vp[2], vp[3]);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb);
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

ANKI_TEST(Gr, Buffer)
{
	COMMON_BEGIN()

	BufferPtr a = gr->newInstance<Buffer>(512, BufferUsageBit::UNIFORM_ALL, BufferMapAccessBit::NONE);

	BufferPtr b =
		gr->newInstance<Buffer>(64, BufferUsageBit::STORAGE_ALL, BufferMapAccessBit::WRITE | BufferMapAccessBit::READ);

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
	BufferPtr b = gr->newInstance<Buffer>(sizeof(Vec4) * 3, BufferUsageBit::UNIFORM_ALL, BufferMapAccessBit::WRITE);

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
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb);

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

	BufferPtr b = gr->newInstance<Buffer>(sizeof(Vert) * 3, BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE);

	Vert* ptr = static_cast<Vert*>(b->map(0, sizeof(Vert) * 3, BufferMapAccessBit::WRITE));
	ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

	ptr[0].m_pos = Vec3(-1.0, 1.0, 0.0);
	ptr[1].m_pos = Vec3(0.0, -1.0, 0.0);
	ptr[2].m_pos = Vec3(1.0, 1.0, 0.0);

	ptr[0].m_color = {{255, 0, 0}};
	ptr[1].m_color = {{0, 255, 0}};
	ptr[2].m_color = {{0, 0, 255}};
	b->unmap();

	BufferPtr c = gr->newInstance<Buffer>(sizeof(Vec3) * 3, BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE);

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
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		cmdb->bindVertexBuffer(0, b, 0, sizeof(Vert));
		cmdb->bindVertexBuffer(1, c, 0, sizeof(Vec3));
		cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);
		cmdb->setVertexAttribute(1, 0, PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM), sizeof(Vec3));
		cmdb->setVertexAttribute(2, 1, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->setPolygonOffset(0.0, 0.0);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb);
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

	SamplerPtr b = gr->newInstance<Sampler>(init);

	COMMON_END()
}

ANKI_TEST(Gr, Texture)
{
	COMMON_BEGIN()

	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT;
	init.m_height = 4;
	init.m_width = 4;
	init.m_mipmapsCount = 2;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
	init.m_type = TextureType::_2D;

	TexturePtr b = gr->newInstance<Texture>(init);

	COMMON_END()
}

ANKI_TEST(Gr, DrawWithTexture)
{
	COMMON_BEGIN()

	//
	// Create texture A
	//
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::TRANSFER_DESTINATION;
	init.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapsCount = 2;
	init.m_samples = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_sampling.m_repeat = false;
	init.m_sampling.m_minMagFilter = SamplingFilter::NEAREST;
	init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
	init.m_type = TextureType::_2D;

	TexturePtr a = gr->newInstance<Texture>(init);

	//
	// Create texture B
	//
	init.m_width = 4;
	init.m_height = 4;
	init.m_mipmapsCount = 3;
	init.m_usage =
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::TRANSFER_DESTINATION | TextureUsageBit::GENERATE_MIPMAPS;
	init.m_initialUsage = TextureUsageBit::NONE;

	TexturePtr b = gr->newInstance<Texture>(init);

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
	CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cmdbinit);

	// Set barriers
	cmdb->setTextureSurfaceBarrier(
		a, TextureUsageBit::SAMPLED_FRAGMENT, TextureUsageBit::TRANSFER_DESTINATION, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(
		a, TextureUsageBit::SAMPLED_FRAGMENT, TextureUsageBit::TRANSFER_DESTINATION, TextureSurfaceInfo(1, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(
		b, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, TextureSurfaceInfo(0, 0, 0, 0));

	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceInfo(0, 0, 0, 0), &mip0[0], sizeof(mip0));

	UPLOAD_TEX_SURFACE(cmdb, a, TextureSurfaceInfo(1, 0, 0, 0), &mip1[0], sizeof(mip1));

	UPLOAD_TEX_SURFACE(cmdb, b, TextureSurfaceInfo(0, 0, 0, 0), &bmip0[0], sizeof(bmip0));

	// Gen mips
	cmdb->setTextureSurfaceBarrier(
		b, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::GENERATE_MIPMAPS, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->generateMipmaps2d(b, 0, 0);

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

	cmdb->flush();

	//
	// Create prog
	//
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_TEX_SRC, *gr);

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
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(fb);

		for(U i = 0; i < 2; ++i)
		{
			cmdb->informTextureSurfaceCurrentUsage(
				a, TextureSurfaceInfo(i, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);
			cmdb->informTextureSurfaceCurrentUsage(
				b, TextureSurfaceInfo(i, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);
		}
		cmdb->informTextureSurfaceCurrentUsage(b, TextureSurfaceInfo(2, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);

		cmdb->bindTexture(0, 0, a);
		cmdb->bindTexture(0, 1, b);
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
	cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);
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
	const PixelFormat COL_FORMAT = PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
	const U TEX_SIZE = 256;

	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = COL_FORMAT;
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	init.m_height = TEX_SIZE;
	init.m_width = TEX_SIZE;
	init.m_mipmapsCount = 1;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_samples = 1;
	init.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
	init.m_type = TextureType::_2D;

	TexturePtr col0 = gr.newInstance<Texture>(init);
	TexturePtr col1 = gr.newInstance<Texture>(init);

	init.m_format = DS_FORMAT;
	TexturePtr dp = gr.newInstance<Texture>(init);

	//
	// Create FB
	//
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 2;
	fbinit.m_colorAttachments[0].m_texture = col0;
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{0.1, 0.0, 0.0, 0.0}};
	fbinit.m_colorAttachments[1].m_texture = col1;
	fbinit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0, 0.1, 0.0, 0.0}};
	fbinit.m_depthStencilAttachment.m_texture = dp;
	fbinit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	fbinit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	FramebufferPtr fb = gr.newInstance<Framebuffer>(fbinit);

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
		CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>(cinit);

		cmdb->setPolygonOffset(0.0, 0.0);

		cmdb->setTextureSurfaceBarrier(
			col0, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(
			col1, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(dp,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->beginRenderPass(fb);

		if(!useSecondLevel)
		{
			drawOffscreenDrawcalls(gr, prog, cmdb, TEX_SIZE, indices, verts);
		}
		else
		{
			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
			cinit.m_framebuffer = fb;
			CommandBufferPtr cmdb2 = gr.newInstance<CommandBuffer>(cinit);

			cmdb2->informTextureSurfaceCurrentUsage(
				col0, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
			cmdb2->informTextureSurfaceCurrentUsage(
				col1, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
			cmdb2->informTextureSurfaceCurrentUsage(
				dp, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

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
		cmdb->beginRenderPass(dfb);
		cmdb->bindShaderProgram(resolveProg);
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->bindTexture(0, 0, col0);
		cmdb->bindTexture(0, 1, col1);
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

	TextureInitInfo init;
	init.m_width = init.m_height = 4;
	init.m_mipmapsCount = 2;
	init.m_usage = TextureUsageBit::CLEAR | TextureUsageBit::SAMPLED_ALL | TextureUsageBit::IMAGE_COMPUTE_WRITE;
	init.m_type = TextureType::_2D;
	init.m_format = PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
	init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	TexturePtr tex = gr->newInstance<Texture>(init);

	// Prog
	ShaderProgramPtr prog = createProgram(VERT_QUAD_SRC, FRAG_SIMPLE_TEX_SRC, *gr);

	// Create shader & compute prog
	ShaderPtr shader = gr->newInstance<Shader>(ShaderType::COMPUTE, COMP_WRITE_IMAGE_SRC);
	ShaderProgramPtr compProg = gr->newInstance<ShaderProgram>(shader);

	// FB
	FramebufferPtr dfb = createDefaultFb(*gr);

	// Write texture data
	CommandBufferInitInfo cmdbinit;
	CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cmdbinit);

	cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::NONE, TextureUsageBit::CLEAR, TextureSurfaceInfo(0, 0, 0, 0));

	ClearValue clear;
	clear.m_colorf = {{0.0, 1.0, 0.0, 1.0}};
	cmdb->clearTextureSurface(tex, TextureSurfaceInfo(0, 0, 0, 0), clear);

	cmdb->setTextureSurfaceBarrier(
		tex, TextureUsageBit::CLEAR, TextureUsageBit::SAMPLED_FRAGMENT, TextureSurfaceInfo(0, 0, 0, 0));

	cmdb->setTextureSurfaceBarrier(tex, TextureUsageBit::NONE, TextureUsageBit::CLEAR, TextureSurfaceInfo(1, 0, 0, 0));

	clear.m_colorf = {{0.0, 0.0, 1.0, 1.0}};
	cmdb->clearTextureSurface(tex, TextureSurfaceInfo(1, 0, 0, 0), clear);

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
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::COMPUTE_WORK;
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		// Write image
		Vec4* col = SET_STORAGE(Vec4*, sizeof(*col), cmdb, 1, 0);
		*col = Vec4(iterations / F32(ITERATION_COUNT));

		cmdb->setTextureSurfaceBarrier(
			tex, TextureUsageBit::NONE, TextureUsageBit::IMAGE_COMPUTE_WRITE, TextureSurfaceInfo(1, 0, 0, 0));
		cmdb->bindShaderProgram(compProg);
		cmdb->bindImage(0, 0, tex, 1);
		cmdb->dispatchCompute(WIDTH / 2, HEIGHT / 2, 1);
		cmdb->setTextureSurfaceBarrier(tex,
			TextureUsageBit::IMAGE_COMPUTE_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(1, 0, 0, 0));

		// Present image
		cmdb->setViewport(0, 0, WIDTH, HEIGHT);

		cmdb->bindShaderProgram(prog);
		cmdb->beginRenderPass(dfb);
		cmdb->informTextureSurfaceCurrentUsage(tex, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);
		cmdb->bindTexture(0, 0, tex);
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

	//
	// Create texture A
	//
	TextureInitInfo init;
	init.m_depth = 1;
	init.m_format = PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
	init.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::TRANSFER_DESTINATION;
	init.m_initialUsage = TextureUsageBit::TRANSFER_DESTINATION;
	init.m_usageWhenEncountered = TextureUsageBit::SAMPLED_FRAGMENT;
	init.m_height = 2;
	init.m_width = 2;
	init.m_mipmapsCount = 2;
	init.m_samples = 1;
	init.m_depth = 2;
	init.m_layerCount = 1;
	init.m_sampling.m_repeat = false;
	init.m_sampling.m_minMagFilter = SamplingFilter::NEAREST;
	init.m_sampling.m_mipmapFilter = SamplingFilter::NEAREST;
	init.m_type = TextureType::_3D;

	TexturePtr a = gr->newInstance<Texture>(init);

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
	cmdbinit.m_flags = CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cmdbinit);

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, TextureVolumeInfo(0));

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, TextureVolumeInfo(1));

	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeInfo(0), &mip0[0], sizeof(mip0));

	UPLOAD_TEX_VOL(cmdb, a, TextureVolumeInfo(1), &mip1[0], sizeof(mip1));

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, TextureVolumeInfo(0));

	cmdb->setTextureVolumeBarrier(
		a, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::SAMPLED_FRAGMENT, TextureVolumeInfo(1));

	cmdb->flush();

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
		cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK;
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

		cmdb->setViewport(0, 0, WIDTH, HEIGHT);
		cmdb->beginRenderPass(dfb);

		cmdb->bindShaderProgram(prog);

		Vec4* uv = SET_UNIFORMS(Vec4*, sizeof(Vec4), cmdb, 0, 0);

		U idx = (F32(ITERATION_COUNT - iterations - 1) / ITERATION_COUNT) * TEX_COORDS_LOD.getSize();
		*uv = TEX_COORDS_LOD[idx];

		cmdb->bindTexture(0, 0, a);
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

} // end namespace anki
