// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Editor/ImageViewerUi.h>
#include <AnKi/Editor/ParticleEditorUi.h>
#include <AnKi/Util/Function.h>
#include <filesystem>

namespace anki {

// Forward
class SceneNode;
#define ANKI_DEFINE_SCENE_COMPONENT(class_, weight, sceneNodeCanHaveMany, icon, serializable) class class_##Component;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

// A class that builds the editor UI and manipulates the scene directly.
class EditorUi
{
public:
	Bool m_quit = false;

	Bool m_mouseOverAnyWindow = false; // Mouse is over one of the editor windows.

	EditorUi();

	~EditorUi();

	void draw(UiCanvas& canvas);

private:
	static constexpr F32 kMargin = 4.0f;
	static constexpr F32 kConsoleHeight = 400.0f;
	static constexpr U32 kMaxTextInputLen = 256;

	enum class AssetFileType : U32
	{
		kNone,
		kTexture,
		kMaterial,
		kMesh,
		kLua,
		kParticleEmitter
	};

	class AssetFile
	{
	public:
		String m_basename;
		String m_filename;
		AssetFileType m_type = AssetFileType::kNone;
	};

	class AssetPath
	{
	public:
		String m_dirname;
		DynamicArray<AssetPath> m_children;
		DynamicArray<AssetFile> m_files;
		U32 m_id = 0;
	};

	class ImageCacheEntry
	{
	public:
		ImageResourcePtr m_image;
		U32 m_lastSeenInFrame = 0;
	};

	UiCanvas* m_canvas = nullptr;

	ImFont* m_font = nullptr;
	ImFont* m_monospaceFont = nullptr;
	F32 m_fontSize = 22.0f;

	Bool m_showCVarEditorWindow = false;
	Bool m_showConsoleWindow = true;
	Bool m_showSceneNodePropsWindow = true;
	Bool m_showSceneHierarcyWindow = true;
	Bool m_showAssetsWindow = true;
	Bool m_showDebugRtsWindow = false;

	ImageResourcePtr m_materialIcon;
	ImageResourcePtr m_meshIcon;

	ImageViewerUi m_imageViewer;
	ParticleEditorUi m_particlesEditor;

	ImGuiTextFilter m_tempFilter;

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
		SceneNode* m_selectedNode = nullptr;
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
		ImGuiTextFilter m_logFilter;
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

	class
	{
	public:
		const AssetPath* m_pathSelected = nullptr;
		DynamicArray<AssetPath> m_assetPaths;

		DynamicArray<ImageCacheEntry> m_imageCache;

		ImGuiTextFilter m_fileFilter;

		I32 m_cellSize = 8; // Icon size
	} m_assetsWindow;

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
	void sceneHierarchyWindow();
	void sceneNodePropertiesWindow();
	void cVarsWindow();
	void consoleWindow();
	void assetsWindow();
	void debugRtsWindow();

	void sceneNode(SceneNode& node);
	void scriptComponent(ScriptComponent& comp);
	void materialComponent(MaterialComponent& comp);
	void meshComponent(MeshComponent& comp);
	void skinComponent(SkinComponent& comp);
	void particleEmitterComponent(ParticleEmitter2Component& comp);
	void lightComponent(LightComponent& comp);
	void jointComponent(JointComponent& comp);
	void bodyComponent(BodyComponent& comp);
	void decalComponent(DecalComponent& comp);
	void cameraComponent(CameraComponent& comp);
	void dirTree(const AssetPath& path);

	// Widget/UI utils
	static void filter(ImGuiTextFilter& filter);
	Bool textEditorWindow(CString extraWindowTitle, Bool* pOpen, String& inout) const;
	static void dummyButton(I32 id);
	template<typename TItemArray>
	static void comboWithFilter(CString text, const TItemArray& items, CString selectedItemIn, U32& selectedItemOut, ImGuiTextFilter& filter);

	// Misc
	static void loggerMessageHandler(void* ud, const LoggerMessageInfo& info);
	static void listDir(const std::filesystem::path& rootPath, const std::filesystem::path& parentPath, AssetPath& parent, U32& id);
	static void gatherAssets(DynamicArray<AssetPath>& paths);
	void loadImageToCache(CString fname, ImageResourcePtr& img);
	void objectPicking();
	static DynamicArray<CString> gatherResourceFilenames(CString filenameContains);
};

} // end namespace anki
