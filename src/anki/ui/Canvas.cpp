// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Canvas.h>
#include <anki/ui/Font.h>
#include <anki/ui/UiManager.h>
#include <anki/resource/ResourceManager.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/input/Input.h>

namespace anki
{

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
	ANKI_CHECK(m_manager->getResourceManager().loadResource("shaders/Ui.glslp", m_prog));
	const ShaderProgramResourceVariant* variant;

	for(U i = 0; i < SHADER_COUNT; ++i)
	{
		ShaderProgramResourceMutationInitList<1> mutators(m_prog);
		mutators.add("TEXTURE_TYPE", i);
		m_prog->getOrCreateVariant(mutators.get(), variant);
		m_grProgs[i] = variant->getProgram();
	}

	// Sampler
	SamplerInitInfo samplerInit("Canvas");
	samplerInit.m_minMagFilter = SamplingFilter::LINEAR;
	samplerInit.m_mipmapFilter = SamplingFilter::LINEAR;
	samplerInit.m_addressing = SamplingAddressing::REPEAT;
	m_sampler = m_manager->getGrManager().newSampler(samplerInit);

	// Allocator
	m_stackAlloc = StackAllocator<U8>(getAllocator().getMemoryPool().getAllocationCallback(),
		getAllocator().getMemoryPool().getAllocationCallbackUserData(),
		512_B);

	// Create the context
	setImAllocator();
	m_imCtx = ImGui::CreateContext(font->getImFontAtlas());
	ImGui::SetCurrentContext(m_imCtx);
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;
	ImGui::StyleColorsLight();
	ImGui::SetCurrentContext(nullptr);
	unsetImAllocator();

	return Error::NONE;
}

void Canvas::handleInput()
{
	const Input& in = m_manager->getInput();

	// Begin
	setImAllocator();
	ImGui::SetCurrentContext(m_imCtx);
	ImGuiIO& io = ImGui::GetIO();

	// Handle mouse
	Array<U32, 4> viewport = {{0, 0, m_width, m_height}};
	Vec2 mousePosf = in.getMousePosition() / 2.0f + 0.5f;
	mousePosf.y() = 1.0f - mousePosf.y();
	const UVec2 mousePos(mousePosf.x() * viewport[2], mousePosf.y() * viewport[3]);

	io.MousePos.x = mousePos.x();
	io.MousePos.y = mousePos.y();

	io.MouseClicked[0] = in.getMouseButton(MouseButton::LEFT) == 1;
	io.MouseDown[0] = in.getMouseButton(MouseButton::LEFT) > 0;

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
	io.DisplaySize = ImVec2(m_width, m_height);

	ImGui::NewFrame();
	ImGui::PushFont(&m_font->getImFont(m_dfltFontHeight));
}

void Canvas::pushFont(const FontPtr& font, U32 fontHeight)
{
	m_references.pushBack(m_stackAlloc, IntrusivePtr<UiObject>(const_cast<Font*>(font.get())));
	ImGui::PushFont(&font->getImFont(fontHeight));
}

void Canvas::appendToCommandBuffer(CommandBufferPtr cmdb)
{
	appendToCommandBufferInternal(cmdb);

	// Done
	ImGui::SetCurrentContext(nullptr);

	m_references.destroy(m_stackAlloc);
	m_stackAlloc.getMemoryPool().reset();
}

