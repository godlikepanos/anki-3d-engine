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
			[](UiCanvas& canvas, void* ud) {
				static_cast<TextureViewerUiNode*>(ud)->draw(canvas);
			},
			this);

		ANKI_CHECK_AND_IGNORE(ResourceManager::getSingleton().loadResource("ShaderBinaries/UiVisualizeImage.ankiprogbin", m_imageProgram));
	}

	void frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime) override
	{
	}

private:
	ImFont* m_font = nullptr;
	ShaderProgramResourcePtr m_imageProgram;
	ShaderProgramPtr m_imageGrProgram;

	U32 m_crntMip = 0;
	F32 m_zoom = 1.0f;
	F32 m_depth = 0.0f;
	Bool m_pointSampling = true;
	Array<Bool, 4> m_colorChannel = {true, true, true, true};
	F32 m_maxColorValue = 1.0f;

	void draw(UiCanvas& canvas)
	{
		if(!m_font)
		{
			m_font = canvas.addFont("EngineAssets/UbuntuMonoRegular.ttf");
		}

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

		const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
		ImGui::Begin("Console", nullptr, windowFlags);

		ImGui::PushFont(m_font, 16.0f);

		ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
		ImGui::SetWindowSize(Vec2(F32(canvas.getWidth()), F32(canvas.getHeight())));

		ImGui::BeginChild("Tools", Vec2(-1.0f, 30.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX, ImGuiWindowFlags_None);

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

		// Avg color
		{
			const Vec4 avgColor = m_imageResource->getAverageColor();

			ImGui::Text("Average Color %.2f %.2f %.2f %.2f", avgColor.x(), avgColor.y(), avgColor.z(), avgColor.w());
			ImGui::SameLine();

			ImGui::ColorButton("Average color", avgColor);
		}

		// Next
		ImGui::EndChild();

		ImGui::BeginChild("Image", Vec2(-1.0f, -1.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX,
						  ImGuiWindowFlags_HorizontalScrollbar
							  | (Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0 ? ImGuiWindowFlags_NoScrollWithMouse : 0));

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

			ImTextureID texid;
			texid.m_texture = &m_imageResource->getTexture();
			texid.m_textureSubresource = TextureSubresourceDesc::surface(m_crntMip, 0, 0, DepthStencilAspectBit::kNone);
			texid.m_customProgram = m_imageGrProgram.get();
			texid.m_extraFastConstantsSize = U8(sizeof(pc));
			texid.setExtraFastConstants(&pc, sizeof(pc));
			texid.m_pointSampling = m_pointSampling;
			ImGui::Image(texid, imageSize);

			if(ImGui::IsItemHovered())
			{
				if(Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0)
				{
					// Zoom
					const F32 zoomSpeed = 0.05f;
					if(Input::getSingleton().getMouseButton(MouseButton::kScrollDown) > 0)
					{
						m_zoom *= 1.0f - zoomSpeed;
					}
					else if(Input::getSingleton().getMouseButton(MouseButton::kScrollUp) > 0)
					{
						m_zoom *= 1.0f + zoomSpeed;
					}

					// Pan
					if(Input::getSingleton().getMouseButton(MouseButton::kLeft) > 0)
					{
						auto toWindow = [](Vec2 in) {
							in = in * 0.5f + 0.5f;
							in.y() = 1.0f - in.y();
							in *= Vec2(UVec2(NativeWindow::getSingleton().getWidth(), NativeWindow::getSingleton().getHeight()));
							return in;
						};

						const Vec2 delta =
							toWindow(Input::getSingleton().getMousePositionNdc()) - toWindow(Input::getSingleton().getMousePreviousPositionNdc());

						if(delta.x() != 0.0f)
						{
							ImGui::SetScrollX(ImGui::GetScrollX() - delta.x());
						}

						if(delta.y() != 0.0f)
						{
							ImGui::SetScrollY(ImGui::GetScrollY() - delta.y());
						}
					}
				}
			}
		}

		ImGui::EndChild();

		ImGui::PopFont();
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
		node->m_imageResource = std::move(image);

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
