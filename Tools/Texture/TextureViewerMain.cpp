// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class TextureViewerUiNode : public SceneNode
{
public:
	TextureViewerUiNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
		SpatialComponent* spatialc = newComponent<SpatialComponent>();
		spatialc->setAlwaysVisible(true);

		UiComponent* uic = newComponent<UiComponent>();
		uic->init([](CanvasPtr& canvas, const void* ud) { static_cast<const TextureViewerUiNode*>(ud)->draw(canvas); },
				  this);

		ANKI_CHECK_AND_IGNORE(getSceneGraph().getUiManager().newInstance(m_font, "EngineAssets/UbuntuMonoRegular.ttf",
																		 Array<U32, 1>{32}));
	}

private:
	FontPtr m_font;

	void draw(CanvasPtr& canvas) const
	{
		ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);

		canvas->pushFont(m_font, 32);

		ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
		ImGui::SetWindowSize(Vec2(F32(canvas->getWidth()), F32(canvas->getHeight())));

		ImGui::PushStyleColor(ImGuiCol_Text, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::TextWrapped("Test");
		ImGui::PopStyleColor();

		canvas->popFont();
		ImGui::End();
	}
};

class MyApp : public App
{
public:
	Error init(int argc, char** argv, CString appName)
	{
		HeapAllocator<U32> alloc(allocAligned, nullptr);
		StringAuto mainDataPath(alloc, ANKI_SOURCE_DIRECTORY);

		ConfigSet config = DefaultConfigSet::get();
		config.set("window_fullscreen", false);
		config.set("rsrc_dataPaths", mainDataPath);
		config.set("gr_validation", 0);
		ANKI_CHECK(config.setFromCommandLineArguments(argc - 1, argv + 1));

		ANKI_CHECK(App::init(config, allocAligned, nullptr));

		SceneGraph& scene = getSceneGraph();
		TextureViewerUiNode* node;
		ANKI_CHECK(scene.newSceneNode("TextureViewer", node));

		return Error::NONE;
	}

	Error userMainLoop(Bool& quit, Second elapsedTime) override
	{
		Input& input = getInput();
		if(input.getKey(KeyCode::ESCAPE))
		{
			quit = true;
		}

		return Error::NONE;
	}
};

int main(int argc, char* argv[])
{
	Error err = Error::NONE;

	MyApp* app = new MyApp;
	err = app->init(argc, argv, "Texture Viewer");
	if(!err)
	{
		err = app->mainLoop();
	}

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
