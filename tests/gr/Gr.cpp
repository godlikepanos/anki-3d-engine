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

#if defined(ANKI_GL)
#define gl_VertexIndex gl_VertexID
#elif defined(ANKI_VK)
// Do nothing
#else
#error See file
#endif

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[3] =
		vec2[](vec2(-1.0, 1.0), vec2(0.0, -1.0), vec2(1.0, 1.0));

	gl_Position = vec4(POSITIONS[gl_VertexIndex % 3], 0.0, 1.0);
#if defined(ANKI_VK)
	gl_Position.y = -gl_Position.y;
#endif
})";

static const char* FRAG_SRC = R"(layout (location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(0.5);
})";

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
	NativeWindow* win = nullptr;
	GrManager* gr = nullptr;
	createGrManager(win, gr);

	delete gr;
	delete win;
}

//==============================================================================
ANKI_TEST(Gr, Shader)
{
	NativeWindow* win = nullptr;
	GrManager* gr = nullptr;
	createGrManager(win, gr);

	{
		ShaderPtr shader =
			gr->newInstance<Shader>(ShaderType::VERTEX, VERT_SRC);
	}

	delete gr;
	delete win;
}

//==============================================================================
ANKI_TEST(Gr, Pipeline)
{
	NativeWindow* win = nullptr;
	GrManager* gr = nullptr;
	createGrManager(win, gr);

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

	delete gr;
	delete win;
}

//==============================================================================
ANKI_TEST(Gr, SimpleDrawcall)
{
	NativeWindow* win = nullptr;
	GrManager* gr = nullptr;
	createGrManager(win, gr);

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

	delete gr;
	delete win;
}

} // end namespace anki