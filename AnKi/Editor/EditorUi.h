// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Editor/EditorCommon.h>
#include <AnKi/Editor/SceneNodePropertiesUi.h>
#include <AnKi/Editor/SceneHierarchyUi.h>
#include <AnKi/Editor/AssetBrowserUi.h>
#include <AnKi/Util/Function.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

// Forward
class SceneNode;
#define ANKI_DEFINE_SCENE_COMPONENT(class_, weight, sceneNodeCanHaveMany, icon, serializable) class class_##Component;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

// A class that builds the editor UI and manipulates the scene directly.
class EditorUi : public EditorUiBase
{
public:
	Second m_dt = 0.0f;

	EditorUi();

	~EditorUi();

	void draw(UiCanvas& canvas);

private:
	static constexpr F32 kMargin = 4.0f;
	static constexpr F32 kConsoleHeight = 400.0f;

	UiCanvas* m_canvas = nullptr;

	ImFont* m_font = nullptr;
	ImFont* m_monospaceFont = nullptr;
	F32 m_fontSize = 22.0f;

	Bool m_showCVarEditorWindow = false;
	Bool m_showConsoleWindow = true;
	Bool m_showDebugRtsWindow = false;
	Bool m_quit = false;
	Bool m_mouseOverAnyWindow = false; // Mouse is over one of the editor windows.

	SceneNode* m_selectedNode = nullptr;
	U32 m_selectedNodeUuid = 0;
	U8 m_onNextUpdateFocusOnSelectedNode = 0; // It's a number because we want ask for focus for a few frames due to some deferred ops
	Bool m_showDeleteSceneNodeDialog = false;

	SceneNodePropertiesUi m_sceneNodePropertiesWindow;
	SceneHierarchyUi m_sceneHierarchyWindow;
	AssetBrowserUi m_assetBrowserWindow;

	SceneGraphView m_sceneGraphView;

	U32 m_newNodeNextIndex = 0;
	U32 m_nextNewSceneIndexInName = 0;

	class
	{
	public:
		U32 m_translationAxisSelected = kMaxU32;
		U32 m_scaleAxisSelected = kMaxU32;
		U32 m_rotationAxisSelected = kMaxU32;
		Vec3 m_pivotPoint;
	} m_objectPicking;

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
		ImGuiTextFilter m_logFilter;
	} m_consoleWindow;

	class
	{
	public:
		Bool m_disableTonemapping = false;
	} m_debugRtsWindow;

	class
	{
	public:
		F32 m_scaleTranslationSnapping = 0.1f;
		F32 m_rotationSnappingDeg = 1.0f;
	} m_toolbox;

	void mainMenu();

	// Windows
	void cVarsWindow();
	void consoleWindow();
	void debugRtsWindow();

	// Widget/UI utils
	void deleteSelectedNodeDialog(Bool del); // Dialog. Can't be inside branches

	// Misc
	static void loggerMessageHandler(void* ud, const LoggerMessageInfo& info);
	void objectPicking();
	void handleInput();
	void validateSelectedNode();
	static void saveScenes(Bool onlyActive);
};

} // end namespace anki
