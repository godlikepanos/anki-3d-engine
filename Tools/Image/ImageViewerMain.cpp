// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class TextureViewerUiNode : public SceneNode
{
public:
	ImageResourcePtr m_imageResource;

	TextureViewerUiNode(CString name)
		: SceneNode(name)
	{
		UiComponent* uic = newComponent<UiComponent>();
		uic->init(
			[](CanvasPtr& canvas, void* ud) {
				static_cast<TextureViewerUiNode*>(ud)->draw(canvas);
			},
			this);

		ANKI_CHECK_AND_IGNORE(UiManager::getSingleton().newInstance(m_font, "EngineAssets/UbuntuMonoRegular.ttf", Array<U32, 1>{16}));

		ANKI_CHECK_AND_IGNORE(ResourceManager::getSingleton().loadResource("ShaderBinaries/UiVisualizeImage.ankiprogbin", m_imageProgram));
	}

	void frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime) override
	{
		if(!m_imageIdExtra.m_textureView.isValid())
		{
			m_imageIdExtra.m_textureView = TextureView(&m_imageResource->getTexture(), TextureSubresourceDesc::all());
		}
	}

private:
	FontPtr m_font;
	ShaderProgramResourcePtr m_imageProgram;
	ShaderProgramPtr m_imageGrProgram;
	UiImageIdData m_imageIdExtra;

	U32 m_crntMip = 0;
	F32 m_zoom = 1.0f;
	F32 m_depth = 0.0f;
	Bool m_pointSampling = true;
	Array<Bool, 4> m_colorChannel = {true, true, true, true};
	F32 m_maxColorValue = 1.0f;

	void draw(CanvasPtr& canvas)
	{
		const Texture& grTex = m_imageResource->getTexture();
		const U32 colorComponentCount = getFormatInfo(grTex.getFormat()).m_componentCount;
		ANKI_ASSERT(grTex.getTextureType() == TextureType::k2D || grTex.getTextureType() == TextureType::k3D);

		if(!m_imageGrProgram.isCreated())
		{
			ShaderProgramResourceVariantInitInfo variantInit(m_imageProgram);
			variantInit.addMutation("TEXTURE_TYPE", (grTex.getTextureType() == TextureType::k2D) ? 0 : 1);

			const ShaderProgramResourceVariant* variant;
			m_imageProgram->getOrCreateVariant(variantInit, variant);
			m_imageGrProgram.reset(&variant->getProgram());
		}

		ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

		canvas->pushFont(m_font, 16);

		ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
		ImGui::SetWindowSize(Vec2(F32(canvas->getWidth()), F32(canvas->getHeight())));

		ImGui::BeginChild("Tools", Vec2(-1.0f, 30.0f), false, ImGuiWindowFlags_AlwaysAutoResize);

		// Zoom
		if(ImGui::Button("-"))
		{
			m_zoom -= 0.1f;
		}
		ImGui::SameLine();
		ImGui::DragFloat("##Zoom", &m_zoom, 0.01f, 0.1f, 20.0f, "Zoom %.3f");
		ImGui::SameLine();
		if(ImGui::Button("+"))
		{
			m_zoom += 0.1f;
		}
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();

		// Sampling
		ImGui::Checkbox("Point sampling", &m_pointSampling);
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();

		// Colors
		ImGui::Checkbox("Red", &m_colorChannel[0]);
		ImGui::SameLine();
		ImGui::Checkbox("Green", &m_colorChannel[1]);
		ImGui::SameLine();
		ImGui::Checkbox("Blue", &m_colorChannel[2]);
		ImGui::SameLine();
		if(colorComponentCount == 4)
		{
			ImGui::Checkbox("Alpha", &m_colorChannel[3]);
			ImGui::SameLine();
		}
		ImGui::Spacing();
		ImGui::SameLine();

		// Mips combo
		{
			UiStringList mipLabels;
			for(U32 mip = 0; mip < grTex.getMipmapCount(); ++mip)
			{
				mipLabels.pushBackSprintf("Mip %u (%u x %u)", mip, grTex.getWidth() >> mip, grTex.getHeight() >> mip);
			}

			const U32 lastCrntMip = m_crntMip;
			if(ImGui::BeginCombo("##Mipmap", (mipLabels.getBegin() + m_crntMip)->cstr(), ImGuiComboFlags_HeightLarge))
			{
				for(U32 mip = 0; mip < grTex.getMipmapCount(); ++mip)
				{
					const Bool isSelected = (m_crntMip == mip);
					if(ImGui::Selectable((mipLabels.getBegin() + mip)->cstr(), isSelected))
					{
						m_crntMip = mip;
					}

					if(isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if(lastCrntMip != m_crntMip)
			{
				// Re-create the image view
				m_imageIdExtra.m_textureView = TextureView(&m_imageResource->getTexture(), TextureSubresourceDesc::surface(m_crntMip, 0, 0));
			}

			ImGui::SameLine();
		}

		// Depth
		if(grTex.getTextureType() == TextureType::k3D)
		{
			UiStringList labels;
			for(U32 d = 0; d < grTex.getDepth(); ++d)
			{
				labels.pushBackSprintf("Depth %u", d);
			}

			if(ImGui::BeginCombo("##Depth", (labels.getBegin() + U32(m_depth))->cstr(), ImGuiComboFlags_HeightLarge))
			{
				for(U32 d = 0; d < grTex.getDepth(); ++d)
				{
					const Bool isSelected = (m_depth == F32(d));
					if(ImGui::Selectable((labels.getBegin() + d)->cstr(), isSelected))
					{
						m_depth = F32(d);
					}

					if(isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
		}

		// Max color slider
		ImGui::SliderFloat("##Max color", &m_maxColorValue, 0.0f, 5.0f, "Max color = %.3f");
		ImGui::SameLine();

		// Next
		ImGui::EndChild();
		ImGui::BeginChild("Image", Vec2(-1.0f, -1.0f), false, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar);

		// Image
		{
			// Center image
			const Vec2 imageSize = Vec2(F32(grTex.getWidth()), F32(grTex.getHeight())) * m_zoom;

			class ExtraPushConstants
			{
			public:
				Vec4 m_colorScale;
				Vec4 m_depth;
			} pc;
			pc.m_colorScale.x() = F32(m_colorChannel[0]) / m_maxColorValue;
			pc.m_colorScale.y() = F32(m_colorChannel[1]) / m_maxColorValue;
			pc.m_colorScale.z() = F32(m_colorChannel[2]) / m_maxColorValue;
			pc.m_colorScale.w() = F32(m_colorChannel[3]);

			pc.m_depth = Vec4((m_depth + 0.5f) / F32(grTex.getDepth()));

			m_imageIdExtra.m_customProgram = m_imageGrProgram;
			m_imageIdExtra.m_extraFastConstantsSize = U8(sizeof(pc));
			m_imageIdExtra.setExtraFastConstants(&pc, sizeof(pc));
			m_imageIdExtra.m_pointSampling = m_pointSampling;

			ImGui::Image(UiImageId(&m_imageIdExtra), imageSize, Vec2(0.0f, 1.0f), Vec2(1.0f, 0.0f), Vec4(1.0f), Vec4(0.0f, 0.0f, 0.0f, 1.0f));

			if(ImGui::IsItemHovered())
			{
				if(ImGui::GetIO().KeyCtrl)
				{
					// Zoom
					const F32 zoomSpeed = 0.05f;
					if(ImGui::GetIO().MouseWheel > 0.0f)
					{
						m_zoom *= 1.0f + zoomSpeed;
					}
					else if(ImGui::GetIO().MouseWheel < 0.0f)
					{
						m_zoom *= 1.0f - zoomSpeed;
					}

					// Pan
					if(ImGui::GetIO().MouseDown[0])
					{
						const Vec2 velocity = toAnki(ImGui::GetIO().MouseDelta);

						if(velocity.x() != 0.0f)
						{
							ImGui::SetScrollX(ImGui::GetScrollX() - velocity.x());
						}

						if(velocity.y() != 0.0f)
						{
							ImGui::SetScrollY(ImGui::GetScrollY() - velocity.y());
						}
					}
				}
			}
		}

		ImGui::EndChild();

		canvas->popFont();
		ImGui::End();
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

		g_windowFullscreenCVar = 0;
		g_dataPathsCVar = ANKI_SOURCE_DIRECTORY;
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
		title.sprintf("%s %u x %u Mips %u Format %s", m_argv[1], image->getWidth(), image->getHeight(), image->getTexture().getMipmapCount(),
					  getFormatInfo(image->getTexture().getFormat()).m_name);
		NativeWindow::getSingleton().setWindowTitle(title);

		// Create the node
		TextureViewerUiNode* node = SceneGraph::getSingleton().newSceneNode<TextureViewerUiNode>("TextureViewer");
		node->m_imageResource = std::move(image);

		return Error::kNone;
	}

	Error userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime) override
	{
		Input& input = Input::getSingleton();
		if(input.getKey(KeyCode::kEscape))
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
