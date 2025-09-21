// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Ui.h>
#include <AnKi/Window.h>

using namespace anki;

namespace {

class Label
{
public:
	Bool m_windowInitialized = false;
	U32 m_buttonClickCount = 0;

	void build(CanvasPtr canvas)
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

		canvas->pushFont(canvas->getDefaultFont().get(), 30);
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

} // namespace

ANKI_TEST(Ui, Ui)
{
	g_cvarGrVsync = true;
	g_cvarGrValidation = true;
	g_cvarWindowWidth = 1024;
	g_cvarWindowHeight = 760;
	g_cvarRsrcDataPaths = "EngineAssets";

	initWindow();
	ANKI_TEST_EXPECT_NO_ERR(Input::allocateSingleton().init());
	initGrManager();
	ANKI_TEST_EXPECT_NO_ERR(ResourceManager::allocateSingleton().init(allocAligned, nullptr));
	UiManager* ui = &UiManager::allocateSingleton();

	RebarTransientMemoryPool::allocateSingleton().init();

	ANKI_TEST_EXPECT_NO_ERR(ui->init(allocAligned, nullptr));

	{
		FontPtr font;
		ANKI_TEST_EXPECT_NO_ERR(ui->newFont("UbuntuRegular.ttf", Array<U32, 4>{10, 20, 30, 60}, font));

		CanvasPtr canvas;
		ANKI_TEST_EXPECT_NO_ERR(
			ui->newCanvas(font.get(), 20, NativeWindow::getSingleton().getWidth(), NativeWindow::getSingleton().getHeight(), canvas));

		Label label;

		Bool done = false;
		while(!done)
		{
			ANKI_TEST_EXPECT_NO_ERR(Input::getSingleton().handleEvents());
			HighRezTimer timer;
			timer.start();

			GrManager::getSingleton().beginFrame();

			canvas->handleInput();
			if(Input::getSingleton().getKey(KeyCode::kEscape))
			{
				done = true;
			}

			canvas->beginBuilding();
			label.build(canvas);

			TexturePtr presentTex = GrManager::getSingleton().acquireNextPresentableTexture();

			CommandBufferInitInfo cinit;
			cinit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
			CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cinit);

			TextureBarrierInfo barrier;
			barrier.m_previousUsage = TextureUsageBit::kNone;
			barrier.m_nextUsage = TextureUsageBit::kRtvDsvWrite;
			barrier.m_textureView = TextureView(presentTex.get(), TextureSubresourceDesc::all());
			cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

			RenderTarget rt;
			rt.m_textureView = TextureView(presentTex.get(), TextureSubresourceDesc::all());
			rt.m_clearValue.m_colorf = {{1.0, 0.0, 1.0, 1.0}};
			cmdb->beginRenderPass({rt});

			canvas->appendToCommandBuffer(*cmdb);
			cmdb->endRenderPass();

			barrier.m_previousUsage = TextureUsageBit::kRtvDsvWrite;
			barrier.m_nextUsage = TextureUsageBit::kPresent;
			cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

			cmdb->endRecording();
			GrManager::getSingleton().submit(cmdb.get());

			GrManager::getSingleton().endFrame();
			RebarTransientMemoryPool::getSingleton().endFrame();

			timer.stop();
			const F32 TICK = 1.0f / 30.0f;
			if(timer.getElapsedTime() < TICK)
			{
				HighRezTimer::sleep(TICK - timer.getElapsedTime());
			}
		}
	}

	UiManager::freeSingleton();
	RebarTransientMemoryPool::freeSingleton();
	ResourceManager::freeSingleton();
	GrManager::freeSingleton();
	Input::freeSingleton();
	NativeWindow::freeSingleton();
}