void Canvas::appendToCommandBufferInternal(CommandBufferPtr& cmdb)
{
	ImGui::PopFont();
	ImGui::Render();
	ImDrawData& drawData = *ImGui::GetDrawData();

	// Allocate index and vertex buffers
	StagingGpuMemoryToken vertsToken, indicesToken;
	{
		const U32 verticesSize = drawData.TotalVtxCount * sizeof(ImDrawVert);
		const U32 indicesSize = drawData.TotalIdxCount * sizeof(ImDrawIdx);

		if(verticesSize == 0 || indicesSize == 0)
		{
			return;
		}

		ImDrawVert* verts = static_cast<ImDrawVert*>(m_manager->getStagingGpuMemoryManager().allocateFrame(
			verticesSize, StagingGpuMemoryType::VERTEX, vertsToken));
		ImDrawIdx* indices = static_cast<ImDrawIdx*>(m_manager->getStagingGpuMemoryManager().allocateFrame(
			indicesSize, StagingGpuMemoryType::VERTEX, indicesToken));

		for(I n = 0; n < drawData.CmdListsCount; ++n)
		{
			const ImDrawList& cmdList = *drawData.CmdLists[n];
			memcpy(verts, cmdList.VtxBuffer.Data, cmdList.VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(indices, cmdList.IdxBuffer.Data, cmdList.IdxBuffer.Size * sizeof(ImDrawIdx));
			verts += cmdList.VtxBuffer.Size;
			indices += cmdList.IdxBuffer.Size;
		}
	}

	cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);
	cmdb->setCullMode(FaceSelectionBit::FRONT);

	const U fbWidth = drawData.DisplaySize.x * drawData.FramebufferScale.x;
	const U fbHeight = drawData.DisplaySize.y * drawData.FramebufferScale.y;
	cmdb->setViewport(0, 0, fbWidth, fbHeight);

	cmdb->bindVertexBuffer(0, vertsToken.m_buffer, vertsToken.m_offset, sizeof(ImDrawVert));
	cmdb->setVertexAttribute(0, 0, Format::R32G32_SFLOAT, 0);
	cmdb->setVertexAttribute(1, 0, Format::R8G8B8A8_UNORM, sizeof(Vec2) * 2);
	cmdb->setVertexAttribute(2, 0, Format::R32G32_SFLOAT, sizeof(Vec2));

	cmdb->bindIndexBuffer(indicesToken.m_buffer, indicesToken.m_offset, IndexType::U16);

	cmdb->bindSampler(0, 0, m_sampler);

	// Will project scissor/clipping rectangles into framebuffer space
	const Vec2 clipOff = drawData.DisplayPos; // (0,0) unless using multi-viewports
	const Vec2 clipScale = drawData.FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render
	U vertOffset = 0;
	U idxOffset = 0;
	for(I n = 0; n < drawData.CmdListsCount; n++)
	{
		const ImDrawList& cmdList = *drawData.CmdLists[n];
		for(I i = 0; i < cmdList.CmdBuffer.Size; i++)
		{
			const ImDrawCmd& pcmd = cmdList.CmdBuffer[i];
			if(pcmd.UserCallback)
			{
				// User callback (registered via ImDrawList::AddCallback)
				pcmd.UserCallback(&cmdList, &pcmd);
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
					cmdb->setScissor(
						clipRect.x(), clipRect.y(), clipRect.z() - clipRect.x(), clipRect.w() - clipRect.y());

					if(pcmd.TextureId)
					{
						cmdb->bindShaderProgram(m_grProgs[RGBA_TEX]);

						TextureView* view = static_cast<TextureView*>(pcmd.TextureId);
						cmdb->bindTexture(0, 1, TextureViewPtr(view), TextureUsageBit::SAMPLED_FRAGMENT);
					}
					else
					{
						cmdb->bindShaderProgram(m_grProgs[NO_TEX]);
					}

					// Push constants. TODO: Set them once
					Vec4 transform;
					transform.x() = 2.0f / drawData.DisplaySize.x;
					transform.y() = -2.0f / drawData.DisplaySize.y;
					transform.z() = (drawData.DisplayPos.x / drawData.DisplaySize.x) * 2.0f - 1.0f;
					transform.w() = -((drawData.DisplayPos.y / drawData.DisplaySize.y) * 2.0f - 1.0f);
					cmdb->setPushConstants(&transform, sizeof(transform));

					// Draw
					cmdb->drawElements(PrimitiveTopology::TRIANGLES, pcmd.ElemCount, 1, idxOffset, vertOffset);
				}
			}
			idxOffset += pcmd.ElemCount;
		}
		vertOffset += cmdList.VtxBuffer.Size;
	}

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
