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
	EditorUi();

	~EditorUi();

	Bool m_quit = false;

	void draw(UiCanvas& canvas);

private:
	static constexpr F32 kMargin = 4.0f;

	UiCanvas* m_canvas = nullptr;

	ImFont* m_font = nullptr;
	F32 m_fontSize = 22.0f;

	Bool m_showCVarEditorWindow = false;

	class
	{
	public:
		Bool m_quitBtnHovered = false;
	} m_mainMenu;

	class
	{
	public:
		ImGuiTextFilter m_filter;
		SceneNode* m_visibleNode = nullptr;
	} m_sceneHierarchyWindow;

	class
	{
	public:
		ImGuiTextFilter m_filter;
	} m_cvarsEditorWindow;

	void buildMainMenu();

	void buildSceneHierarchyWindow();
	void buildSceneNode(SceneNode& node);

	void buildSceneNodePropertiesWindow();

	void buildCVarsEditorWindow();

	// Utils
	void buildFilter(ImGuiTextFilter& filter);
};
/// @}

} // end namespace anki
