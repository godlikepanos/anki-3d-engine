// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>

namespace anki {

class SceneNode;
class ScriptComponent;

/// @addtogroup editor
/// @{

/// A class that builds the editor UI and manipulates the scene directly.
class EditorUi
{
public:
	EditorUi();

	~EditorUi();

	Bool m_quit = false;

	void draw(UiCanvas& canvas);

private:
	static constexpr F32 kMargin = 4.0f;
	static constexpr F32 kConsoleHeight = 300.0f;
	static constexpr U32 kMaxTextInputLen = 256;

	UiCanvas* m_canvas = nullptr;

	ImFont* m_font = nullptr;
	ImFont* m_monospaceFont = nullptr;
	F32 m_fontSize = 22.0f;

	Bool m_showCVarEditorWindow = false;
	Bool m_showConsoleWindow = true;
	Bool m_showSceneNodePropsWindow = true;
	Bool m_showSceneHierarcyWindow = true;
	Bool m_showAssetsWindow = true;

	class
	{
	public:
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

	class
	{
	public:
		List<std::pair<LoggerMessageType, String>> m_log;
		Bool m_forceLogScrollDown = true;
		SpinLock m_logMtx;
	} m_consoleWindow;

	class
	{
	public:
		I32 m_selectedSceneComponentType = 0;
		Bool m_uniformScale = false;

		U32 m_scriptComponentThatHasTheTextEditorOpen = 0;
		Bool m_textEditorOpen = false;
		String m_textEditorTxt;

		U32 m_currentSceneNodeUuid = 0;
	} m_sceneNodePropsWindow;

	void buildMainMenu();

	void buildSceneHierarchyWindow();
	void buildSceneNode(SceneNode& node);

	void buildSceneNodePropertiesWindow();
	void buildScriptComponent(ScriptComponent& comp);

	void buildCVarsEditorWindow();
	void buildConsoleWindow();
	void buildAssetsWindow();

	// Utils
	void buildFilter(ImGuiTextFilter& filter);
	Bool buildTextEditorWindow(CString extraWindowTitle, Bool* pOpen, String& inout);

	// Misc
	static void loggerMessageHandler(void* ud, const LoggerMessageInfo& info);
};
/// @}

} // end namespace anki
