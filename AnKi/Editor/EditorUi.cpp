// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/EditorUi.h>
#include <AnKi/Scene.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Renderer/Renderer.h>
#include <filesystem>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

template<typename TFunc>
class DeferredPop
{
public:
	TFunc m_func;

	DeferredPop(TFunc func)
		: m_func(func)
	{
	}

	~DeferredPop()
	{
		m_func();
	}
};

#define ANKI_PUSH_POP(func, ...) \
	ImGui::Push##func(__VA_ARGS__); \
	DeferredPop ANKI_CONCATENATE(pop, __LINE__)([] { \
		ImGui::Pop##func(); \
	})

/// This pushes a width for text input widgets that leaves "labelSize" chars for the label
#define ANKI_PUSH_POP_TEXT_INPUT_WIDTH(labelSize) \
	const F32 labelWidthBase = ImGui::GetFontSize() * labelSize; /* Some amount of width for label, based on font size. */ \
	const F32 labelWidthMax = ImGui::GetContentRegionAvail().x * 0.40f; /* ...but always leave some room for framed widgets. */ \
	ANKI_PUSH_POP(ItemWidth, -min(labelWidthBase, labelWidthMax))

EditorUi::EditorUi()
{
	Logger::getSingleton().addMessageHandler(this, loggerMessageHandler);

	gatherAssets(m_assetsWindow.m_assetPaths);

	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/Editor/Material.png", m_materialIcon));
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/Editor/Mesh.png", m_meshIcon));
}

EditorUi::~EditorUi()
{
	Logger::getSingleton().removeMessageHandler(this, loggerMessageHandler);
}

void EditorUi::listDir(const std::filesystem::path& rootPath, const std::filesystem::path& parentPath, AssetPath& parent, U32& id)
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

void EditorUi::gatherAssets(DynamicArray<AssetPath>& paths)
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

void EditorUi::loggerMessageHandler(void* ud, const LoggerMessageInfo& info)
{
	EditorUi& self = *static_cast<EditorUi*>(ud);
	String logEntry;
	logEntry.sprintf("[%s][%-4s] %s [%s:%d][%s][%s]\n", kLoggerMessageTypeText[info.m_type], info.m_subsystem ? info.m_subsystem : "N/A", info.m_msg,
					 info.m_file, info.m_line, info.m_func, info.m_threadName);

	LockGuard lock(self.m_consoleWindow.m_logMtx);
	self.m_consoleWindow.m_log.emplaceBack(std::pair(info.m_type, logEntry));
	self.m_consoleWindow.m_forceLogScrollDown = true;
}

void EditorUi::draw(UiCanvas& canvas)
{
	m_canvas = &canvas;

	if(!m_font)
	{
		const Array<CString, 2> fnames = {"EngineAssets/UbuntuRegular.ttf", "EngineAssets/Editor/materialdesignicons-webfont.ttf"};
		m_font = canvas.addFonts(fnames);
	}

	if(!m_monospaceFont)
	{
		m_monospaceFont = canvas.addFont("EngineAssets/UbuntuMonoRegular.ttf");
	}

	ImGui::PushFont(m_font, m_fontSize);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 20.0f);

	const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.0f;

	ImGui::Begin("MainWindow", nullptr,
				 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
					 | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
	ImGui::SetWindowSize(canvas.getSizef());

	mainMenu();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	sceneHierarchyWindow();
	sceneNodePropertiesWindow();
	consoleWindow();
	assetsWindow();
	cVarsWindow();
	debugRtsWindow();

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(viewportSize.y() * 0.75f);
		const Vec2 initialPos = (viewportSize - initialSize) / 2.0f;

		m_imageViewer.drawWindow(canvas, initialPos, initialSize, 0);
	}

	ImGui::End();

	ImGui::PopStyleVar();
	ImGui::PopFont();

	m_mouseHoveredOverAnyWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

	m_canvas = nullptr;
}

