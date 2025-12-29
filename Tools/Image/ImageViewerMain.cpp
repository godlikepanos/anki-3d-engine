// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class TextureViewerUiNode : public SceneNode
{
public:
	ImageViewerUi m_ui;

	TextureViewerUiNode(CString name)
		: SceneNode(name)
	{
		UiComponent* uic = newComponent<UiComponent>();
		uic->init(
			[](UiCanvas& canvas, void* ud) {
				static_cast<TextureViewerUiNode*>(ud)->draw(canvas);
			},
			this);

		m_ui.m_open = true;
	}

private:
	ImFont* m_font = nullptr;

	void draw(UiCanvas& canvas)
	{
		if(!m_font)
		{
			m_font = canvas.addFont("EngineAssets/UbuntuRegular.ttf");
		}

		ImGui::PushFont(m_font, 16.0f);

		ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
		ImGui::SetWindowSize(canvas.getSizef());

		m_ui.drawWindow(canvas, Vec2(0.0f), canvas.getSizef(), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

		ImGui::PopFont();
	}
};

class MyApp : public App
{
public:
	U32 m_argc = 0;
	Char** m_argv = nullptr;

	MyApp(U32 argc, Char** argv)
		: App("ImageViewer")
		, m_argc(argc)
		, m_argv(argv)
	{
	}

	Error userPreInit() override
	{
		if(m_argc < 2)
		{
			ANKI_LOGE("Wrong number of arguments");
			return Error::kUserData;
		}

		g_cvarWindowFullscreen = 0;
		g_cvarRsrcDataPaths = ANKI_SOURCE_DIRECTORY;
		ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 2, m_argv + 2));

		return Error::kNone;
	}

	Error userPostInit() override
	{
		// Load the texture
		ImageResourcePtr image;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(m_argv[1], image, false));

		// Change window name
		String title;
		title.sprintf("%s %u x %u Mips %u Format %s", m_argv[1], image->getTexture().getWidth(), image->getTexture().getHeight(),
					  image->getTexture().getMipmapCount(), getFormatInfo(image->getTexture().getFormat()).m_name);
		NativeWindow::getSingleton().setWindowTitle(title);

		// Create the node
		TextureViewerUiNode* node = SceneGraph::getSingleton().newSceneNode<TextureViewerUiNode>("TextureViewer");
		node->m_ui.m_image = std::move(image);

		return Error::kNone;
	}

	Error userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime) override
	{
		Input& input = Input::getSingleton();
		if(input.getKey(KeyCode::kEscape) > 0)
		{
			quit = true;
		}

		return Error::kNone;
	}
};

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	MyApp* app = new MyApp(argc, argv);
	const Error err = app->mainLoop();
	delete app;

	if(err)
	{
		ANKI_LOGE("Error reported. Bye!!");
	}
	else
	{
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
