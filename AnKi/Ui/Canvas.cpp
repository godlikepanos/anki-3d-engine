// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/Canvas.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

static void setColorStyleAdia()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Base Colors
	const ImVec4 bgColor = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);
	const ImVec4 lightBgColor = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
	const ImVec4 panelColor = ImVec4(0.17f, 0.18f, 0.19f, 1.00f);
	const ImVec4 panelHoverColor = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
	const ImVec4 panelActiveColor = ImVec4(0.23f, 0.26f, 0.29f, 1.00f);
	const ImVec4 textColor = ImVec4(0.86f, 0.87f, 0.88f, 1.00f);
	const ImVec4 textDisabledColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	const ImVec4 borderColor = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
	const ImVec4 blueColor = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

	// Text
	colors[ImGuiCol_Text] = textColor;
	colors[ImGuiCol_TextDisabled] = textDisabledColor;

	// Windows
	colors[ImGuiCol_WindowBg] = bgColor;
	colors[ImGuiCol_ChildBg] = bgColor;
	colors[ImGuiCol_PopupBg] = bgColor;
	colors[ImGuiCol_Border] = borderColor;
	colors[ImGuiCol_BorderShadow] = borderColor;

	// Headers
	colors[ImGuiCol_Header] = panelColor;
	colors[ImGuiCol_HeaderHovered] = panelHoverColor;
	colors[ImGuiCol_HeaderActive] = panelActiveColor;

	// Buttons
	colors[ImGuiCol_Button] = panelColor;
	colors[ImGuiCol_ButtonHovered] = panelHoverColor;
	colors[ImGuiCol_ButtonActive] = panelActiveColor;

	// Frame BG
	colors[ImGuiCol_FrameBg] = lightBgColor;
	colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
	colors[ImGuiCol_FrameBgActive] = panelActiveColor;

	// Tabs
	colors[ImGuiCol_Tab] = panelColor;
	colors[ImGuiCol_TabHovered] = panelHoverColor;
	colors[ImGuiCol_TabActive] = panelActiveColor;
	colors[ImGuiCol_TabUnfocused] = panelColor;
	colors[ImGuiCol_TabUnfocusedActive] = panelHoverColor;

	// Title
	colors[ImGuiCol_TitleBg] = bgColor;
	colors[ImGuiCol_TitleBgActive] = bgColor;
	colors[ImGuiCol_TitleBgCollapsed] = bgColor;

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = bgColor;
	colors[ImGuiCol_ScrollbarGrab] = panelColor;
	colors[ImGuiCol_ScrollbarGrabHovered] = panelHoverColor;
	colors[ImGuiCol_ScrollbarGrabActive] = panelActiveColor;

	// Checkmark
	colors[ImGuiCol_CheckMark] = blueColor;

	// Slider
	colors[ImGuiCol_SliderGrab] = blueColor;
	colors[ImGuiCol_SliderGrabActive] = blueColor;

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = panelColor;
	colors[ImGuiCol_ResizeGripHovered] = panelHoverColor;
	colors[ImGuiCol_ResizeGripActive] = panelActiveColor;

	// Separator
	colors[ImGuiCol_Separator] = borderColor;
	colors[ImGuiCol_SeparatorHovered] = panelHoverColor;
	colors[ImGuiCol_SeparatorActive] = panelActiveColor;

	// Plot
	colors[ImGuiCol_PlotLines] = textColor;
	colors[ImGuiCol_PlotLinesHovered] = panelActiveColor;
	colors[ImGuiCol_PlotHistogram] = textColor;
	colors[ImGuiCol_PlotHistogramHovered] = panelActiveColor;

	// Text Selected BG
	colors[ImGuiCol_TextSelectedBg] = panelActiveColor;

	// Modal Window Dim Bg
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.105f, 0.11f, 0.5f);

	// Tables
	colors[ImGuiCol_TableHeaderBg] = panelColor;
	colors[ImGuiCol_TableBorderStrong] = borderColor;
	colors[ImGuiCol_TableBorderLight] = borderColor;
	colors[ImGuiCol_TableRowBg] = bgColor;
	colors[ImGuiCol_TableRowBgAlt] = lightBgColor;

	// Styles
	style.FrameBorderSize = 1.0f;
	style.FrameRounding = 2.0f;
	style.WindowBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 2.0f;
	style.GrabMinSize = 7.0f;
	style.GrabRounding = 2.0f;
	style.TabBorderSize = 1.0f;
	style.TabRounding = 2.0f;

	// Reduced Padding and Spacing
	style.WindowPadding = ImVec2(5.0f, 5.0f);
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.ItemSpacing = ImVec2(6.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
}

