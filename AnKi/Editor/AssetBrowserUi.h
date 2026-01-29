// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Editor/EditorUtils.h>
#include <AnKi/Editor/ParticleEditorUi.h>
#include <AnKi/Editor/ImageViewerUi.h>

namespace anki {

class AssetFile;
class AssetPath;

class AssetBrowserUi
{
public:
	Bool m_open = true;

	AssetBrowserUi();

	~AssetBrowserUi();

	void drawWindow(Vec2 initialSize, Vec2 initialPosition, ImGuiWindowFlags windowFlags);

private:
	class ImageCacheEntry
	{
	public:
		ImageResourcePtr m_image;
		U32 m_lastSeenInFrame = 0;
	};

	const AssetPath* m_pathSelected = nullptr;
	DynamicArray<AssetPath> m_assetPaths;

	DynamicArray<ImageCacheEntry> m_imageCache;

	ImGuiTextFilter m_fileFilter;

	I32 m_cellSize = 8; // Icon size

	ImageResourcePtr m_materialIcon;
	ImageResourcePtr m_meshIcon;

	ParticleEditorUi m_particleEditorWindow;
	ImageViewerUi m_imageViewerWindow;

	void dirTree(const AssetPath& path); // Like the "tree" command

	void loadImageToCache(CString fname, ImageResourcePtr& img);
};

} // end namespace anki
