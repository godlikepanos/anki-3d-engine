// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/Canvas.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Input/Input.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Canvas::Canvas(UiManager* manager)
	: UiObject(manager)
{
}

Canvas::~Canvas()
{
	if(m_imCtx)
	{
		setImAllocator();
		ImGui::DestroyContext(m_imCtx);
		unsetImAllocator();
	}
}

Error Canvas::init(FontPtr font, U32 fontHeight, U32 width, U32 height)
{
	m_font = font;
	m_dfltFontHeight = fontHeight;
	resize(width, height);

	// Create program
	ANKI_CHECK(m_manager->getResourceManager().loadResource("ShaderBinaries/Ui.ankiprogbin", m_prog));

	for(U32 i = 0; i < kShaderCount; ++i)
	{
		const ShaderProgramResourceVariant* variant;
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addMutation("TEXTURE_TYPE", i);
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProgs[i] = variant->getProgram();
	}

	// Sampler
	SamplerInitInfo samplerInit("Canvas");
	samplerInit.m_minMagFilter = SamplingFilter::kLinear;
	samplerInit.m_mipmapFilter = SamplingFilter::kLinear;
	samplerInit.m_addressing = SamplingAddressing::kRepeat;
	m_linearLinearRepeatSampler = m_manager->getGrManager().newSampler(samplerInit);

	samplerInit.m_minMagFilter = SamplingFilter::kNearest;
	samplerInit.m_mipmapFilter = SamplingFilter::kNearest;
	m_nearestNearestRepeatSampler = m_manager->getGrManager().newSampler(samplerInit);

	// Allocator
	m_tempPool.init(getMemoryPool().getAllocationCallback(), getMemoryPool().getAllocationCallbackUserData(), 512_B);

	// Create the context
	setImAllocator();
	m_imCtx = ImGui::CreateContext(font->getImFontAtlas());
	ImGui::SetCurrentContext(m_imCtx);
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;
	ImGui::StyleColorsLight();

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
	unsetImAllocator();

	return Error::kNone;
}

void Canvas::handleInput()
{
	const Input& in = m_manager->getInput();

	// Begin
	setImAllocator();
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
	unsetImAllocator();
}

void Canvas::beginBuilding()
{
	setImAllocator();
	ImGui::SetCurrentContext(m_imCtx);

	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = 1.0f / 60.0f;
	io.DisplaySize = ImVec2(F32(m_width), F32(m_height));

	ImGui::NewFrame();
	ImGui::PushFont(&m_font->getImFont(m_dfltFontHeight));
}

void Canvas::pushFont(const FontPtr& font, U32 fontHeight)
{
	m_references.pushBack(m_tempPool, IntrusivePtr<UiObject>(const_cast<Font*>(font.get())));
	ImGui::PushFont(&font->getImFont(fontHeight));
}

void Canvas::appendToCommandBuffer(CommandBufferPtr cmdb)
{
	appendToCommandBufferInternal(cmdb);

	// Done
	ImGui::SetCurrentContext(nullptr);

	m_references.destroy(m_tempPool);
	m_tempPool.reset();
}