void EditorUi::mainMenu()
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu(ICON_MDI_FOLDER_OUTLINE " File"))
		{
			if(ImGui::MenuItem(ICON_MDI_CLOSE_CIRCLE " Quit", "CTRL+Q"))
			{
				m_quit = true;
			}
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu(ICON_MDI_APPLICATION_OUTLINE " Windows"))
		{
			if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Console"))
			{
				m_showConsoleWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " SceneNode Props"))
			{
				m_showSceneNodePropsWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Scene Hierarchy"))
			{
				m_showSceneHierarcyWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Assets"))
			{
				m_showAssetsWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " CVars Editor"))
			{
				m_showCVarEditorWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_APPLICATION_OUTLINE " Debug Render Targets"))
			{
				m_showDebugRtsWindow = true;
			}

			ImGui::EndMenu();
		}

		// Title
		{
			CString text = "AnKi 3D Engine Editor";
			const F32 menuBarWidth = ImGui::GetWindowWidth();
			const F32 textWidth = ImGui::CalcTextSize(text.cstr()).x;
			ImGui::SameLine(menuBarWidth - menuBarWidth / 2.0f - textWidth / 2.0f);

			ImGui::TextUnformatted(text.cstr());
		}

		// Quit bnt
		{
			const Char* text = ICON_MDI_CLOSE_CIRCLE;
			const Vec2 textSize = ImGui::CalcTextSize(text);

			const F32 menuBarWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(menuBarWidth - textSize.x() - ImGui::GetStyle().FramePadding.x * 2.0f - kMargin);

			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4(1.0f, 0.0f, 0.0f, 1.0f));

			if(ImGui::Button(text))
			{
				m_quit = true;
			}

			ImGui::PopStyleColor();
		}

		ImGui::EndMainMenuBar();
	}

	if(Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0 && Input::getSingleton().getKey(KeyCode::kQ) > 0)
	{
		m_quit = true;
	}
}

