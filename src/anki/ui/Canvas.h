// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_USE_RESULT Error init(FontPtr font);

	nk_context& getContext()
	{
		return m_nkCtx;
	}

	/// Handle input.
	virtual void handleInput();

	/// Begin building the UI.
	void beginBuilding();

	/// End building.
	void endBuilding();

	void appendToCommandBuffer(CommandBufferPtr cmdb);

private:
	FontPtr m_font;
	nk_context m_nkCtx = {};
	nk_buffer m_nkCmdsBuff = {};

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_texGrProg;
	ShaderProgramPtr m_whiteTexGrProg;

#if ANKI_EXTRA_CHECKS
	Bool8 m_building = false;
#endif
};
/// @}

} // end namespace anki