void Canvas::appendToCommandBufferInternal(CommandBufferPtr& cmdb)
{
	ImGui::PopFont();
	ImGui::Render();
	ImDrawData& drawData = *ImGui::GetDrawData();

	// Allocate index and vertex buffers
	StagingGpuMemoryToken vertsToken, indicesToken;
	{
		const U32 verticesSize = U32(drawData.TotalVtxCount) * sizeof(ImDrawVert);
		const U32 indicesSize = U32(drawData.TotalIdxCount) * sizeof(ImDrawIdx);

		if(verticesSize == 0 || indicesSize == 0)
		{
			return;
		}

		ImDrawVert* verts = static_cast<ImDrawVert*>(
			m_manager->getStagingGpuMemory().allocateFrame(verticesSize, StagingGpuMemoryType::kVertex, vertsToken));
		ImDrawIdx* indices = static_cast<ImDrawIdx*>(
			m_manager->getStagingGpuMemory().allocateFrame(indicesSize, StagingGpuMemoryType::kVertex, indicesToken));

		for(I n = 0; n < drawData.CmdListsCount; ++n)
		{
			const ImDrawList& cmdList = *drawData.CmdLists[n];
			memcpy(verts, cmdList.VtxBuffer.Data, cmdList.VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(indices, cmdList.IdxBuffer.Data, cmdList.IdxBuffer.Size * sizeof(ImDrawIdx));
			verts += cmdList.VtxBuffer.Size;
			indices += cmdList.IdxBuffer.Size;
		}
	}

	cmdb->setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb->setCullMode(FaceSelectionBit::kNone);

	const F32 fbWidth = drawData.DisplaySize.x * drawData.FramebufferScale.x;
	const F32 fbHeight = drawData.DisplaySize.y * drawData.FramebufferScale.y;
	cmdb->setViewport(0, 0, U32(fbWidth), U32(fbHeight));

	cmdb->bindVertexBuffer(0, vertsToken.m_buffer, vertsToken.m_offset, sizeof(ImDrawVert));
	cmdb->setVertexAttribute(0, 0, Format::kR32G32_Sfloat, 0);
	cmdb->setVertexAttribute(1, 0, Format::kR8G8B8A8_Unorm, sizeof(Vec2) * 2);
	cmdb->setVertexAttribute(2, 0, Format::kR32G32_Sfloat, sizeof(Vec2));

	cmdb->bindIndexBuffer(indicesToken.m_buffer, indicesToken.m_offset, IndexType::kU16);

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
				// Project scissor/clipping rectangles into framebuffer space. Flip Y
				Vec4 clipRect;
				clipRect.x() = (pcmd.ClipRect.x - clipOff.x()) * clipScale.x();
				clipRect.y() = (fbHeight - pcmd.ClipRect.w - clipOff.y()) * clipScale.y();
				clipRect.z() = (pcmd.ClipRect.z - clipOff.x()) * clipScale.x();
				clipRect.w() = (fbHeight - pcmd.ClipRect.y - clipOff.y()) * clipScale.y();

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
					cmdb->setScissor(U32(clipRect.x()), U32(clipRect.y()), U32(clipRect.z() - clipRect.x()),
									 U32(clipRect.w() - clipRect.y()));

					UiImageId id(pcmd.TextureId);
					const UiImageIdExtra* idExtra = nullptr;
					TextureViewPtr textureView;
					if(id.m_bits.m_extra)
					{
						idExtra = numberToPtr<const UiImageIdExtra*>(id.m_bits.m_textureViewPtrOrComplex);
						textureView = idExtra->m_textureView;
					}
					else
					{
						textureView.reset(numberToPtr<TextureView*>(id.m_bits.m_textureViewPtrOrComplex));
					}

					// Bind program
					if(idExtra && idExtra->m_customProgram.isCreated())
					{
						cmdb->bindShaderProgram(idExtra->m_customProgram);
					}
					else if(textureView.isCreated())
					{
						cmdb->bindShaderProgram(m_grProgs[kRgbaTex]);
					}
					else
					{
						cmdb->bindShaderProgram(m_grProgs[kNoTex]);
					}

					// Bindings
					if(textureView.isCreated())
					{
						cmdb->bindSampler(0, 0,
										  (id.m_bits.m_pointSampling) ? m_nearestNearestRepeatSampler
																	  : m_linearLinearRepeatSampler);
						cmdb->bindTexture(0, 1, textureView);
					}

					// Push constants
					class PC
					{
					public:
						Vec4 m_transform;
						Array<U8, sizeof(UiImageIdExtra::m_extraPushConstants)> m_extra;
					} pc;
					pc.m_transform.x() = 2.0f / drawData.DisplaySize.x;
					pc.m_transform.y() = -2.0f / drawData.DisplaySize.y;
					pc.m_transform.z() = (drawData.DisplayPos.x / drawData.DisplaySize.x) * 2.0f - 1.0f;
					pc.m_transform.w() = -((drawData.DisplayPos.y / drawData.DisplaySize.y) * 2.0f - 1.0f);
					U32 extraPushConstantsSize = 0;
					if(idExtra && idExtra->m_extraPushConstantsSize)
					{
						ANKI_ASSERT(idExtra->m_extraPushConstantsSize <= sizeof(idExtra->m_extraPushConstants));
						memcpy(&pc.m_extra[0], idExtra->m_extraPushConstants.getBegin(),
							   idExtra->m_extraPushConstantsSize);

						extraPushConstantsSize = idExtra->m_extraPushConstantsSize;
					}
					cmdb->setPushConstants(&pc, sizeof(Vec4) + extraPushConstantsSize);

					// Draw
					cmdb->drawElements(PrimitiveTopology::kTriangles, pcmd.ElemCount, 1, idxOffset, vertOffset);
				}
			}
			idxOffset += pcmd.ElemCount;
		}
		vertOffset += cmdList.VtxBuffer.Size;
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb->setCullMode(FaceSelectionBit::kBack);
}

} // end namespace anki
