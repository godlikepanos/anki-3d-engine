// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>

namespace anki {

class SceneNode;

/// @addtogroup editor
/// @{

/// XXX
class EditorUi
{
public:
	Bool m_quit = false;

	EditorUi()
	{
		if(UiManager::getSingleton().newFont("EngineAssets/UbuntuRegular.ttf", Array<U32, 3>{12, 16, 20}, m_font))
		{
			// Ignore
		}
	}

	void draw(Canvas& canvas);

private:
	FontPtr m_font;

	F32 m_menuHeight = 0.0f;
	Bool m_firstBuild = true;

	static constexpr F32 kMargin = 4.0f;

	class
	{
	public:
		ImGuiTextFilter m_filter;
		SceneNode* m_visibleNode;
	} m_sceneHierarchyWindow;

	void buildMainMenu(Canvas& canvas);

	void buildSceneHierarchyWindow(Canvas& canvas);
	void buildSceneNode(SceneNode& node);
};
/// @}

} // end namespace anki
