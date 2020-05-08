// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/resource/ShaderProgramResource.h>

namespace anki
{

/// @addtogroup ui
/// @{

/// UI canvas.
class Canvas : public UiObject
{
public:
	Canvas(UiManager* manager);

	virtual ~Canvas();

	ANKI_USE_RESULT Error init(FontPtr font, U32 fontHeight, U32 width, U32 height);

	const FontPtr& getDefaultFont() const
	{
		return m_font;
	}

	/// Resize canvas.
	void resize(U32 width, U32 height)
	{
		ANKI_ASSERT(width > 0 && height > 0);
		m_width = width;
		m_height = height;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	/// @name building commands. The order matters.
	/// @{

	/// Handle input.
	virtual void handleInput();

	/// Begin building the UI.
	void beginBuilding();

	void pushFont(const FontPtr& font, U32 fontHeight);

	void popFont()
	{
		ImGui::PopFont();
	}

	void appendToCommandBuffer(CommandBufferPtr cmdb);
	/// @}

private:
	FontPtr m_font;
	U32 m_dfltFontHeight = 0;
	ImGuiContext* m_imCtx = nullptr;
	U32 m_width;
	U32 m_height;

	enum SHADER_TYPE
	{
		NO_TEX,
		RGBA_TEX,
		SHADER_COUNT
	};

	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, SHADER_COUNT> m_grProgs;
	SamplerPtr m_sampler;

	StackAllocator<U8> m_stackAlloc;

	List<IntrusivePtr<UiObject>> m_references;

	void appendToCommandBufferInternal(CommandBufferPtr& cmdb);
};
/// @}

} // end namespace anki
