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

static const char* FRAG_SRC = R"(layout (location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(0.5);
})";

static const char* FRAG_UBO_SRC = R"(layout (location = 0) out vec4 out_color;

in vec3 in_color;

void main()
{
	out_color = vec4(in_color, 1.0);
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
		ShaderPtr vert = gr->newInstance<Shader>(ShaderType::VERTEX, VERT_SRC);
		ShaderPtr frag =
			gr->newInstance<Shader>(ShaderType::FRAGMENT, FRAG_SRC);

		PipelineInitInfo init;
		init.m_shaders[ShaderType::VERTEX] = vert;
		init.m_shaders[ShaderType::FRAGMENT] = frag;
		init.m_color.m_attachments[0].m_format =
			PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
		init.m_color.m_attachmentCount = 1;
		init.m_depthStencil.m_depthWriteEnabled = false;

		PipelinePtr ppline = gr->newInstance<Pipeline>(init);
	}

	COMMON_END();
}

//==============================================================================
ANKI_TEST(Gr, SimpleDrawcall)
{
	COMMON_BEGIN();

	{
		ShaderPtr vert = gr->newInstance<Shader>(ShaderType::VERTEX, VERT_SRC);
		ShaderPtr frag =
			gr->newInstance<Shader>(ShaderType::FRAGMENT, FRAG_SRC);

		PipelineInitInfo init;
		init.m_shaders[ShaderType::VERTEX] = vert;
		init.m_shaders[ShaderType::FRAGMENT] = frag;
		init.m_color.m_drawsToDefaultFramebuffer = true;
		init.m_color.m_attachmentCount = 1;
		init.m_depthStencil.m_depthWriteEnabled = false;

		PipelinePtr ppline = gr->newInstance<Pipeline>(init);

		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentCount = 1;
		fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {
			1.0, 0.0, 1.0, 1.0};
		FramebufferPtr fb = gr->newInstance<Framebuffer>(fbinit);

		U iterations = 100;
		while(--iterations)
		{
			HighRezTimer timer;
			timer.start();

			gr->beginFrame();

			CommandBufferInitInfo cinit;
			cinit.m_frameFirstCommandBuffer = true;
			cinit.m_frameLastCommandBuffer = true;
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

		// Shaders
		ShaderPtr vert =
			gr->newInstance<Shader>(ShaderType::VERTEX, VERT_UBO_SRC);
		ShaderPtr frag =
			gr->newInstance<Shader>(ShaderType::FRAGMENT, FRAG_UBO_SRC);

		// Ppline
		PipelineInitInfo init;
		init.m_shaders[ShaderType::VERTEX] = vert;
		init.m_shaders[ShaderType::FRAGMENT] = frag;
		init.m_color.m_drawsToDefaultFramebuffer = true;
		init.m_color.m_attachmentCount = 1;
		init.m_depthStencil.m_depthWriteEnabled = false;

		PipelinePtr ppline = gr->newInstance<Pipeline>(init);

		// FB
		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentCount = 1;
		fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {
			1.0, 0.0, 1.0, 1.0};
		FramebufferPtr fb = gr->newInstance<Framebuffer>(fbinit);

		U iterations = 100;
		while(--iterations)
		{
			HighRezTimer timer;
			timer.start();

			gr->beginFrame();

			CommandBufferInitInfo cinit;
			cinit.m_frameFirstCommandBuffer = true;
			cinit.m_frameLastCommandBuffer = true;
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

} // end namespace anki