Canvas::~Canvas()
{
	if(m_imCtx)
	{
		ImGui::DestroyContext(m_imCtx);
	}
}

Error Canvas::init(FontPtr font, U32 fontHeight, U32 width, U32 height)
{
	m_font = font;
	m_dfltFontHeight = fontHeight;
	resize(width, height);

	// Create program
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/Ui.ankiprogbin", m_prog));

	for(U32 i = 0; i < kShaderCount; ++i)
	{
		const ShaderProgramResourceVariant* variant;
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addMutation("TEXTURE_TYPE", i);
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProgs[i].reset(&variant->getProgram());
	}

	// Sampler
	SamplerInitInfo samplerInit("Canvas");
	samplerInit.m_minMagFilter = SamplingFilter::kLinear;
	samplerInit.m_mipmapFilter = SamplingFilter::kLinear;
	samplerInit.m_addressing = SamplingAddressing::kRepeat;
	m_linearLinearRepeatSampler = GrManager::getSingleton().newSampler(samplerInit);

	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kNearest;
	m_nearestNearestRepeatSampler = GrManager::getSingleton().newSampler(samplerInit);

	// Create the context
	m_imCtx = ImGui::CreateContext(font->getImFontAtlas());
	ImGui::SetCurrentContext(m_imCtx);
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;
	// ImGui::StyleColorsLight();
	setColorStyleAdia();

#define ANKI_HANDLE(ak, im) ImGui::GetIO().KeyMap[im] = static_cast<int>(ak);

	ANKI_HANDLE(KeyCode::kTab, ImGuiKey_Tab)
	ANKI_HANDLE(KeyCode::kLeft, ImGuiKey_LeftArrow)
	ANKI_HANDLE(KeyCode::kRight, ImGuiKey_RightArrow)
	ANKI_HANDLE(KeyCode::kUp, ImGuiKey_UpArrow)
	ANKI_HANDLE(KeyCode::kDown, ImGuiKey_DownArrow)
	ANKI_HANDLE(KeyCode::kPageUp, ImGuiKey_PageUp)
	ANKI_HANDLE(KeyCode::kPageDown, ImGuiKey_PageDown)
	ANKI_HANDLE(KeyCode::kHome, ImGuiKey_Home)
	ANKI_HANDLE(KeyCode::kEnd, ImGuiKey_End)
	ANKI_HANDLE(KeyCode::kInsert, ImGuiKey_Insert)
	ANKI_HANDLE(KeyCode::kDelete, ImGuiKey_Delete)
	ANKI_HANDLE(KeyCode::kBackspace, ImGuiKey_Backspace)
	ANKI_HANDLE(KeyCode::kSpace, ImGuiKey_Space)
	ANKI_HANDLE(KeyCode::kReturn, ImGuiKey_Enter)
	// ANKI_HANDLE(KeyCode::RETURN2, ImGuiKey_Enter)
	ANKI_HANDLE(KeyCode::kEscape, ImGuiKey_Escape)
	ANKI_HANDLE(KeyCode::kA, ImGuiKey_A)
	ANKI_HANDLE(KeyCode::kC, ImGuiKey_C)
	ANKI_HANDLE(KeyCode::kV, ImGuiKey_V)
	ANKI_HANDLE(KeyCode::kX, ImGuiKey_X)
	ANKI_HANDLE(KeyCode::kY, ImGuiKey_Y)
	ANKI_HANDLE(KeyCode::kZ, ImGuiKey_Z)

#undef ANKI_HANDLE

	ImGui::SetCurrentContext(nullptr);

	return Error::kNone;
}

