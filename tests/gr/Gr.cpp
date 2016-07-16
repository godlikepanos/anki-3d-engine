// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/Gr.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/Config.h>
#include <anki/util/HighRezTimer.h>

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
	const vec2 POSITIONS[3] =
		vec2[](vec2(-1.0, 1.0), vec2(0.0, -1.0), vec2(1.0, 1.0));

	gl_Position = vec4(POSITIONS[gl_VertexID % 3], 0.0, 1.0);
#if defined(ANKI_VK)
	gl_Position.y = -gl_Position.y;
#endif
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

out vec3 out_color;

void main()
{
	const vec2 POSITIONS[3] =
		vec2[](vec2(-1.0, 1.0), vec2(0.0, -1.0), vec2(1.0, 1.0));
		
	out_color = u_color[gl_VertexID].rgb;

	gl_Position = vec4(POSITIONS[gl_VertexID % 3], 0.0, 1.0);
#if defined(ANKI_VK)
	gl_Position.y = -gl_Position.y;
#endif
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
#if defined(ANKI_VK)
	gl_Position.y = -gl_Position.y;
#endif

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
#if defined(ANKI_VK)
	gl_Position.y = -gl_Position.y;
#endif
	out_uv = POSITIONS[gl_VertexID] / 2.0 + 0.5;
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

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex;

void main()
{
	float factor = in_uv.x;
	out_color = vec4(textureLod(u_tex, in_uv, factor).rgb, 1.0);
})";

#define COMMON_BEGIN()                                                         \
	NativeWindow* win = nullptr;                                               \
	GrManager* gr = nullptr;                                                   \
	createGrManager(win, gr)

#define COMMON_END()                                                           \
	delete gr;                                                                 \
	delete win

//==============================================================================
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

//==============================================================================
static void createGrManager(NativeWindow*& win, GrManager*& gr)
{
	win = createWindow();
	gr = new GrManager();

	Config cfg;
	cfg.set("debugContext", 1);
	GrManagerInitInfo inf;
	inf.m_allocCallback = allocAligned;
	inf.m_cacheDirectory = "./";
	inf.m_config = &cfg;
	inf.m_window = win;
	ANKI_TEST_EXPECT_NO_ERR(gr->init(inf));
}

//==============================================================================
static PipelinePtr createSimplePpline(
	CString vertSrc, CString fragSrc, GrManager& gr)
{
	ShaderPtr vert = gr.newInstance<Shader>(ShaderType::VERTEX, vertSrc);
	ShaderPtr frag = gr.newInstance<Shader>(ShaderType::FRAGMENT, fragSrc);

	PipelineInitInfo init;
	init.m_shaders[ShaderType::VERTEX] = vert;
	init.m_shaders[ShaderType::FRAGMENT] = frag;
	init.m_color.m_drawsToDefaultFramebuffer = true;
	init.m_color.m_attachmentCount = 1;
	init.m_depthStencil.m_depthWriteEnabled = false;

	return gr.newInstance<Pipeline>(init);
}

//==============================================================================
static FramebufferPtr createDefaultFb(GrManager& gr)
{
	FramebufferInitInfo fbinit;
	fbinit.m_colorAttachmentCount = 1;
	fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {1.0, 0.0, 1.0, 1.0};

	return gr.newInstance<Framebuffer>(fbinit);
}

