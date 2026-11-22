// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Editor/EditorCommon.h>

namespace anki {

class StreamingControlsUi : public EditorUiBase
{
public:
	Bool m_open = false;

	void drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags = 0);
};

} // end namespace anki