void Canvas::handleInput()
{
	const Input& in = Input::getSingleton();

	// Begin
	ImGui::SetCurrentContext(m_imCtx);
	ImGuiIO& io = ImGui::GetIO();

	// Handle mouse
	Array<U32, 4> viewport = {0, 0, m_width, m_height};
	Vec2 mousePosf = in.getMousePosition() / 2.0f + 0.5f;
	mousePosf.y() = 1.0f - mousePosf.y();
	const UVec2 mousePos(U32(mousePosf.x() * F32(viewport[2])), U32(mousePosf.y() * F32(viewport[3])));

	io.MousePos.x = F32(mousePos.x());
	io.MousePos.y = F32(mousePos.y());

	io.MouseClicked[0] = in.getMouseButton(MouseButton::kLeft) == 1;
	io.MouseDown[0] = in.getMouseButton(MouseButton::kLeft) > 0;

	if(in.getMouseButton(MouseButton::kScrollUp) == 1)
	{
		io.MouseWheel = F32(in.getMouseButton(MouseButton::kScrollUp));
	}
	else if(in.getMouseButton(MouseButton::kScrollDown) == 1)
	{
		io.MouseWheel = -F32(in.getMouseButton(MouseButton::kScrollDown));
	}

	io.KeyCtrl = (in.getKey(KeyCode::kLeftCtrl) || in.getKey(KeyCode::kRightCtrl));

// Handle keyboard
#define ANKI_HANDLE(ak) io.KeysDown[int(ak)] = (in.getKey(ak) == 1);

	ANKI_HANDLE(KeyCode::kTab)
	ANKI_HANDLE(KeyCode::kLeft)
	ANKI_HANDLE(KeyCode::kRight)
	ANKI_HANDLE(KeyCode::kUp)
	ANKI_HANDLE(KeyCode::kDown)
	ANKI_HANDLE(KeyCode::kPageUp)
	ANKI_HANDLE(KeyCode::kPageDown)
	ANKI_HANDLE(KeyCode::kHome)
	ANKI_HANDLE(KeyCode::kEnd)
	ANKI_HANDLE(KeyCode::kInsert)
	ANKI_HANDLE(KeyCode::kDelete)
	ANKI_HANDLE(KeyCode::kBackspace)
	ANKI_HANDLE(KeyCode::kSpace)
	ANKI_HANDLE(KeyCode::kReturn)
	// ANKI_HANDLE(KeyCode::RETURN2)
	ANKI_HANDLE(KeyCode::kEscape)
	ANKI_HANDLE(KeyCode::kA)
	ANKI_HANDLE(KeyCode::kC)
	ANKI_HANDLE(KeyCode::kV)
	ANKI_HANDLE(KeyCode::kX)
	ANKI_HANDLE(KeyCode::kY)
	ANKI_HANDLE(KeyCode::kZ)

#undef ANKI_HANDLE

	io.AddInputCharactersUTF8(in.getTextInput().cstr());

	// End
	ImGui::SetCurrentContext(nullptr);
}

void Canvas::beginBuilding()
{
	ImGui::SetCurrentContext(m_imCtx);

	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;
	io.DisplaySize = ImVec2(F32(m_width), F32(m_height));

	ImGui::NewFrame();
	ImGui::PushFont(&m_font->getImFont(m_dfltFontHeight));
}

void Canvas::pushFont(const FontPtr& font, U32 fontHeight)
{
	m_references.pushBack(UiObjectPtr(const_cast<Font*>(font.get())));
	ImGui::PushFont(&font->getImFont(fontHeight));
}

void Canvas::appendToCommandBuffer(CommandBuffer& cmdb)
{
	appendToCommandBufferInternal(cmdb);

	// Done
	ImGui::SetCurrentContext(nullptr);

	m_references.destroy();
}

