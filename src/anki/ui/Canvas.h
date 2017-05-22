// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>

namespace anki
{

/// @addtogroup ui
/// @{

/// UI canvas.
class Canvas : public UiObject
{
public:
	Canvas(UiManager* manager)
		: UiObject(manager)
	{
	}

	~Canvas();

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

private:
	FontPtr m_font;
	nk_context m_nkCtx = {};
};
/// @}

} // end namespace anki
