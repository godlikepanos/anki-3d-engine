// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/AssetBrowserUi.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

enum class AssetFileType : U32
{
	kNone,
	kTexture,
	kMaterial,
	kMesh,
	kLua,
	kParticleEmitter
};

class AssetBrowserUi::AssetFile
{
public:
	String m_basename;
	String m_fullFilename;
	AssetFileType m_type = AssetFileType::kNone;
};

class AssetBrowserUi::AssetDir
{
public:
	String m_dirname;
	DynamicArray<AssetDir> m_children;
	DynamicArray<AssetFile> m_files;
	U32 m_id = 0;
};

AssetBrowserUi::AssetBrowserUi()
{
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/Editor/Material.png", m_materialIcon));
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/Editor/Mesh.png", m_meshIcon));
}

AssetBrowserUi::~AssetBrowserUi()
{
}

void AssetBrowserUi::buildAssetStructure(DynamicArray<AssetDir>& dirs)
{
	dirs.destroy();

	U32 id = 0;
	ResourceFilesystem::getSingleton().iterateAllDataPaths([&](CString dataPath) {
		// Create root dir for the data path
		AssetDir& dir = *dirs.emplaceBack();
		dir.m_dirname = dataPath;
		dir.m_id = id++;

		ResourceFilesystem::getSingleton().iterateFilenamesInDataPath(dataPath, [&](CString filename) {
			StringList dirNames;
			dirNames.splitString(filename, '/');

			// Don't need the file
			String basename = dirNames.getBack();
			dirNames.popBack();

			// Create the dirs recursively
			AssetDir* crntDir = &dir;
			for(CString dirName : dirNames)
			{
				Bool dirFound = false;
				for(AssetDir& childDir : crntDir->m_children)
				{
					if(childDir.m_dirname == dirName)
					{
						dirFound = true;
						crntDir = &childDir;
						break;
					}
				}

				if(!dirFound)
				{
					AssetDir* newDir = crntDir->m_children.emplaceBack();
					newDir->m_dirname = dirName;
					newDir->m_id = id++;

					crntDir = newDir;
				}
			}

			// Create the file
			String extension;
			getFilepathExtension(basename, extension);
			AssetFileType filetype = AssetFileType::kNone;
			if(extension == "ankitex" || extension == "png")
			{
				filetype = AssetFileType::kTexture;
			}
			else if(extension == "ankimtl")
			{
				filetype = AssetFileType::kMaterial;
			}
			else if(extension == "ankimesh")
			{
				filetype = AssetFileType::kMesh;
			}
			else if(extension == "ankipart")
			{
				filetype = AssetFileType::kParticleEmitter;
			}

			if(filetype != AssetFileType::kNone)
			{
				AssetFile* file = crntDir->m_files.emplaceBack();
				file->m_fullFilename = String(dataPath) + "/" + filename;
				file->m_basename = basename;
				file->m_type = filetype;
			}

			return FunctorContinue::kContinue;
		});

		return FunctorContinue::kContinue;
	});
}

void AssetBrowserUi::dirTree(const AssetDir& dir)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
	treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow
				 | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
	treeFlags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Left arrow support
	treeFlags |= ImGuiTreeNodeFlags_SpanFullWidth; // Span full width for easier mouse reach
	treeFlags |= ImGuiTreeNodeFlags_DrawLinesToNodes; // Always draw hierarchy outlines

	if(m_selectedPathDirname == dir.m_dirname)
	{
		m_runCtx.m_selectedDir = &dir;
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	const Bool hasChildren = dir.m_children.getSize();
	if(!hasChildren)
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	}

	ImGui::PushID(dir.m_id);
	const Bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s", dir.m_dirname.cstr());
	ImGui::PopID();
	ImGui::SetItemTooltip("%s", dir.m_dirname.cstr());

	if(ImGui::IsItemFocused())
	{
		m_runCtx.m_selectedDir = &dir;
		m_selectedPathDirname = dir.m_dirname;
	}

	if(nodeOpen)
	{
		for(const AssetDir& p : dir.m_children)
		{
			dirTree(p);
		}

		ImGui::TreePop();
	}
}