void Canvas::appendToCommandBufferInternal(CommandBuffer& cmdb)
{
	ImGui::PopFont();
	ImGui::Render();
	ImDrawData& drawData = *ImGui::GetDrawData();

	// Allocate index and vertex buffers
	BufferView vertsToken, indicesToken;
	{
		if(drawData.TotalVtxCount == 0 || drawData.TotalIdxCount == 0)
		{
			return;
		}

		ImDrawVert* verts;
		vertsToken = RebarTransientMemoryPool::getSingleton().allocate(drawData.TotalVtxCount * sizeof(ImDrawVert), sizeof(F32), verts);
		ImDrawIdx* indices;
		indicesToken = RebarTransientMemoryPool::getSingleton().allocate(drawData.TotalIdxCount * sizeof(ImDrawIdx), sizeof(ImDrawIdx), indices);

		for(I n = 0; n < drawData.CmdListsCount; ++n)
		{
			const ImDrawList& cmdList = *drawData.CmdLists[n];
			memcpy(verts, cmdList.VtxBuffer.Data, cmdList.VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(indices, cmdList.IdxBuffer.Data, cmdList.IdxBuffer.Size * sizeof(ImDrawIdx));
			verts += cmdList.VtxBuffer.Size;
			indices += cmdList.IdxBuffer.Size;
		}
	}

	cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb.setCullMode(FaceSelectionBit::kNone);

	const F32 fbWidth = drawData.DisplaySize.x * drawData.FramebufferScale.x;
	const F32 fbHeight = drawData.DisplaySize.y * drawData.FramebufferScale.y;
	cmdb.setViewport(0, 0, U32(fbWidth), U32(fbHeight));

	cmdb.bindVertexBuffer(0, vertsToken, sizeof(ImDrawVert));
	cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32_Sfloat, 0);
	cmdb.setVertexAttribute(VertexAttributeSemantic::kColor, 0, Format::kR8G8B8A8_Unorm, sizeof(Vec2) * 2);
	cmdb.setVertexAttribute(VertexAttributeSemantic::kTexCoord, 0, Format::kR32G32_Sfloat, sizeof(Vec2));

	cmdb.bindIndexBuffer(indicesToken, IndexType::kU16);

	// Will project scissor/clipping rectangles into framebuffer space
	const Vec2 clipOff = drawData.DisplayPos; // (0,0) unless using multi-viewports
	const Vec2 clipScale = drawData.FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render
	U32 vertOffset = 0;
	U32 idxOffset = 0;
	for(I32 n = 0; n < drawData.CmdListsCount; n++)
	{
		const ImDrawList& cmdList = *drawData.CmdLists[n];
		for(I32 i = 0; i < cmdList.CmdBuffer.Size; i++)
		{
			const ImDrawCmd& pcmd = cmdList.CmdBuffer[i];
			if(pcmd.UserCallback)
			{
				// User callback (registered via ImDrawList::AddCallback), ignore
				ANKI_UI_LOGW("Got user callback. Will ignore");
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				Vec4 clipRect;
				clipRect.x() = (pcmd.ClipRect.x - clipOff.x()) * clipScale.x();
				clipRect.y() = (pcmd.ClipRect.y - clipOff.y()) * clipScale.y();
				clipRect.z() = (pcmd.ClipRect.z - clipOff.x()) * clipScale.x();
				clipRect.w() = (pcmd.ClipRect.w - clipOff.y()) * clipScale.y();

				if(clipRect.x() < fbWidth && clipRect.y() < fbHeight && clipRect.z() >= 0.0f && clipRect.w() >= 0.0f)
				{
					// Negative offsets are illegal for vkCmdSetScissor
					if(clipRect.x() < 0.0f)
					{
						clipRect.x() = 0.0f;
					}
					if(clipRect.y() < 0.0f)
					{
						clipRect.y() = 0.0f;
					}

					// Apply scissor/clipping rectangle
					cmdb.setScissor(U32(clipRect.x()), U32(clipRect.y()), U32(clipRect.z() - clipRect.x()), U32(clipRect.w() - clipRect.y()));

					UiImageId id(pcmd.TextureId);
					const UiImageIdData* data = id.m_data;

					// Bind program
					if(data && data->m_customProgram.isCreated())
					{
						cmdb.bindShaderProgram(data->m_customProgram.get());
					}
					else if(data && data->m_textureView.isValid())
					{
						cmdb.bindShaderProgram(m_grProgs[kRgbaTex].get());
					}
					else
					{
						cmdb.bindShaderProgram(m_grProgs[kNoTex].get());
					}

					// Bindings
					if(data && data->m_textureView.isValid())
					{
						cmdb.bindSampler(0, 0, (data->m_pointSampling) ? m_nearestNearestRepeatSampler.get() : m_linearLinearRepeatSampler.get());
						cmdb.bindSrv(0, 0, data->m_textureView);
					}

					// Push constants
					class PC
					{
					public:
						Vec4 m_transform;
						Array<U8, sizeof(UiImageIdData::m_extraFastConstants)> m_extra;
					} pc;
					pc.m_transform.x() = 2.0f / drawData.DisplaySize.x;
					pc.m_transform.y() = -2.0f / drawData.DisplaySize.y;
					pc.m_transform.z() = (drawData.DisplayPos.x / drawData.DisplaySize.x) * 2.0f - 1.0f;
					pc.m_transform.w() = -((drawData.DisplayPos.y / drawData.DisplaySize.y) * 2.0f - 1.0f);
					U32 extraFastConstantsSize = 0;
					if(data && data->m_extraFastConstantsSize)
					{
						ANKI_ASSERT(data->m_extraFastConstantsSize <= sizeof(data->m_extraFastConstants));
						memcpy(&pc.m_extra[0], data->m_extraFastConstants.getBegin(), data->m_extraFastConstantsSize);

						extraFastConstantsSize = data->m_extraFastConstantsSize;
					}
					cmdb.setFastConstants(&pc, sizeof(Vec4) + extraFastConstantsSize);

					// Draw
					cmdb.drawIndexed(PrimitiveTopology::kTriangles, pcmd.ElemCount, 1, idxOffset, vertOffset);
				}
			}
			idxOffset += pcmd.ElemCount;
		}
		vertOffset += cmdList.VtxBuffer.Size;
	}

	// Restore state
	cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb.setCullMode(FaceSelectionBit::kBack);
}

} // end namespace anki
