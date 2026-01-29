// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/AssetBrowserUi.h>
#include <AnKi/Resource/ImageResource.h>
#include <filesystem>

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

static void listDir(const std::filesystem::path& rootPath, const std::filesystem::path& parentPath, AssetPath& parent, U32& id)
{
	for(const auto& entry : std::filesystem::directory_iterator(parentPath))
	{
		if(entry.is_directory())
		{
			AssetPath& p = *parent.m_children.emplaceBack();
			const std::filesystem::path rpath = std::filesystem::relative(entry, parentPath);
			p.m_dirname = rpath.string().c_str();
			p.m_id = id++;

			listDir(rootPath, entry, p, id);
		}
		else if(entry.is_regular_file())
		{
			const String extension = entry.path().extension().string().c_str();
			AssetFile file;
			if(extension == ".ankitex")
			{
				file.m_type = AssetFileType::kTexture;
			}
			else if(extension == ".ankimtl")
			{
				file.m_type = AssetFileType::kMaterial;
			}
			else if(extension == ".ankimesh")
			{
				file.m_type = AssetFileType::kMesh;
			}
			else if(extension == ".ankipart")
			{
				file.m_type = AssetFileType::kParticleEmitter;
			}

			if(file.m_type != AssetFileType::kNone)
			{
				String rpath = std::filesystem::relative(entry.path(), rootPath).string().c_str();
				if(rpath.isEmpty())
				{
					// Sometimes it happens with paths that have links, ignore for now
					continue;
				}

				rpath.replaceAll("\\", "/");

				const String basefname = entry.path().filename().string().c_str();

				file.m_basename = basefname;
				file.m_filename = rpath;

				parent.m_files.emplaceBack(file);
			}
		}
	}
};

static void buildAssetStructure(DynamicArray<AssetPath>& paths)
{
	U32 id = 0;
	ResourceFilesystem::getSingleton().iterateAllResourceBasePaths([&](CString pathname) {
		AssetPath& path = *paths.emplaceBack();
		path.m_dirname = pathname;
		path.m_id = id++;

		std::filesystem::path stdpath(pathname.cstr());
		listDir(stdpath, stdpath, path, id);

		return FunctorContinue::kContinue;
	});
}

AssetBrowserUi::AssetBrowserUi()
{
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/Editor/Material.png", m_materialIcon));
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/Editor/Mesh.png", m_meshIcon));
}

AssetBrowserUi::~AssetBrowserUi()
{
}

void AssetBrowserUi::dirTree(const AssetPath& path)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
	treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow
				 | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
	treeFlags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Left arrow support
	treeFlags |= ImGuiTreeNodeFlags_SpanFullWidth; // Span full width for easier mouse reach
	treeFlags |= ImGuiTreeNodeFlags_DrawLinesToNodes; // Always draw hierarchy outlines

	if(m_pathSelected == &path)
	{
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	const Bool hasChildren = path.m_children.getSize();
	if(!hasChildren)
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	}

	ImGui::PushID(path.m_id);
	const Bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s", path.m_dirname.cstr());
	ImGui::PopID();
	ImGui::SetItemTooltip("%s", path.m_dirname.cstr());

	if(ImGui::IsItemFocused())
	{
		m_pathSelected = &path;
	}

	if(nodeOpen)
	{
		for(const AssetPath& p : path.m_children)
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

	if(m_assetPaths.getSize() == 0)
	{
		buildAssetStructure(m_assetPaths);
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
					for(const AssetPath& p : m_assetPaths)
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
			if(m_pathSelected)
			{
				for(const AssetFile& f : m_pathSelected->m_files)
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

				drawfilteredText(m_fileFilter);

				if(ImGui::BeginChild("RightBottom", Vec2(-1.0f, -1.0f), 0))
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
										loadImageToCache(file.m_filename, img);
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
											ANKI_CHECKF(ResourceManager::getSingleton().loadResource(file.m_filename, rsrc));
											m_particleEditorWindow.open(*rsrc);
										}
										ImGui::PopFont();
									}
									ImGui::PopID();

									ImGui::TextWrapped("%s", file.m_basename.cstr());
									ImGui::SetItemTooltip("%s", file.m_filename.cstr());
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
				ImGui::EndChild();
			}
			ImGui::EndChild();
		} // Right side
	}
	ImGui::End();
}

} // end namespace anki