void EditorUi::sceneNode(SceneNode& node)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushID(I32(node.getUuid()));
	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
	treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow
				 | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
	treeFlags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Left arrow support
	treeFlags |= ImGuiTreeNodeFlags_SpanFullWidth; // Span full width for easier mouse reach
	treeFlags |= ImGuiTreeNodeFlags_DrawLinesToNodes; // Always draw hierarchy outlines

	if(&node == m_sceneHierarchyWindow.m_visibleNode)
	{
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if(!node.hasChildren())
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	}

	const Bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s", node.getName().cstr());

	if(ImGui::IsItemFocused())
	{
		m_sceneHierarchyWindow.m_visibleNode = &node;
	}

	if(nodeOpen)
	{
		for(SceneNode* child : node.getChildren())
		{
			sceneNode(*child);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void EditorUi::sceneHierarchyWindow()
{
	if(!m_showSceneHierarcyWindow)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		ImGui::SetNextWindowPos(viewportPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(Vec2(400.0f, viewportSize.y() - kConsoleHeight), ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Scene Hierarchy", &m_showSceneHierarcyWindow, ImGuiWindowFlags_NoCollapse))
	{
		filter(m_sceneHierarchyWindow.m_filter);

		if(ImGui::BeginChild("##tree", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_None))
		{
			if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
			{
				SceneGraph::getSingleton().visitNodes([this](SceneNode& node) {
					if(!node.getParent() && m_sceneHierarchyWindow.m_filter.PassFilter(node.getName().cstr()))
					{
						sceneNode(node);
					}
					return true;
				});

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void EditorUi::sceneNodePropertiesWindow()
{
	if(!m_showSceneNodePropsWindow)
	{
		return;
	}

	auto& state = m_sceneNodePropsWindow;

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const F32 initialWidth = 500.0f;
		ImGui::SetNextWindowPos(Vec2(viewportSize.x() - initialWidth, viewportPos.y()), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(Vec2(initialWidth, viewportSize.y() - kConsoleHeight), ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("SceneNode Props", &m_showSceneNodePropsWindow, ImGuiWindowFlags_NoCollapse) && m_sceneHierarchyWindow.m_visibleNode)
	{
		SceneNode& node = *m_sceneHierarchyWindow.m_visibleNode;
		I32 id = 0;

		if(state.m_currentSceneNodeUuid != node.getUuid())
		{
			// Node changed, reset a few things
			state.m_currentSceneNodeUuid = node.getUuid();
			state.m_textEditorOpen = false;
			state.m_scriptComponentThatHasTheTextEditorOpen = 0;
			state.m_textEditorTxt.destroy();
		}

		ANKI_PUSH_POP_TEXT_INPUT_WIDTH(12);

		// Name
		{
			dummyButton(id++);

			Array<Char, kMaxTextInputLen> name;
			std::strncpy(name.getBegin(), node.getName().getBegin(), name.getSize());
			if(ImGui::InputText("Name", name.getBegin(), name.getSize(), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				node.setName(&name[0]);
			}
		}

		// Local transform
		{
			dummyButton(id++);

			F32 localOrigin[3] = {node.getLocalOrigin().x(), node.getLocalOrigin().y(), node.getLocalOrigin().z()};
			if(ImGui::DragFloat3(ICON_MDI_AXIS_ARROW " Origin", localOrigin, 0.025f, -1000000.0f, 1000000.0f))
			{
				node.setLocalOrigin(Vec3(&localOrigin[0]));
			}
		}

		// Local scale
		{
			ImGui::PushID(id++);
			if(ImGui::Button(state.m_uniformScale ? ICON_MDI_LOCK : ICON_MDI_LOCK_OPEN))
			{
				state.m_uniformScale = !state.m_uniformScale;
			}
			ImGui::SetItemTooltip("Uniform/Non-uniform scale");
			ImGui::PopID();
			ImGui::SameLine();

			F32 localScale[3] = {node.getLocalScale().x(), node.getLocalScale().y(), node.getLocalScale().z()};
			if(ImGui::DragFloat3(ICON_MDI_ARROW_EXPAND_ALL " Scale", localScale, 0.0025f, 0.01f, 1000000.0f))
			{
				if(!state.m_uniformScale)
				{
					node.setLocalScale(Vec3(&localScale[0]));
				}
				else
				{
					// The component that have changed wins
					F32 scale = scale = localScale[2];
					if(localScale[0] != node.getLocalScale().x())
					{
						scale = localScale[0];
					}
					else if(localScale[1] != node.getLocalScale().y())
					{
						scale = localScale[1];
					}
					node.setLocalScale(Vec3(scale));
				}
			}
		}

		// Local rotation
		{
			dummyButton(id++);

			const Euler rot(node.getLocalRotation());
			F32 localRotation[3] = {toDegrees(rot.x()), toDegrees(rot.y()), toDegrees(rot.z())};
			if(ImGui::DragFloat3(ICON_MDI_ROTATE_ORBIT " Rotation", localRotation, 0.25f, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				const Euler rot(toRad(localRotation[0]), toRad(localRotation[1]), toRad(localRotation[2]));
				node.setLocalRotation(Mat3(rot));
			}

			ImGui::Text(" ");
		}

		// Component controls
		{
			if(ImGui::Button(ICON_MDI_PLUS_BOX))
			{
				switch(SceneComponentType(state.m_selectedSceneComponentType))
				{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon) \
	case SceneComponentType::k##name: \
		node.newComponent<name##Component>(); \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
				default:
					ANKI_ASSERT(0);
				}
			}
			ImGui::SetItemTooltip("Add new component");

			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1.0f);

			I32 n = 0;
			if(ImGui::BeginCombo(" ", kSceneComponentTypeName[state.m_selectedSceneComponentType]))
			{
				for(const Char* name : kSceneComponentTypeName)
				{
					const Bool isSelected = (state.m_selectedSceneComponentType == n);
					if(ImGui::Selectable(name, isSelected))
					{
						state.m_selectedSceneComponentType = n;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if(isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}

					++n;
				}
				ImGui::EndCombo();
			}

			ImGui::Text(" ");
		}

		// Components
		{
			ANKI_PUSH_POP(StyleVar, ImGuiStyleVar_SeparatorTextAlign, Vec2(0.5f));

			U32 count = 0;
			node.iterateComponents([&](SceneComponent& comp) {
				ANKI_PUSH_POP(ID, comp.getUuid());
				const F32 alpha = 0.1f;
				ANKI_PUSH_POP(StyleColor, ImGuiCol_ChildBg, (count & 1) ? Vec4(0.0, 0.0f, 1.0f, alpha) : Vec4(1.0, 0.0f, 0.0f, alpha));

				if(ImGui::BeginChild("Child", Vec2(0.0f), ImGuiChildFlags_AutoResizeY))
				{
					ANKI_PUSH_POP_TEXT_INPUT_WIDTH(12);

					// Find the icon
					CString icon = ICON_MDI_TOY_BRICK;
					switch(comp.getType())
					{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon_) \
	case SceneComponentType::k##name: \
		icon = ANKI_CONCATENATE(ICON_MDI_, icon_); \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
					}

					// Header
					{
						String label;
						label.sprintf(" %s %s (%u)", icon.cstr(), kSceneComponentTypeName[comp.getType()], comp.getUuid());
						ImGui::SeparatorText(label.cstr());

						if(ImGui::Button(ICON_MDI_MINUS_BOX))
						{
							ANKI_LOGW("TODO");
						}
						ImGui::SetItemTooltip("Delete Component");
						ImGui::SameLine();

						if(ImGui::Button(ICON_MDI_EYE))
						{
							ANKI_LOGW("TODO");
						}
						ImGui::SetItemTooltip("Disable component");
					}

					switch(comp.getType())
					{
					case SceneComponentType::kMove:
					case SceneComponentType::kUi:
						// Nothing
						break;
					case SceneComponentType::kScript:
						scriptComponent(static_cast<ScriptComponent&>(comp));
						break;
					case SceneComponentType::kMaterial:
						materialComponent(static_cast<MaterialComponent&>(comp));
						break;
					case SceneComponentType::kMesh:
						meshComponent(static_cast<MeshComponent&>(comp));
						break;
					case SceneComponentType::kSkin:
						skinComponent(static_cast<SkinComponent&>(comp));
						break;
					default:
						ImGui::Text("TODO");
					}
				}
				ImGui::EndChild();
				ImGui::Text(" ");
				++count;
			});
		}
	}

	ImGui::End();
}

void EditorUi::scriptComponent(ScriptComponent& comp)
{
	auto& state = m_sceneNodePropsWindow;

	// Play button
	{
		ImGui::SameLine();
		if(ImGui::Button(ICON_MDI_PLAY "##ScriptComponentResourceFilename"))
		{
			ANKI_LOGV("TODO");
		}
		ImGui::SetItemTooltip("Play script");
	}

	// Clear button
	{
		if(ImGui::Button(ICON_MDI_DELETE "##ScriptComponentResourceFilename"))
		{
			comp.setScriptResourceFilename("");
		}
		ImGui::SetItemTooltip("Clear");
		ImGui::SameLine();
	}

	// Filename input
	{
		Char rsrcTxt[kMaxTextInputLen] = "";
		if(comp.hasScriptResource())
		{
			std::strncpy(rsrcTxt, comp.getScriptResourceFilename().cstr(), sizeof(rsrcTxt));
		}
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##", "Script Filename", rsrcTxt, sizeof(rsrcTxt));
	}

	ImGui::Text(" -- or --");

	// Clear button
	{
		if(ImGui::Button(ICON_MDI_DELETE "##ScriptComponentScriptText"))
		{
			comp.setScriptText("");
		}
		ImGui::SetItemTooltip("Clear");
		ImGui::SameLine();
	}

	// Button
	{
		String buttonTxt;
		buttonTxt.sprintf(ICON_MDI_LANGUAGE_LUA " Embedded Script%s", (comp.hasScriptText()) ? "" : " *Empty*");
		const Bool showEditor = ImGui::Button(buttonTxt.cstr(), Vec2(-1.0f, 0.0f));
		if(showEditor && (state.m_scriptComponentThatHasTheTextEditorOpen == 0 || state.m_scriptComponentThatHasTheTextEditorOpen == comp.getUuid()))
		{
			state.m_textEditorOpen = true;
			state.m_scriptComponentThatHasTheTextEditorOpen = comp.getUuid();
			state.m_textEditorTxt = (comp.hasScriptText()) ? comp.getScriptText() : "";
		}
	}

	if(state.m_textEditorOpen && state.m_scriptComponentThatHasTheTextEditorOpen == comp.getUuid())
	{
		if(textEditorWindow(String().sprintf("ScriptComponent %u", comp.getUuid()), &state.m_textEditorOpen, state.m_textEditorTxt))
		{
			ANKI_LOGV("Updating ScriptComponent");
			comp.setScriptText(state.m_textEditorTxt);
		}

		if(!state.m_textEditorOpen)
		{
			state.m_scriptComponentThatHasTheTextEditorOpen = 0;
			state.m_textEditorTxt.destroy();
		}
	}
}

void EditorUi::materialComponent(MaterialComponent& comp)
{
	// Locate button
	{
		ImGui::BeginDisabled(!comp.hasMaterialResource());
		if(ImGui::Button(comp.hasMaterialResource() ? ICON_MDI_MAP_MARKER : ICON_MDI_ALERT_CIRCLE "##MaterialCompBtn"))
		{
			ANKI_LOGW("TODO");
		}
		ImGui::SetItemTooltip("Locate");
		ImGui::EndDisabled();
		ImGui::SameLine();
	}

	// Filename
	{
		ImGui::SetNextItemWidth(-1.0f);

		Char buff[kMaxTextInputLen] = "";
		if(comp.hasMaterialResource())
		{
			std::strncpy(buff, comp.getMaterialFilename().cstr(), sizeof(buff));
		}

		if(ImGui::InputTextWithHint("##MaterialCompFname", ".ankimtl Filename", buff, sizeof(buff)))
		{
			comp.setMaterialFilename(buff);
		}

		if(comp.hasMaterialResource())
		{
			ImGui::SetItemTooltip("%s", comp.getMaterialFilename().cstr());
		}
	}

	// Submesh ID
	{
		dummyButton(0);

		I32 value = comp.getSubmeshIndex();
		Char txt[100] = "lala";
		if(ImGui::InputInt(ICON_MDI_VECTOR_POLYGON " Submesh ID", &value, 1, 1, 0))
		{
			comp.setSubmeshIndex(value);
		}
	}
}

void EditorUi::meshComponent(MeshComponent& comp)
{
	// Locate button
	{
		ImGui::BeginDisabled(!comp.hasMeshResource());
		if(ImGui::Button(comp.hasMeshResource() ? ICON_MDI_MAP_MARKER : ICON_MDI_ALERT_CIRCLE "##MeshCompBtn"))
		{
			ANKI_LOGW("TODO");
		}
		ImGui::SetItemTooltip("Locate");
		ImGui::EndDisabled();
		ImGui::SameLine();
	}

	// Filename
	{
		ImGui::SetNextItemWidth(-1.0f);

		Char buff[kMaxTextInputLen] = "";
		if(comp.hasMeshResource())
		{
			std::strncpy(buff, comp.getMeshFilename().cstr(), sizeof(buff));
		}

		if(ImGui::InputTextWithHint("##MeshCompFname", ".ankimesh Filename", buff, sizeof(buff)))
		{
			comp.setMeshFilename(buff);
		}

		if(comp.hasMeshResource())
		{
			ImGui::SetItemTooltip("%s", comp.getMeshFilename().cstr());
		}
	}
}

void EditorUi::skinComponent(SkinComponent& comp)
{
	// Locate button
	{
		ImGui::BeginDisabled(!comp.hasSkeletonResource());
		if(ImGui::Button(comp.hasSkeletonResource() ? ICON_MDI_MAP_MARKER : ICON_MDI_ALERT_CIRCLE "##SkinCompBtn"))
		{
			ANKI_LOGW("TODO");
		}
		ImGui::SetItemTooltip("Locate");
		ImGui::EndDisabled();
		ImGui::SameLine();
	}

	// Filename
	{
		ImGui::SetNextItemWidth(-1.0f);

		Char buff[kMaxTextInputLen] = "";
		if(comp.hasSkeletonResource())
		{
			std::strncpy(buff, comp.getSkeletonFilename().cstr(), sizeof(buff));
		}

		if(ImGui::InputTextWithHint("##SkelCompFname", ".ankiskel Filename", buff, sizeof(buff)))
		{
			comp.setSkeletonFilename(buff);
		}

		if(comp.hasSkeletonResource())
		{
			ImGui::SetItemTooltip("%s", comp.getSkeletonFilename().cstr());
		}
	}
}

void EditorUi::cVarsWindow()
{
	if(!m_showCVarEditorWindow)
	{
		return;
	}

	F32 maxCvarTextSize = 0.0f;
	CVarSet::getSingleton().iterateCVars([&](CVar& cvar) {
		maxCvarTextSize = max(maxCvarTextSize, ImGui::CalcTextSize(cvar.getName().cstr()).x);
		return FunctorContinue::kContinue;
	});

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(900.0f, m_canvas->getSizef().y() * 0.8f);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Once, Vec2(0.5f));
	}

	if(ImGui::Begin("CVars Editor", &m_showCVarEditorWindow, 0))
	{
		filter(m_cvarsEditorWindow.m_filter);

		if(ImGui::BeginChild("##Child", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if(ImGui::BeginTable("Table", 2, 0))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, maxCvarTextSize + 20.0f);
				ImGui::TableSetupColumn("Value");

				I32 id = 0;
				CVarSet::getSingleton().iterateCVars([&](CVar& cvar) {
					if(!m_cvarsEditorWindow.m_filter.PassFilter(cvar.getName().cstr()))
					{
						return FunctorContinue::kContinue;
					}

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(cvar.getName().cstr());

					ImGui::TableNextColumn();

					ImGui::PushID(id++);

					ImGui::SetNextItemWidth(-1.0f);

					if(cvar.getValueType() == CVarValueType::kBool)
					{
						BoolCVar& bcvar = static_cast<BoolCVar&>(cvar);
						Bool val = bcvar;
						ImGui::Checkbox("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericF32)
					{
						NumericCVar<F32>& bcvar = static_cast<NumericCVar<F32>&>(cvar);
						F32 val = bcvar;
						ImGui::InputFloat("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericF64)
					{
						NumericCVar<F64>& bcvar = static_cast<NumericCVar<F64>&>(cvar);
						F64 val = bcvar;
						ImGui::InputDouble("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericU8)
					{
						NumericCVar<U8>& bcvar = static_cast<NumericCVar<U8>&>(cvar);
						I32 val = bcvar;
						ImGui::InputInt("", &val);
						bcvar = U8(val);
					}
					else if(cvar.getValueType() == CVarValueType::kNumericU16)
					{
						NumericCVar<U16>& bcvar = static_cast<NumericCVar<U16>&>(cvar);
						I32 val = bcvar;
						ImGui::InputInt("", &val);
						bcvar = U8(val);
					}
					else if(cvar.getValueType() == CVarValueType::kNumericU32)
					{
						NumericCVar<U32>& bcvar = static_cast<NumericCVar<U32>&>(cvar);
						I32 val = bcvar;
						ImGui::InputInt("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kNumericPtrSize)
					{
						NumericCVar<PtrSize>& bcvar = static_cast<NumericCVar<PtrSize>&>(cvar);
						I32 val = I32(bcvar);
						ImGui::InputInt("", &val);
						bcvar = val;
					}
					else if(cvar.getValueType() == CVarValueType::kString)
					{
						StringCVar& bcvar = static_cast<StringCVar&>(cvar);
						CString value = bcvar;
						Char str[256];
						std::strncpy(str, value.cstr(), sizeof(str));
						ImGui::InputText("", str, sizeof(str));
						bcvar = str;
					}
					else
					{
						ANKI_ASSERT(!"TODO");
					}

					ImGui::PopID();

					return FunctorContinue::kContinue;
				});

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	ImGui::End();
}

void EditorUi::debugRtsWindow()
{
	if(!m_showDebugRtsWindow)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(450.0f, m_canvas->getSizef().y() * 0.4f);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Once, Vec2(0.5f));
	}

	if(ImGui::Begin("Debug Render Targets", &m_showDebugRtsWindow, 0))
	{
		const Bool refresh = ImGui::Checkbox("Disable tonemapping", &m_debugRtsWindow.m_disableTonemapping);
		ImGui::TextUnformatted("");
		ImGui::Separator();

		if(ImGui::BeginChild("Content", Vec2(-1.0f, -1.0f)))
		{
			Renderer::getSingleton().iterateDebugRenderTargetNames([&](CString name) {
				Bool isActive = (name == Renderer::getSingleton().getCurrentDebugRenderTarget());
				if(ImGui::Checkbox(name.cstr(), &isActive) || (isActive && refresh))
				{
					Renderer::getSingleton().setCurrentDebugRenderTarget(isActive ? name : "", m_debugRtsWindow.m_disableTonemapping);
				}
				return FunctorContinue::kContinue;
			});
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void EditorUi::consoleWindow()
{
	if(!m_showConsoleWindow)
	{
		return;
	}

	auto& state = m_consoleWindow;

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(viewportSize.x() / 2.0f, kConsoleHeight);
		const Vec2 initialPos = Vec2(0.0f, viewportPos.y() + viewportSize.y() - initialSize.y());
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Console", &m_showConsoleWindow, ImGuiWindowFlags_NoCollapse))
	{
		// Log controls
		{
			if(ImGui::Button(ICON_MDI_DELETE))
			{
				state.m_log.destroy();
			}
			ImGui::SetItemTooltip("Clear log");
			ImGui::SameLine();
		}

		// Lua input
		{
			Char consoleTxt[kMaxTextInputLen] = "";
			ImGui::SetNextItemWidth(-1.0f);
			if(ImGui::InputTextWithHint(" ", ICON_MDI_LANGUAGE_LUA " LUA console", consoleTxt, sizeof(consoleTxt),
										ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
			{
				[[maybe_unused]] const Error err = ScriptManager::getSingleton().evalString(consoleTxt);
				ImGui::SetKeyboardFocusHere(-1); // On enter it loses focus so call this to fix it
			}
		}

		// Log
		{
			if(ImGui::BeginChild("Log", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
			{
				ImGui::PushFont(m_monospaceFont, 0.0f);

				if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					LockGuard lock(state.m_logMtx);

					for(const auto& logEntry : state.m_log)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						constexpr Array<Vec3, U(LoggerMessageType::kCount)> colors = {Vec3(0.074f, 0.631f, 0.054f), Vec3(0.074f, 0.354f, 0.631f),
																					  Vec3(1.0f, 0.0f, 0.0f), Vec3(0.756f, 0.611f, 0.0f),
																					  Vec3(1.0f, 0.0f, 0.0f)};
						ImGui::PushStyleColor(ImGuiCol_Text, colors[logEntry.first].xyz1());
						ImGui::TextUnformatted(logEntry.second.cstr());
						ImGui::PopStyleColor();
					}

					if(state.m_forceLogScrollDown)
					{
						ImGui::SetScrollHereY(1.0f);
						state.m_forceLogScrollDown = false;
					}

					ImGui::EndTable();
				}

				ImGui::PopFont();
			}
			ImGui::EndChild();
		}
	}

	ImGui::End();
}

void EditorUi::dirTree(const AssetPath& path)
{
	auto& state = m_assetsWindow;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
	treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow
				 | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
	treeFlags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Left arrow support
	treeFlags |= ImGuiTreeNodeFlags_SpanFullWidth; // Span full width for easier mouse reach
	treeFlags |= ImGuiTreeNodeFlags_DrawLinesToNodes; // Always draw hierarchy outlines

	if(state.m_pathSelected == &path)
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
		state.m_pathSelected = &path;
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

void EditorUi::assetsWindow()
{
	if(!m_showAssetsWindow)
	{
		return;
	}

	auto& state = m_assetsWindow;

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(viewportSize.x() / 2.0f, kConsoleHeight);
		const Vec2 initialPos = Vec2(viewportSize.x() / 2.0f, viewportPos.y() + viewportSize.y() - initialSize.y());
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Assets", &m_showAssetsWindow, ImGuiWindowFlags_NoCollapse))
	{
		// Left side
		{
			if(ImGui::BeginChild("Left", Vec2(300.0f, -1.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
			{
				if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					for(const AssetPath& p : state.m_assetPaths)
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
			if(state.m_pathSelected)
			{
				for(const AssetFile& f : state.m_pathSelected->m_files)
				{
					if(state.m_fileFilter.PassFilter(f.m_basename.cstr()))
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
					ImGui::SliderInt("##Icon Size", &state.m_cellSize, 5, 11, "%d", ImGuiSliderFlags_AlwaysClamp);
					ImGui::SameLine();
				}

				filter(state.m_fileFilter);

				if(ImGui::BeginChild("RightBottom", Vec2(-1.0f, -1.0f), 0))
				{
					const F32 cellWidth = F32(state.m_cellSize) * 16;
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
											m_imageViewer.m_image = img;
											m_imageViewer.m_open = true;
										}
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

void EditorUi::filter(ImGuiTextFilter& filter)
{
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
	ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
	if(ImGui::InputTextWithHint("##Filter", ICON_MDI_MAGNIFY " Search incl,-excl", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf),
								ImGuiInputTextFlags_EscapeClearsAll))
	{
		filter.Build();
	}
	ImGui::PopItemFlag();
}

void EditorUi::dummyButton(I32 id)
{
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
	ImGui::PushID(id);
	ImGui::Button(ICON_MDI_LOCK);
	ImGui::PopID();
	ImGui::PopStyleVar();
	ImGui::SameLine();
};

Bool EditorUi::textEditorWindow(CString extraWindowTitle, Bool* pOpen, String& inout) const
{
	Bool save = false;

	ImGui::SetNextWindowSize(Vec2(600.0f, 800.0f), ImGuiCond_FirstUseEver);
	const String title = String().sprintf("Text Editor: %s", extraWindowTitle.cstr());
	if(ImGui::Begin(title.cstr(), pOpen))
	{
		if(ImGui::Button(ICON_MDI_CONTENT_SAVE " Save"))
		{
			save = true;
		}
		ImGui::SameLine();
		if(pOpen && ImGui::Button(ICON_MDI_CLOSE " Close"))
		{
			*pOpen = false;
		}

		if(ImGui::IsWindowFocused() && Input::getSingleton().getKey(KeyCode::kEscape) && pOpen)
		{
			*pOpen = false;
		}

		DynamicArray<Char> buffer;
		buffer.resize(max(inout.getLength() + 1, 1024_U32), '\0');

		if(inout.getLength())
		{
			std::strncpy(buffer.getBegin(), inout.cstr(), inout.getLength());
		}

		auto replaceTabCallback = [](ImGuiInputTextCallbackData* data) -> int {
			if(data->CursorPos > 0 && data->Buf[data->CursorPos - 1] == '\t')
			{
				data->DeleteChars(data->CursorPos - 1, 1);
				data->InsertChars(data->CursorPos, "    ");
				return 0;
			}
			else
			{
				return 0;
			}
		};

		ImGui::PushFont(m_monospaceFont, 0.0f);
		if(ImGui::InputTextMultiline("##", buffer.getBegin(), buffer.getSize(), Vec2(-1.0f),
									 ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackEdit, replaceTabCallback))
		{
			inout = buffer.getBegin();
		}
		ImGui::PopFont();
	}
	ImGui::End();

	return save;
}

void EditorUi::loadImageToCache(CString fname, ImageResourcePtr& img)
{
	// Try to load first
	ANKI_CHECKF(ResourceManager::getSingleton().loadResource(fname, img));

	// Update the cache
	const U32 crntFrame = ImGui::GetFrameCount();
	Bool entryFound = false;
	DynamicArray<ImageCacheEntry>& cache = m_assetsWindow.m_imageCache;
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

	// Try to remove stale entries
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

} // end namespace anki
