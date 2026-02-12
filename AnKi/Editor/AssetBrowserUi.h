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

	void drawWindow(Vec2 initialSize, Vec2 initialPosition, ImGuiWindowFlags windowFlags);

private:
	class AssetFile;
	class AssetDir;

	class ImageCacheEntry
	{
	public:
		ImageResourcePtr m_image;
		U32 m_lastSeenInFrame = 0;
	};

	DynamicArray<AssetDir> m_assetPaths;
	Bool m_refreshAssetsPathsNextTime = true;

	String m_selectedDirPath;
	String m_selectedFilepath;
	Bool m_showRightClickMenuDialog = false;

	DynamicArray<ImageCacheEntry> m_imageCache;

	ImGuiTextFilter m_fileFilter;

	I32 m_cellSize = 8; // Icon size

	ImageResourcePtr m_materialIcon;
	ImageResourcePtr m_meshIcon;

	ParticleEditorUi m_particleEditorWindow;
	MaterialEditorUi m_materialEditorWindow;
	ImageViewerUi m_imageViewerWindow;

	U32 m_newMaterialIndex = 0;

	class
	{
	public:
		const AssetDir* m_selectedDir = nullptr;
		const AssetFile* m_selectedFile = nullptr;
	} m_runCtx;

	void dirTree(const AssetDir& dir); // Like the "tree" command

	void loadImageToCache(CString fname, ImageResourcePtr& img);

	void iconsChild(ConstWeakArray<const AssetFile*> filteredFiles);

	void rightClickMenuDialog();

	void drawMenu();

	static void buildAssetStructure(DynamicArray<AssetDir>& dirs);
	static void sortFilesRecursively(AssetDir& root);

	void setSelected();

	template<typename TFunc, typename TFunc2>
	FunctorContinue visitTree(AssetDir& dir, TFunc dirFunc, TFunc2 fileFunc);
};

} // end namespace anki
