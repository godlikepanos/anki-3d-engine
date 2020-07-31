// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/HighRezTimer.h>
#include <anki/Ui.h>
#include <anki/Input.h>
#include <anki/core/StagingGpuMemoryManager.h>

namespace anki
{

class Label : public UiImmediateModeBuilder
{
public:
	using UiImmediateModeBuilder::UiImmediateModeBuilder;

	Bool m_windowInitialized = false;
	U32 m_buttonClickCount = 0;

	void build(CanvasPtr canvas) final
	{
		Vec4 oldBackground = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.8f;

		ImGui::Begin("ImGui Demo", nullptr);

		if(!m_windowInitialized)
		{
			ImGui::SetWindowPos(Vec2(20, 10));
			ImGui::SetWindowSize(Vec2(200, 500));
			m_windowInitialized = true;
		}

		ImGui::Text("Label default size");

		canvas->pushFont(canvas->getDefaultFont(), 30);
		ImGui::Text("Label size 30");
		ImGui::PopFont();

		m_buttonClickCount += ImGui::Button("Toggle");
		if(m_buttonClickCount & 1)
		{
			ImGui::Button("Toggled");
		}

		ImGui::End();
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldBackground;
	}
};

ANKI_TEST(Ui, Ui)
{
	ConfigSet cfg = DefaultConfigSet::get();
	initConfig(cfg);
	cfg.set("gr_vsync", 1);
	cfg.set("gr_debugContext", 0);
	cfg.set("width", 1024);
	cfg.set("height", 760);
	cfg.set("rsrc_dataPaths", "engine_data");

	NativeWindow* win = createWindow(cfg);
	Input* in = new Input();
	GrManager* gr = createGrManager(cfg, win);
	PhysicsWorld* physics;
	ResourceFilesystem* fs;
	ResourceManager* resource = createResourceManager(cfg, gr, physics, fs);
	UiManager* ui = new UiManager();

	ANKI_TEST_EXPECT_NO_ERR(in->init(win));

	StagingGpuMemoryManager* stagingMem = new StagingGpuMemoryManager();
	ANKI_TEST_EXPECT_NO_ERR(stagingMem->init(gr, cfg));

	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ANKI_TEST_EXPECT_NO_ERR(ui->init(allocAligned, nullptr, resource, gr, stagingMem, in));

	{
		FontPtr font;
		ANKI_TEST_EXPECT_NO_ERR(ui->newInstance(font, "UbuntuRegular.ttf", std::initializer_list<U32>{10, 20, 30, 60}));

		CanvasPtr canvas;
		ANKI_TEST_EXPECT_NO_ERR(ui->newInstance(canvas, font, 20, win->getWidth(), win->getHeight()));

		IntrusivePtr<Label> label;
		ANKI_TEST_EXPECT_NO_ERR(ui->newInstance(label));

		Bool done = false;
		while(!done)
		{
			ANKI_TEST_EXPECT_NO_ERR(in->handleEvents());
			HighRezTimer timer;
			timer.start();

			canvas->handleInput();
			if(in->getKey(KeyCode::ESCAPE))
			{
				done = true;
			}

			canvas->beginBuilding();
			label->build(canvas);

			TexturePtr presentTex = gr->acquireNextPresentableTexture();
			FramebufferPtr fb;
			{
				TextureViewInitInfo init;
				init.m_texture = presentTex;
				TextureViewPtr view = gr->newTextureView(init);

				FramebufferInitInfo fbinit;
				fbinit.m_colorAttachmentCount = 1;
				fbinit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 1.0, 1.0}};
				fbinit.m_colorAttachments[0].m_textureView = view;

				fb = gr->newFramebuffer(fbinit);
			}

			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SMALL_BATCH;
			CommandBufferPtr cmdb = gr->newCommandBuffer(cinit);

			cmdb->setTextureBarrier(presentTex, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
									TextureSubresourceInfo());

			cmdb->beginRenderPass(fb, {{TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE}}, {});
			canvas->appendToCommandBuffer(cmdb);
			cmdb->endRenderPass();

			cmdb->setTextureBarrier(presentTex, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::PRESENT,
									TextureSubresourceInfo());

			cmdb->flush();

			gr->swapBuffers();
			stagingMem->endFrame();

			timer.stop();
			const F32 TICK = 1.0f / 30.0f;
			if(timer.getElapsedTime() < TICK)
			{
				HighRezTimer::sleep(TICK - timer.getElapsedTime());
			}
		}
	}

	delete ui;
	delete stagingMem;
	delete resource;
	delete physics;
	delete fs;
	GrManager::deleteInstance(gr);
	delete in;
	delete win;
}

} // end namespace anki
