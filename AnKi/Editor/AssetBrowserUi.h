// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Editor/EditorCommon.h>
#include <AnKi/Editor/ParticleEditorUi.h>
#include <AnKi/Editor/MaterialEditorUi.h>
#include <AnKi/Editor/ImageViewerUi.h>

namespace anki {

class AssetBrowserUi : public EditorUiBase
{
public:
	Bool m_open = true;

	AssetBrowserUi();

	~AssetBrowserUi();

	void drawWindow(Vec2 initialPosition, Vec2 initialSize, CString resourceToLocate, ImGuiWindowFlags windowFlags);

private:
	class AssetFile;
	class AssetDir;
	class AssetDirOrFile;

	class ImageCacheEntry
	{
	public:
		ImageResourcePtr m_image;
		U32 m_lastSeenInFrame = 0;
	};

	DynamicArray<AssetDir> m_assetPaths;
	Bool m_refreshAssetsPathsNextTime = true;

	String m_selectedDirPath;
	String m_rightClickSelectedFilepath;
	Bool m_showRightClickMenuDialog = false;

	// "Locate a resource" state: when a properties panel requests it (m_resourceToLocate), switch to that file's directory, scroll to it once and
	// briefly highlight it. m_framesUntilScrollToLocatedFile == -1 means idle.
	String m_resourceToLocate;
	I32 m_framesUntilScrollToLocatedFile = -1; // Wait a few frames for the table to build its scroll extent before scrolling
	static constexpr U32 kLocatedFileHighlightFrameCount = 3 * 60;
	U32 m_locatedFileHighlightFramesLeft = 0; // How many frames until we stop highlighting the located file

	DynamicArray<ImageCacheEntry> m_imageCache;

	ImGuiTextFilter m_fileFilter;

	I32 m_cellSize = 8; // Icon size

	ImageResourcePtr m_materialIcon;
	ImageResourcePtr m_meshIcon;

	ParticleEditorUi m_particleEditorWindow;
	MaterialEditorUi m_materialEditorWindow;
	ImageViewerUi m_imageViewerWindow;

	class
	{
	public:
		const AssetDir* m_selectedDir = nullptr;
		const AssetFile* m_rightClickSelectedFile = nullptr;
		const AssetFile* m_fileToLocate = nullptr;
	} m_runCtx;

	void loadImageToCache(CString fname, ImageResourcePtr& img);

	void drawMenu();
	void drawToolbox();
	void drawDirPath();
	void drawIcons(ConstWeakArray<AssetDirOrFile> filteredItems);
	void rightClickMenuDialog();

	static void buildAssetStructure(DynamicArray<AssetDir>& dirs);
	static void sortFilesRecursively(AssetDir& root);
	static void assignParentDirRecursively(AssetDir& root);

	// Look at the m_selectedXXX strings and update the pointers in m_runCtx
	void setSelectedPointers();

	DynamicArray<AssetDirOrFile> gatherFilteredItems();

	template<typename TFunc, typename TFunc2>
	FunctorContinue visitTree(AssetDir& dir, TFunc dirFunc, TFunc2 fileFunc);

	void setResourceToLocate(CString resourceToLocate)
	{
		m_resourceToLocate = resourceToLocate;
		m_rightClickSelectedFilepath.destroy(); // Deselect
		m_showRightClickMenuDialog = false;
		m_framesUntilScrollToLocatedFile = 2;
		m_locatedFileHighlightFramesLeft = kLocatedFileHighlightFrameCount;
	}
};

} // end namespace anki