void AssetBrowserUi::loadImageToCache(CString fname, ImageResourcePtr& img)
{
	// Try to load first
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource(fname, img));

	// Update the cache
	const U32 crntFrame = ImGui::GetFrameCount();
	Bool entryFound = false;
	DynamicArray<ImageCacheEntry>& cache = m_imageCache;
	for(ImageCacheEntry& entry : cache)
	{
		if(entry.m_image->getUuid() == img->getUuid())
		{
			entry.m_lastSeenInFrame = crntFrame;
			entryFound = true;
			break;
		}
	}

	if(!entryFound)
	{
		cache.emplaceBack(ImageCacheEntry{img, crntFrame});
	}

	// Trym the cache: Try to remove stale entries
	const U32 frameInactivityCount = 60 * 30; // ~30"
	while(true)
	{
		Bool foundStaleEntry = false;
		for(auto it = cache.getBegin(); it != cache.getEnd(); ++it)
		{
			ANKI_ASSERT(crntFrame >= it->m_lastSeenInFrame);
			if(crntFrame - it->m_lastSeenInFrame > frameInactivityCount)
			{
				cache.erase(it);
				foundStaleEntry = true;
				break;
			}
		}

		if(!foundStaleEntry)
		{
			break;
		}
	}
}

void AssetBrowserUi::drawWindow(Vec2 initialSize, Vec2 initialPosition, ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	m_runCtx = {};

	if(m_assetPaths.getSize() == 0 || m_refreshAssetsPathsNextTime)
	{
		ANKI_CHECKF(ResourceFilesystem::getSingleton().refreshAll());
		buildAssetStructure(m_assetPaths);
		m_refreshAssetsPathsNextTime = false;
	}

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 initialSize = Vec2(viewportSize.y * 0.75f);
		const Vec2 initialPos = (viewportSize - initialSize) / 2.0f;

		m_imageViewerWindow.drawWindow(initialPos, initialSize, 0);
	}

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 initialSize = Vec2(800.0f, 600.0f);
		const Vec2 initialPos = (viewportSize - initialSize) / 2.0f;

		m_particleEditorWindow.drawWindow(initialPos, initialSize, 0);
	}

	rightClickMenuDialog();

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initialPosition, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Assets", &m_open, windowFlags))
	{
		// Left side
		{
			if(ImGui::BeginChild("Left", Vec2(300.0f, -1.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
			{
				if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					for(const AssetDir& p : m_assetPaths)
					{
						dirTree(p);
					}

					ImGui::EndTable();
				}
			}
			ImGui::EndChild();
		} // Left side

		ImGui::SameLine();

		// Right side
		{
			// Use the filter to gather the files
			DynamicArray<const AssetFile*> filteredFiles;
			if(m_runCtx.m_selectedDir)
			{
				for(const AssetFile& f : m_runCtx.m_selectedDir->m_files)
				{
					if(m_fileFilter.PassFilter(f.m_basename.cstr()))
					{
						filteredFiles.emplaceBack(&f);
					}
				}
			}

			if(ImGui::BeginChild("Right", Vec2(-1.0f, -1.0f), 0))
			{
				// Increase/decrease icon size
				{
					ImGui::TextUnformatted("Icon Size");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(64.0f);
					ImGui::SliderInt("##Icon Size", &m_cellSize, 5, 11, "%d", ImGuiSliderFlags_AlwaysClamp);
					ImGui::SameLine();
				}

				// Refresh tree
				if(ImGui::Button(ICON_MDI_REFRESH))
				{
					m_refreshAssetsPathsNextTime = true;
				}
				ImGui::SameLine();

				// Filter
				drawfilteredText(m_fileFilter);

				// Contents
				if(ImGui::BeginChild("RightBottom", Vec2(-1.0f, -1.0f), 0))
				{
					iconsChild(filteredFiles);
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		} // Right side
	}
	ImGui::End();
}

void AssetBrowserUi::iconsChild(ConstWeakArray<const AssetFile*> filteredFiles)
{
	const F32 cellWidth = F32(m_cellSize) * 16;
	const U32 columnCount = U32(ImGui::GetContentRegionAvail().x / cellWidth);
	ImGui::SetNextItemWidth(-1.0f);
	const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter
								  | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
	if(filteredFiles.getSize() && ImGui::BeginTable("Grid", columnCount, flags))
	{
		const U32 rowCount = (filteredFiles.getSize() + columnCount - 1) / columnCount;

		for(U32 row = 0; row < rowCount; ++row)
		{
			ImGui::TableNextRow();
			for(U32 column = 0; column < columnCount; ++column)
			{
				ImGui::TableNextColumn();

				const U32 idx = row * columnCount + column;
				if(idx < filteredFiles.getSize())
				{
					const AssetFile& file = *filteredFiles[idx];

					ImGui::PushID(idx);
					if(file.m_type == AssetFileType::kMaterial)
					{
						ImTextureID id;
						id.m_texture = &m_materialIcon->getTexture();
						ImGui::ImageButton("##", id, Vec2(cellWidth));
					}
					else if(file.m_type == AssetFileType::kMesh)
					{
						ImTextureID id;
						id.m_texture = &m_meshIcon->getTexture();
						ImGui::ImageButton("##", id, Vec2(cellWidth));
					}
					else if(file.m_type == AssetFileType::kTexture)
					{
						ImageResourcePtr img;
						loadImageToCache(file.m_fullFilename, img);
						ImTextureID id;
						id.m_texture = &img->getTexture();
						id.m_textureSubresource = TextureSubresourceDesc::all();
						if(ImGui::ImageButton("##", id, Vec2(cellWidth)))
						{
							m_imageViewerWindow.m_image = img;
							m_imageViewerWindow.m_open = true;
						}
					}
					else if(file.m_type == AssetFileType::kParticleEmitter)
					{
						ImGui::PushFont(nullptr, cellWidth - 1.0f);
						if(ImGui::Button(ICON_MDI_CREATION, Vec2(cellWidth)))
						{
							ParticleEmitterResource2Ptr rsrc;
							ANKI_CHECKF(ResourceManager::getSingleton().loadResource(file.m_fullFilename, rsrc));
							m_particleEditorWindow.open(*rsrc);
						}
						ImGui::PopFont();
					}
					ImGui::PopID();

					// Right click
					if(ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
					{
						m_selectedFileFilename = file.m_fullFilename;
						m_showRightClockMenuDialog = true;
					}

					if(m_selectedFileFilename == file.m_fullFilename)
					{
						m_runCtx.m_selectedFile = &file;
					}

					ImGui::TextWrapped("%s", file.m_basename.cstr());
					ImGui::SetItemTooltip("%s", file.m_fullFilename.cstr());
				}
			}
		}

		ImGui::EndTable();
	}
	else
	{
		ImGui::TextUnformatted("Empty");
	}
}

void AssetBrowserUi::rightClickMenuDialog()
{
	if(m_showRightClockMenuDialog)
	{
		ImGui::OpenPopup("Right Click");
		m_showRightClockMenuDialog = false;
	}

	if(ImGui::BeginPopup("Right Click"))
	{
		if(ImGui::Button(ICON_MDI_DELETE_FOREVER " Delete"))
		{
			if(removeFile(m_selectedFileFilename))
			{
				ANKI_LOGE("Failed to remove file: %s", m_selectedFileFilename.cstr());
			}

			m_refreshAssetsPathsNextTime = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	else
	{
		// Diselect the file
		m_selectedFileFilename.destroy();
	}
}

} // end namespace anki
