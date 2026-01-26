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

	TextureViewerUiNode(const SceneNodeInitInfo& inf)
		: SceneNode(inf)
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
	const Char* m_imageFilename;

	MyApp(U32 argc, Char** argv, const Char* imageFilename)
		: App("ImageViewer", argc, argv)
		, m_imageFilename(imageFilename)
	{
	}

	Error userPreInit() override
	{
		g_cvarWindowFullscreen = 0;
		g_cvarRsrcDataPaths = ANKI_SOURCE_DIRECTORY;

		return Error::kNone;
	}

	Error userPostInit() override
	{
		// Load the texture
		ImageResourcePtr image;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(m_imageFilename, image, false));

		// Change window name
		String title;
		title.sprintf("%s %u x %u Mips %u Format %s", m_imageFilename, image->getTexture().getWidth(), image->getTexture().getHeight(),
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
	if(argc < 2)
	{
		ANKI_LOGE("Wrong number of arguments");
		return 1;
	}

	Array<Char*, 32> args;
	U32 argCount = 0;
	args[argCount++] = argv[0];
	for(I32 i = 2; i < argc; ++i)
	{
		args[argCount++] = argv[i];
	}

	MyApp* app = new MyApp(argCount, args.getBegin(), argv[1]);
	const Error err = app->mainLoop();
	delete app;

	if(err)
	{
		ANKI_LOGE("Error reported. Bye!!");
		return 1;
	}
	else
	{
		ANKI_LOGI("Bye!!");
		return 0;
	}
}