//==============================================================================
ANKI_TEST(Gr, GrManager)
{
	COMMON_BEGIN();
	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, Shader)
{
	COMMON_BEGIN();

	{
		ShaderPtr shader =
			gr->newInstance<Shader>(ShaderType::VERTEX, VERT_SRC);
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, Pipeline)
{
	COMMON_BEGIN();

	{
		PipelinePtr ppline = createSimplePpline(VERT_SRC, FRAG_SRC, *gr);
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, SimpleDrawcall)
{
	COMMON_BEGIN();

	{
		PipelinePtr ppline = createSimplePpline(VERT_SRC, FRAG_SRC, *gr);
		FramebufferPtr fb = createDefaultFb(*gr);

		U iterations = 100;
		while(--iterations)
		{
			HighRezTimer timer;
			timer.start();

			gr->beginFrame();

			CommandBufferInitInfo cinit;
			cinit.m_flags =
				CommandBufferFlag::FRAME_FIRST | CommandBufferFlag::FRAME_LAST;
			CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

			cmdb->setViewport(0, 0, WIDTH, HEIGHT);
			cmdb->setPolygonOffset(0.0, 0.0);
			cmdb->bindPipeline(ppline);
			cmdb->beginRenderPass(fb);
			cmdb->drawArrays(3);
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
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, Buffer)
{
	COMMON_BEGIN();

	{
		BufferPtr a = gr->newInstance<Buffer>(
			512, BufferUsageBit::UNIFORM, BufferAccessBit::NONE);

		BufferPtr b = gr->newInstance<Buffer>(64,
			BufferUsageBit::STORAGE,
			BufferAccessBit::CLIENT_MAP_WRITE
				| BufferAccessBit::CLIENT_MAP_READ);

		void* ptr = b->map(0, 64, BufferAccessBit::CLIENT_MAP_WRITE);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
		U8 ptr2[64];
		memset(ptr, 0xCC, 64);
		memset(ptr2, 0xCC, 64);
		b->unmap();

		ptr = b->map(0, 64, BufferAccessBit::CLIENT_MAP_READ);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
		ANKI_TEST_EXPECT_EQ(memcmp(ptr, ptr2, 64), 0);
		b->unmap();
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, ResourceGroup)
{
	COMMON_BEGIN();

	{
		BufferPtr b = gr->newInstance<Buffer>(sizeof(F32) * 4,
			BufferUsageBit::UNIFORM,
			BufferAccessBit::CLIENT_MAP_WRITE);

		ResourceGroupInitInfo rcinit;
		rcinit.m_uniformBuffers[0].m_buffer = b;
		ResourceGroupPtr rc = gr->newInstance<ResourceGroup>(rcinit);
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, DrawWithUniforms)
{
	COMMON_BEGIN();

	{
		// The buffer
		BufferPtr b = gr->newInstance<Buffer>(sizeof(Vec4) * 3,
			BufferUsageBit::UNIFORM,
			BufferAccessBit::CLIENT_MAP_WRITE);

		Vec4* ptr = static_cast<Vec4*>(
			b->map(0, sizeof(Vec4) * 3, BufferAccessBit::CLIENT_MAP_WRITE));
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
		ptr[0] = Vec4(1.0, 0.0, 0.0, 0.0);
		ptr[1] = Vec4(0.0, 1.0, 0.0, 0.0);
		ptr[2] = Vec4(0.0, 0.0, 1.0, 0.0);
		b->unmap();

		// Resource group
		ResourceGroupInitInfo rcinit;
		rcinit.m_uniformBuffers[0].m_buffer = b;
		ResourceGroupPtr rc = gr->newInstance<ResourceGroup>(rcinit);

		// Ppline
		PipelinePtr ppline =
			createSimplePpline(VERT_UBO_SRC, FRAG_UBO_SRC, *gr);

		// FB
		FramebufferPtr fb = createDefaultFb(*gr);

		U iterations = 100;
		while(--iterations)
		{
			HighRezTimer timer;
			timer.start();

			gr->beginFrame();

			CommandBufferInitInfo cinit;
			cinit.m_flags =
				CommandBufferFlag::FRAME_FIRST | CommandBufferFlag::FRAME_LAST;
			CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

			cmdb->setViewport(0, 0, WIDTH, HEIGHT);
			cmdb->setPolygonOffset(0.0, 0.0);
			cmdb->bindPipeline(ppline);
			cmdb->beginRenderPass(fb);
			cmdb->bindResourceGroup(rc, 0, nullptr);
			cmdb->drawArrays(3);
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
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, DrawWithVertex)
{
	COMMON_BEGIN();

	{
		// The buffers
		struct Vert
		{
			Vec3 m_pos;
			Array<U8, 4> m_color;
		};
		static_assert(sizeof(Vert) == sizeof(Vec4), "See file");

		BufferPtr b = gr->newInstance<Buffer>(sizeof(Vert) * 3,
			BufferUsageBit::VERTEX,
			BufferAccessBit::CLIENT_MAP_WRITE);

		Vert* ptr = static_cast<Vert*>(
			b->map(0, sizeof(Vert) * 3, BufferAccessBit::CLIENT_MAP_WRITE));
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

		ptr[0].m_pos = Vec3(-1.0, 1.0, 0.0);
		ptr[1].m_pos = Vec3(0.0, -1.0, 0.0);
		ptr[2].m_pos = Vec3(1.0, 1.0, 0.0);

		ptr[0].m_color = {255, 0, 0};
		ptr[1].m_color = {0, 255, 0};
		ptr[2].m_color = {0, 0, 255};
		b->unmap();

		BufferPtr c = gr->newInstance<Buffer>(sizeof(Vec3) * 3,
			BufferUsageBit::VERTEX,
			BufferAccessBit::CLIENT_MAP_WRITE);

		Vec3* otherColor = static_cast<Vec3*>(
			c->map(0, sizeof(Vec3) * 3, BufferAccessBit::CLIENT_MAP_WRITE));

		otherColor[0] = Vec3(0.0, 1.0, 1.0);
		otherColor[1] = Vec3(1.0, 0.0, 1.0);
		otherColor[2] = Vec3(1.0, 1.0, 0.0);
		c->unmap();

		// Resource group
		ResourceGroupInitInfo rcinit;
		rcinit.m_vertexBuffers[0].m_buffer = b;
		rcinit.m_vertexBuffers[1].m_buffer = c;
		ResourceGroupPtr rc = gr->newInstance<ResourceGroup>(rcinit);

		// Shaders
		ShaderPtr vert =
			gr->newInstance<Shader>(ShaderType::VERTEX, VERT_INP_SRC);
		ShaderPtr frag =
			gr->newInstance<Shader>(ShaderType::FRAGMENT, FRAG_INP_SRC);

		// Ppline
		PipelineInitInfo init;
		init.m_shaders[ShaderType::VERTEX] = vert;
		init.m_shaders[ShaderType::FRAGMENT] = frag;
		init.m_color.m_drawsToDefaultFramebuffer = true;
		init.m_color.m_attachmentCount = 1;
		init.m_depthStencil.m_depthWriteEnabled = false;

		init.m_vertex.m_attributeCount = 3;
		init.m_vertex.m_attributes[0].m_format =
			PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
		init.m_vertex.m_attributes[1].m_format =
			PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
		init.m_vertex.m_attributes[1].m_offset = sizeof(Vec3);
		init.m_vertex.m_attributes[2].m_format =
			PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
		init.m_vertex.m_attributes[2].m_binding = 1;

		init.m_vertex.m_bindingCount = 2;
		init.m_vertex.m_bindings[0].m_stride = sizeof(Vert);
		init.m_vertex.m_bindings[1].m_stride = sizeof(Vec3);

		PipelinePtr ppline = gr->newInstance<Pipeline>(init);

		// FB
		FramebufferPtr fb = createDefaultFb(*gr);

		U iterations = 100;
		while(--iterations)
		{
			HighRezTimer timer;
			timer.start();

			gr->beginFrame();

			CommandBufferInitInfo cinit;
			cinit.m_flags =
				CommandBufferFlag::FRAME_FIRST | CommandBufferFlag::FRAME_LAST;
			CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

			cmdb->setViewport(0, 0, WIDTH, HEIGHT);
			cmdb->setPolygonOffset(0.0, 0.0);
			cmdb->bindPipeline(ppline);
			cmdb->beginRenderPass(fb);
			cmdb->bindResourceGroup(rc, 0, nullptr);
			cmdb->drawArrays(3);
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
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, Sampler)
{
	COMMON_BEGIN();

	{
		SamplerInitInfo init;

		SamplerPtr b = gr->newInstance<Sampler>(init);
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, Texture)
{
	COMMON_BEGIN();

	{
		TextureInitInfo init;
		init.m_depth = 1;
		init.m_format =
			PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
		init.m_usage = TextureUsageBit::FRAGMENT_SHADER_SAMPLED;
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
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, DrawWithTexture)
{
	COMMON_BEGIN();

	{
		//
		// Create the texture
		//
		TextureInitInfo init;
		init.m_depth = 1;
		init.m_format =
			PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
		init.m_usage =
			TextureUsageBit::FRAGMENT_SHADER_SAMPLED | TextureUsageBit::UPLOAD;
		init.m_initialUsage = TextureUsageBit::FRAGMENT_SHADER_SAMPLED;
		init.m_height = 2;
		init.m_width = 2;
		init.m_mipmapsCount = 2;
		init.m_samples = 1;
		init.m_depth = 1;
		init.m_layerCount = 1;
		init.m_sampling.m_repeat = false;
		init.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
		init.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;
		init.m_type = TextureType::_2D;

		TexturePtr b = gr->newInstance<Texture>(init);

		//
		// Upload
		//
		Array<U8, 2 * 2 * 4> mip0 = {
			{255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 0, 255, 0}};

		Array<U8, 4> mip1 = {{128, 128, 128, 0}};

		CommandBufferInitInfo cmdbinit;
		cmdbinit.m_flags = CommandBufferFlag::TRANSFER_WORK;
		CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cmdbinit);

		// Set barriers
		cmdb->setTextureBarrier(b,
			TextureUsageBit::FRAGMENT_SHADER_SAMPLED,
			TextureUsageBit::UPLOAD,
			TextureSurfaceInfo(0, 0, 0, 0));

		cmdb->setTextureBarrier(b,
			TextureUsageBit::FRAGMENT_SHADER_SAMPLED,
			TextureUsageBit::UPLOAD,
			TextureSurfaceInfo(1, 0, 0, 0));

		Error err = ErrorCode::NONE;
		TransientMemoryToken token;
		void* ptr = gr->allocateFrameTransientMemory(
			sizeof(mip0), BufferUsage::TRANSFER, token, &err);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
		ANKI_TEST_EXPECT_NO_ERR(err);
		memcpy(ptr, &mip0[0], sizeof(mip0));

		cmdb->uploadTextureSurface(b, TextureSurfaceInfo(0, 0, 0, 0), token);

		ptr = gr->allocateFrameTransientMemory(
			sizeof(mip1), BufferUsage::TRANSFER, token, &err);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);
		ANKI_TEST_EXPECT_NO_ERR(err);
		memcpy(ptr, &mip1[0], sizeof(mip1));

		cmdb->uploadTextureSurface(b, TextureSurfaceInfo(1, 0, 0, 0), token);

		// Set barriers
		cmdb->setTextureBarrier(b,
			TextureUsageBit::UPLOAD,
			TextureUsageBit::FRAGMENT_SHADER_SAMPLED,
			TextureSurfaceInfo(0, 0, 0, 0));

		cmdb->setTextureBarrier(b,
			TextureUsageBit::UPLOAD,
			TextureUsageBit::FRAGMENT_SHADER_SAMPLED,
			TextureSurfaceInfo(1, 0, 0, 0));

		cmdb->flush();

		//
		// Create resource group
		//
		ResourceGroupInitInfo rcinit;
		rcinit.m_textures[0].m_texture = b;
		ResourceGroupPtr rc = gr->newInstance<ResourceGroup>(rcinit);

		//
		// Create ppline
		//
		PipelinePtr ppline =
			createSimplePpline(VERT_QUAD_SRC, FRAG_TEX_SRC, *gr);

		//
		// Create FB
		//
		FramebufferPtr fb = createDefaultFb(*gr);

		//
		// Draw
		//
		U iterations = 100;
		while(--iterations)
		{
			HighRezTimer timer;
			timer.start();

			gr->beginFrame();

			CommandBufferInitInfo cinit;
			cinit.m_flags =
				CommandBufferFlag::FRAME_FIRST | CommandBufferFlag::FRAME_LAST;
			CommandBufferPtr cmdb = gr->newInstance<CommandBuffer>(cinit);

			cmdb->setViewport(0, 0, WIDTH, HEIGHT);
			cmdb->setPolygonOffset(0.0, 0.0);
			cmdb->bindPipeline(ppline);
			cmdb->beginRenderPass(fb);
			cmdb->bindResourceGroup(rc, 0, nullptr);
			cmdb->drawArrays(6);
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
	}

	COMMON_END();
}

} // end namespace anki
