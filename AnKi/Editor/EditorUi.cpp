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
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Collision.h>
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

// Using some NDC coords find the closest point these coords are to a ray
static F32 projectNdcToRay(Vec2 ndc, Vec3 rayOrigin, Vec3 rayDir)
{
	const Frustum& frustum = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>().getFrustum();
	const Mat4 invMvp = frustum.getViewProjectionMatrix().invert();

	Vec4 v4 = frustum.getViewProjectionMatrix() * rayOrigin.xyz1();
	const Vec2 rayOriginNdc = v4.xy() / v4.w();

	v4 = frustum.getViewProjectionMatrix() * (rayOrigin + rayDir).xyz1();
	const Vec2 rayDirNdc = (v4.xy() / v4.w() - rayOriginNdc).normalize();

	const Vec2 disiredPosNdc = ndc.projectTo(rayOriginNdc, rayDirNdc);
	const Bool positiveSizeOfAxis = (disiredPosNdc - rayOriginNdc).dot(rayDirNdc) > 0.0f;

	// Create 2 lines (0 and 1) and find the closest point between them on line 1
	// Line equation: r(t) = a + t * b

	// Create line 0 which is built from the camera origin and a far point that was unprojected from NDC
	v4 = invMvp * Vec4(disiredPosNdc, 1.0f, 1.0f);
	const Vec3 a0 = v4.xyz() / v4.w();
	const Vec3 b0 = frustum.getWorldTransform().getOrigin().xyz() - a0;

	// Line 1 is built from the ray
	const Vec3 a1 = rayOrigin;
	const Vec3 b1 = rayDir; // = rayDir + rayOrigin - rayOrigin

	// Solve the system
	const Vec3 w = a0 - a1;

	const F32 A = b0.dot(b0);
	const F32 B = b0.dot(b1);
	const F32 C = b1.dot(b1);
	const F32 D = b0.dot(w);
	const F32 E = b1.dot(w);

	const F32 t = (A * E - B * D) / (A * C - B * B);

	const Vec3 touchingPoint = a1 + t * b1;

	const F32 distFromOrign = (touchingPoint - rayOrigin).length() * ((positiveSizeOfAxis) ? 1.0f : -1.0f);

	return distFromOrign;
}

static Bool projectNdcToPlane(Vec2 ndc, Plane plane, Vec3& point)
{
	const Frustum& frustum = SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>().getFrustum();
	const Mat4 invMvp = frustum.getViewProjectionMatrix().invert();

	Vec4 v4 = invMvp * Vec4(ndc, 1.0f, 1.0f);
	v4 /= v4.w();

	const Vec3 rayOrigin = frustum.getWorldTransform().getOrigin().xyz();
	const Vec3 rayDir = (v4.xyz() - rayOrigin).normalize();

	Vec4 collisionPoint;
	const Bool collides = testCollision(plane, Ray(rayOrigin, rayDir), collisionPoint);

	if(collides)
	{
		point = collisionPoint.xyz();
	}

	return collides;
}

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

	objectPicking();

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
		const Vec2 initialSize = Vec2(viewportSize.y() * 0.75f);
		const Vec2 initialPos = (viewportSize - initialSize) / 2.0f;

		m_imageViewer.drawWindow(canvas, initialPos, initialSize, 0);
	}

	{
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 initialSize = Vec2(800.0f, 600.0f);
		const Vec2 initialPos = (viewportSize - initialSize) / 2.0f;

		m_particlesEditor.drawWindow(canvas, initialPos, initialSize, 0);
	}

	ImGui::End();

	ImGui::PopStyleVar();
	ImGui::PopFont();

	// Mouse is over any window if mouse is hovering or if clicked on a window
	m_mouseOverAnyWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::GetIO().WantCaptureMouse;

	m_canvas = nullptr;
}

void EditorUi::mainMenu()
{
	// The menu and toolbox is based on the comments in https://github.com/ocornut/imgui/issues/3518

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	if(ImGui::BeginViewportSideBar("##MainMenu", viewport, ImGuiDir_Up, ImGui::GetFrameHeight(), windowFlags))
	{
		if(ImGui::BeginMenuBar())
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

			if(ImGui::BeginMenu(ICON_MDI_CUBE_SCAN " Debug"))
			{
				Bool bBoundingBoxes = !!(Renderer::getSingleton().getDbg().getOptions() & DbgOption::kBoundingBoxes);
				if(ImGui::Checkbox("Visible Renderables", &bBoundingBoxes))
				{
					DbgOption options = Renderer::getSingleton().getDbg().getOptions();
					if(bBoundingBoxes)
					{
						options |= DbgOption::kBoundingBoxes;
					}
					else
					{
						options &= ~(DbgOption::kBoundingBoxes);
					}
					Renderer::getSingleton().getDbg().setOptions(options);
				}

				Bool bPhysics = !!(Renderer::getSingleton().getDbg().getOptions() & DbgOption::kPhysics);
				if(ImGui::Checkbox("Physics Bodies", &bPhysics))
				{
					DbgOption options = Renderer::getSingleton().getDbg().getOptions();
					if(bPhysics)
					{
						options |= DbgOption::kPhysics;
					}
					else
					{
						options &= ~DbgOption::kPhysics;
					}
					Renderer::getSingleton().getDbg().setOptions(options);
				}

				Bool bDepthTest = !!(Renderer::getSingleton().getDbg().getOptions() & DbgOption::kDepthTest);
				if(ImGui::Checkbox("Depth Test", &bDepthTest))
				{
					DbgOption options = Renderer::getSingleton().getDbg().getOptions();
					if(bDepthTest)
					{
						options |= DbgOption::kDepthTest;
					}
					else
					{
						options &= ~DbgOption::kDepthTest;
					}
					Renderer::getSingleton().getDbg().setOptions(options);
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

			ImGui::EndMenuBar();
		}

		if(Input::getSingleton().getKey(KeyCode::kLeftCtrl) > 0 && Input::getSingleton().getKey(KeyCode::kQ) > 0)
		{
			m_quit = true;
		}
	}
	ImGui::End();

	// Toolbox
	if(ImGui::BeginViewportSideBar("##Toolbox", viewport, ImGuiDir_Up, ImGui::GetFrameHeight(), windowFlags))
	{
		if(ImGui::BeginMenuBar())
		{
			ImGui::SetNextItemWidth(ImGui::CalcTextSize("00.000").x);

			if(ImGui::SliderFloat(ICON_MDI_AXIS_ARROW "&" ICON_MDI_ARROW_EXPAND_ALL " Snapping", &m_toolbox.m_scaleTranslationSnapping, 0.0, 10.0f))
			{
				const F32 roundTo = 0.5f;
				m_toolbox.m_scaleTranslationSnapping = round(m_toolbox.m_scaleTranslationSnapping / roundTo) * roundTo;
			}

			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::CalcTextSize("00.000").x);

			if(ImGui::SliderFloat(ICON_MDI_ROTATE_ORBIT " Snapping", &m_toolbox.m_rotationSnappingDeg, 0.0, 90.0f))
			{
				const F32 roundTo = 1.0f;
				m_toolbox.m_rotationSnappingDeg = round(m_toolbox.m_rotationSnappingDeg / roundTo) * roundTo;
			}

			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
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

	if(&node == m_sceneHierarchyWindow.m_selectedNode)
	{
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if(!node.hasChildren())
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	}

	String componentsString;
	for(SceneComponentType sceneComponentType : EnumBitsIterable<SceneComponentType, SceneComponentTypeMask>(node.getSceneComponentMask()))
	{
		switch(sceneComponentType)
		{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon) \
	case SceneComponentType::k##name: \
		componentsString += ICON_MDI_##icon; \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
		default:
			ANKI_ASSERT(0);
		}
	}

	const Bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s  %s", node.getName().cstr(), componentsString.cstr());

	if(ImGui::IsItemFocused())
	{
		m_sceneHierarchyWindow.m_selectedNode = &node;
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
					return FunctorContinue::kContinue;
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

	if(ImGui::Begin("SceneNode Props", &m_showSceneNodePropsWindow, ImGuiWindowFlags_NoCollapse) && m_sceneHierarchyWindow.m_selectedNode)
	{
		SceneNode& node = *m_sceneHierarchyWindow.m_selectedNode;
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
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon) \
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
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon_) \
	case SceneComponentType::k##name: \
		icon = ANKI_CONCATENATE(ICON_MDI_, icon_); \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
					default:
						ANKI_ASSERT(0);
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
					case SceneComponentType::kParticleEmitter2:
						particleEmitterComponent(static_cast<ParticleEmitter2Component&>(comp));
						break;
					case SceneComponentType::kLight:
						lightComponent(static_cast<LightComponent&>(comp));
						break;
					case SceneComponentType::kBody:
						bodyComponent(static_cast<BodyComponent&>(comp));
						break;
					case SceneComponentType::kJoint:
						jointComponent(static_cast<JointComponent&>(comp));
						break;
					case SceneComponentType::kDecal:
						decalComponent(static_cast<DecalComponent&>(comp));
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
		const DynamicArray<CString> filenames = gatherResourceFilenames(".lua");

		const String currentFilename = (comp.hasScriptResource()) ? comp.getScriptResourceFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < filenames.getSize() && currentFilename != filenames[newSelectedFilename])
		{
			comp.setScriptResourceFilename(filenames[newSelectedFilename]);
		}
	}

	ImGui::Text(" -- or --");

	// Clear button
	{
		if(ImGui::Button(ICON_MDI_DELETE "##ScriptComponentScriptText"))
		{
			comp.setScriptText("");
		}
		ImGui::SetItemTooltip("Unset");
		ImGui::SameLine();
	}

	// Button
	{
		String buttonTxt;
		buttonTxt.sprintf(ICON_MDI_LANGUAGE_LUA " Embedded Script (%s)", comp.hasScriptText() ? "Set" : "Unset");
		const Bool showEditor = ImGui::Button(buttonTxt.cstr(), Vec2(-1.0f, 0.0f));
		if(showEditor && (state.m_scriptComponentThatHasTheTextEditorOpen == 0 || state.m_scriptComponentThatHasTheTextEditorOpen == comp.getUuid()))
		{
			state.m_textEditorOpen = true;
			state.m_scriptComponentThatHasTheTextEditorOpen = comp.getUuid();
			state.m_textEditorTxt =
				(comp.hasScriptText()) ? comp.getScriptText() : "function update(node, prevTime, crntTime)\n    -- Your code here\nend";
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
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Filename
	{
		const DynamicArray<CString> mtlFilenames = gatherResourceFilenames(".ankimtl");

		const String currentFilename = (comp.hasMaterialResource()) ? comp.getMaterialFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames", mtlFilenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < mtlFilenames.getSize() && currentFilename != mtlFilenames[newSelectedFilename])
		{
			comp.setMaterialFilename(mtlFilenames[newSelectedFilename]);
		}
	}

	// Submesh ID
	{
		I32 value = comp.getSubmeshIndex();
		if(ImGui::InputInt(ICON_MDI_VECTOR_POLYGON " Submesh ID", &value, 1, 1, 0))
		{
			comp.setSubmeshIndex(value);
		}
	}
}

void EditorUi::meshComponent(MeshComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Filename
	{
		const DynamicArray<CString> meshFilenames = gatherResourceFilenames(".ankimesh");

		const String currentFilename = (comp.hasMeshResource()) ? comp.getMeshFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames", meshFilenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < meshFilenames.getSize() && currentFilename != meshFilenames[newSelectedFilename])
		{
			comp.setMeshFilename(meshFilenames[newSelectedFilename]);
		}
	}
}

void EditorUi::skinComponent(SkinComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankiskel");

		const String currentFilename = (comp.hasSkeletonResource()) ? comp.getSkeletonFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < filenames.getSize() && currentFilename != filenames[newSelectedFilename])
		{
			comp.setSkeletonFilename(filenames[newSelectedFilename]);
		}
	}
}

void EditorUi::particleEmitterComponent(ParticleEmitter2Component& comp)
{
	// Filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankiparts");

		const String currentFilename = (comp.hasParticleEmitterResource()) ? comp.getParticleEmitterFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < filenames.getSize() && currentFilename != filenames[newSelectedFilename])
		{
			comp.setParticleEmitterFilename(filenames[newSelectedFilename]);
		}
	}

	// Geometry type
	{
		dummyButton(0);

		if(ImGui::BeginCombo("Geometry Type", kParticleEmitterGeometryTypeName[comp.getParticleGeometryType()]))
		{
			for(ParticleGeometryType n : EnumIterable<ParticleGeometryType>())
			{
				const Bool isSelected = (comp.getParticleGeometryType() == n);
				if(ImGui::Selectable(kParticleEmitterGeometryTypeName[n], isSelected))
				{
					comp.setParticleGeometryType(n);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if(isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
}

void EditorUi::lightComponent(LightComponent& comp)
{
	// Light type
	if(ImGui::BeginCombo("Type", kLightComponentTypeNames[comp.getLightComponentType()]))
	{
		for(LightComponentType type : EnumIterable<LightComponentType>())
		{
			const Bool selected = type == comp.getLightComponentType();
			if(ImGui::Selectable(kLightComponentTypeNames[type], selected))
			{
				comp.setLightComponentType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Diffuse color
	Vec4 diffuseCol = comp.getDiffuseColor();
	if(ImGui::InputFloat4("Diffuse Color", &diffuseCol[0]))
	{
		comp.setDiffuseColor(diffuseCol.max(0.01f));
	}

	// Shadow
	Bool shadow = comp.getShadowEnabled();
	if(ImGui::Checkbox("Shadow", &shadow))
	{
		comp.setShadowEnabled(shadow);
	}

	if(comp.getLightComponentType() == LightComponentType::kPoint)
	{
		// Radius
		F32 radius = comp.getRadius();
		if(ImGui::SliderFloat("Radius", &radius, 0.01f, 100.0f))
		{
			comp.setRadius(max(radius, 0.01f));
		}
	}
	else if(comp.getLightComponentType() == LightComponentType::kSpot)
	{
		// Radius
		F32 distance = comp.getDistance();
		if(ImGui::SliderFloat("Distance", &distance, 0.01f, 100.0f))
		{
			comp.setDistance(max(distance, 0.01f));
		}

		// Inner & outter angles
		Vec2 angles(comp.getInnerAngle(), comp.getOuterAngle());
		angles[0] = toDegrees(angles[0]);
		angles[1] = toDegrees(angles[1]);
		if(ImGui::SliderFloat2("Inner & Outer Angles", &angles[0], 1.0f, 89.0f))
		{
			angles[0] = clamp(toRad(angles[0]), 1.0_degrees, 80.0_degrees);
			angles[1] = clamp(toRad(angles[1]), angles[0], 89.0_degrees);

			comp.setInnerAngle(angles[0]);
			comp.setOuterAngle(angles[1]);
		}
	}
	else
	{
		ANKI_ASSERT(comp.getLightComponentType() == LightComponentType::kDirectional);

		// Day of month
		I32 month, day;
		F32 hour;
		comp.getTimeOfDay(month, day, hour);
		Bool fieldChanged = false;
		if(ImGui::SliderInt("Month", &month, 0, 11))
		{
			fieldChanged = true;
		}

		if(ImGui::SliderInt("Day", &day, 0, 30))
		{
			fieldChanged = true;
		}

		if(ImGui::SliderFloat("Hour (0-24)", &hour, 0.0f, 24.0f))
		{
			fieldChanged = true;
		}

		if(fieldChanged)
		{
			comp.setDirectionFromTimeOfDay(month, day, hour);
		}
	}
}

void EditorUi::jointComponent(JointComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Joint type
	if(ImGui::BeginCombo("Type", kJointComponentTypeName[comp.getJointType()]))
	{
		for(JointComponentyType type : EnumIterable<JointComponentyType>())
		{
			const Bool selected = type == comp.getJointType();
			if(ImGui::Selectable(kBodyComponentCollisionShapeTypeNames[type], selected))
			{
				comp.setJointType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void EditorUi::bodyComponent(BodyComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Shape type
	if(ImGui::BeginCombo("Type", kBodyComponentCollisionShapeTypeNames[comp.getCollisionShapeType()]))
	{
		for(BodyComponentCollisionShapeType type : EnumIterable<BodyComponentCollisionShapeType>())
		{
			const Bool selected = type == comp.getCollisionShapeType();
			if(ImGui::Selectable(kBodyComponentCollisionShapeTypeNames[type], selected))
			{
				comp.setCollisionShapeType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Mass
	F32 mass = comp.getMass();
	if(ImGui::SliderFloat("Mass", &mass, 0.0f, 100.0f))
	{
		comp.setMass(mass);
	}

	if(comp.getCollisionShapeType() == BodyComponentCollisionShapeType::kAabb)
	{
		Vec3 extend = comp.getBoxExtend();
		if(ImGui::SliderFloat3("Box Extend", &extend[0], 0.01f, 100.0f))
		{
			comp.setBoxExtend(extend);
		}
	}
	else if(comp.getCollisionShapeType() == BodyComponentCollisionShapeType::kSphere)
	{
		F32 radius = comp.getSphereRadius();
		if(ImGui::SliderFloat("Radius", &radius, 0.01f, 100.0f))
		{
			comp.setSphereRadius(radius);
		}
	}
}

void EditorUi::decalComponent(DecalComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	ImGui::SeparatorText("Diffuse");

	// Diffuse filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankitex");

		const String currentFilename = (comp.hasDiffuseImageResource()) ? comp.getDiffuseImageFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < filenames.getSize() && currentFilename != filenames[newSelectedFilename])
		{
			comp.setDiffuseImageFilename(filenames[newSelectedFilename]);
		}
	}

	// Diffuse factor
	ImGui::SetNextItemWidth(-1.0f);
	F32 diffFactor = comp.getDiffuseBlendFactor();
	if(ImGui::SliderFloat("##Factor0", &diffFactor, 0.0f, 1.0f))
	{
		comp.setDiffuseBlendFactor(diffFactor);
	}
	ImGui::SetItemTooltip("Blend Factor");

	ImGui::SeparatorText("Roughness and Metallic");

	// Roughness/metallic filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankitex");

		const String currentFilename = (comp.hasRoughnessMetalnessImageResource()) ? comp.getRoughnessMetalnessImageFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		comboWithFilter("##Filenames2", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(newSelectedFilename < filenames.getSize() && currentFilename != filenames[newSelectedFilename])
		{
			comp.setRoughnessMetalnessImageFilename(filenames[newSelectedFilename]);
		}
	}

	// Roughness/metallic factor
	ImGui::SetNextItemWidth(-1.0f);
	F32 rmFactor = comp.getRoughnessMetalnessBlendFactor();
	if(ImGui::SliderFloat("##Factor1", &rmFactor, 0.0f, 1.0f))
	{
		comp.setRoughnessMetalnessBlendFactor(rmFactor);
	}
	ImGui::SetItemTooltip("Blend Factor");
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
			// Gather the names
			DynamicArray<String> rtNames;
			Renderer::getSingleton().iterateDebugRenderTargetNames([&](CString name) {
				rtNames.emplaceBack(name);
				return FunctorContinue::kContinue;
			});

			std::sort(rtNames.getBegin(), rtNames.getEnd());

			for(const String& name : rtNames)
			{
				Bool isActive = (name == Renderer::getSingleton().getCurrentDebugRenderTarget());
				if(ImGui::Checkbox(name.cstr(), &isActive) || (isActive && refresh))
				{
					Renderer::getSingleton().setCurrentDebugRenderTarget(isActive ? name : "", m_debugRtsWindow.m_disableTonemapping);
				}
			}
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

		// Clear Log
		{
			if(ImGui::Button(ICON_MDI_DELETE))
			{
				state.m_log.destroy();
			}
			ImGui::SetItemTooltip("Clear log");
			ImGui::SameLine();
		}

		// Search log
		filter(state.m_logFilter);

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
						if(state.m_logFilter.PassFilter(logEntry.second.cstr()))
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
									else if(file.m_type == AssetFileType::kParticleEmitter)
									{
										ImGui::PushFont(nullptr, cellWidth - 1.0f);
										if(ImGui::Button(ICON_MDI_CREATION, Vec2(cellWidth)))
										{
											ParticleEmitterResource2Ptr rsrc;
											ANKI_CHECKF(ResourceManager::getSingleton().loadResource(file.m_filename, rsrc));
											m_particlesEditor.open(*rsrc);
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

template<typename TItemArray>
void EditorUi::comboWithFilter(CString text, const TItemArray& items, CString selectedItemIn, U32& selectedItemOut, ImGuiTextFilter& filter)
{
	if(ImGui::BeginCombo(text.cstr(), selectedItemIn.cstr()))
	{
		if(ImGui::IsWindowAppearing())
		{
			ImGui::SetKeyboardFocusHere();
			filter.Clear();
		}

		ImGui::SetNextItemWidth(-1.0f);
		if(ImGui::InputTextWithHint("##Filter", ICON_MDI_MAGNIFY " Search incl,-excl", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf),
									ImGuiInputTextFlags_EscapeClearsAll))
		{
			filter.Build();
		}

		for(U32 i = 0; i < items.getSize(); ++i)
		{
			CString item = items[i];
			if(!filter.PassFilter(item.cstr()))
			{
				continue;
			}

			const Bool isSelected = (selectedItemIn == item);
			if(ImGui::Selectable(item.cstr(), isSelected))
			{
				selectedItemOut = i;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

Bool EditorUi::textEditorWindow(CString extraWindowTitle, Bool* pOpen, String& inout) const
{
	Bool save = false;

	const Vec2 windowSize(600.0f, 800.0f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(Vec2(ImGui::GetMainViewport()->WorkSize) / 2.0f - windowSize / 2.0f, ImGuiCond_FirstUseEver);

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

void EditorUi::objectPicking()
{
	if(!m_mouseOverAnyWindow && Input::getSingleton().getMouseButton(MouseButton::kLeft) == 1)
	{
		const DbgObjectPickingResult& res = Renderer::getSingleton().getDbg().getObjectPickingResultAtMousePosition();

		if(res.m_sceneNodeUuid != 0)
		{
			SceneGraph::getSingleton().visitNodes([this, uuid = res.m_sceneNodeUuid](SceneNode& node) {
				if(node.getUuid() == uuid)
				{
					m_sceneHierarchyWindow.m_selectedNode = &node;
					ANKI_LOGV("Selecting scene node: %s", node.getName().cstr());
					return FunctorContinue::kStop;
				}
				return FunctorContinue::kContinue;
			});

			m_objectPicking.m_translationAxisSelected = kMaxU32;
			m_objectPicking.m_scaleAxisSelected = kMaxU32;
			m_objectPicking.m_rotationAxisSelected = kMaxU32;
		}
		else if(res.isValid())
		{
			// Clicked a gizmo

			const Transform& nodeTrf = m_sceneHierarchyWindow.m_selectedNode->getLocalTransform();
			const Vec3 nodeOrigin = nodeTrf.getOrigin().xyz();
			Array<Vec3, 3> rotationAxis;
			rotationAxis[0] = nodeTrf.getRotation().getXAxis();
			rotationAxis[1] = nodeTrf.getRotation().getYAxis();
			rotationAxis[2] = nodeTrf.getRotation().getZAxis();

			if(res.m_translationAxis < 3)
			{
				m_objectPicking.m_translationAxisSelected = res.m_translationAxis;
				m_objectPicking.m_scaleAxisSelected = kMaxU32;
				m_objectPicking.m_rotationAxisSelected = kMaxU32;
			}
			else if(res.m_scaleAxis < 3)
			{
				m_objectPicking.m_translationAxisSelected = kMaxU32;
				m_objectPicking.m_scaleAxisSelected = res.m_scaleAxis;
				m_objectPicking.m_rotationAxisSelected = kMaxU32;
			}
			else if(res.m_rotationAxis < 3)
			{
				m_objectPicking.m_translationAxisSelected = kMaxU32;
				m_objectPicking.m_scaleAxisSelected = kMaxU32;
				m_objectPicking.m_rotationAxisSelected = res.m_rotationAxis;

				// Calc the pivot point
				const Transform camTrf =
					SceneGraph::getSingleton().getActiveCameraNode().getFirstComponentOfType<CameraComponent>().getFrustum().getWorldTransform();

				Plane axisPlane;
				axisPlane.setFromRay(nodeOrigin, rotationAxis[res.m_rotationAxis]);

				Bool collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, m_objectPicking.m_pivotPoint);
				if(!collides)
				{
					// Clicked the gizmo from the back side, use the negative plane
					axisPlane.setFromRay(nodeOrigin, -rotationAxis[res.m_rotationAxis]);
					collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, m_objectPicking.m_pivotPoint);
					if(!collides)
					{
						ANKI_LOGW("Can't determin the pivot point");
						m_objectPicking.m_pivotPoint = nodeOrigin;
					}
				}
			}

			if(res.m_translationAxis < 3 || res.m_scaleAxis < 3)
			{
				const U32 axis = (res.m_translationAxis < 3) ? res.m_translationAxis : res.m_scaleAxis;
				m_objectPicking.m_pivotPoint =
					nodeOrigin + rotationAxis[axis] * projectNdcToRay(Input::getSingleton().getMousePositionNdc(), nodeOrigin, rotationAxis[axis]);
			}
		}
		else
		{
			// Clicked on sky
			m_sceneHierarchyWindow.m_selectedNode = nullptr;
			m_objectPicking.m_translationAxisSelected = kMaxU32;
			m_objectPicking.m_scaleAxisSelected = kMaxU32;
			m_objectPicking.m_rotationAxisSelected = kMaxU32;
		}
	}

	if(!m_mouseOverAnyWindow && Input::getSingleton().getMouseButton(MouseButton::kLeft) > 1 && m_sceneHierarchyWindow.m_selectedNode)
	{
		// Possibly dragging

		const Transform& nodeTrf = m_sceneHierarchyWindow.m_selectedNode->getLocalTransform();
		Array<Vec3, 3> rotationAxis;
		rotationAxis[0] = nodeTrf.getRotation().getXAxis();
		rotationAxis[1] = nodeTrf.getRotation().getYAxis();
		rotationAxis[2] = nodeTrf.getRotation().getZAxis();

		if(m_objectPicking.m_translationAxisSelected < 3)
		{
			const U32 axis = m_objectPicking.m_translationAxisSelected;
			const F32 moveDistance = projectNdcToRay(Input::getSingleton().getMousePositionNdc(), m_objectPicking.m_pivotPoint, rotationAxis[axis]);

			const Vec3 oldPosition = nodeTrf.getOrigin().xyz();
			Vec3 newPosition = oldPosition + rotationAxis[axis] * moveDistance;

			// Snap position
			for(U32 i = 0; i < 3 && m_toolbox.m_scaleTranslationSnapping > kEpsilonf; ++i)
			{
				newPosition[i] = round(newPosition[i] / m_toolbox.m_scaleTranslationSnapping) * m_toolbox.m_scaleTranslationSnapping;
			}
			m_sceneHierarchyWindow.m_selectedNode->setLocalOrigin(newPosition);

			// Update the pivot
			const Vec3 moveDiff = newPosition - oldPosition; // Move the pivot as you moved the node origin
			m_objectPicking.m_pivotPoint = m_objectPicking.m_pivotPoint + moveDiff;
		}
		else if(m_objectPicking.m_scaleAxisSelected < 3)
		{
			const U32 axis = m_objectPicking.m_scaleAxisSelected;
			const F32 moveDistance = projectNdcToRay(Input::getSingleton().getMousePositionNdc(), m_objectPicking.m_pivotPoint, rotationAxis[axis]);

			const F32 oldAxisScale = nodeTrf.getScale()[axis];
			F32 newAxisScale = oldAxisScale + moveDistance;

			// Snap scale
			if(m_toolbox.m_scaleTranslationSnapping > kEpsilonf)
			{
				newAxisScale = round(newAxisScale / m_toolbox.m_scaleTranslationSnapping) * m_toolbox.m_scaleTranslationSnapping;
			}

			Vec3 scale = nodeTrf.getScale().xyz();
			scale[axis] = max(newAxisScale, m_toolbox.m_scaleTranslationSnapping);
			m_sceneHierarchyWindow.m_selectedNode->setLocalScale(scale);

			// Update the pivot
			const F32 adjustedMoveDistance = newAxisScale - oldAxisScale;
			m_objectPicking.m_pivotPoint = m_objectPicking.m_pivotPoint + rotationAxis[axis] * adjustedMoveDistance;
		}
		else if(m_objectPicking.m_rotationAxisSelected < 3)
		{
			const U32 axis = m_objectPicking.m_rotationAxisSelected;
			const Vec3 nodeOrigin = nodeTrf.getOrigin().xyz();

			// Compute the new pivot point
			Plane axisPlane;
			axisPlane.setFromRay(nodeOrigin, rotationAxis[axis]);

			Vec3 newPivotPoint;
			Bool collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, newPivotPoint);
			if(!collides)
			{
				// Clicked the gizmo from the back side, use the negative plane
				axisPlane.setFromRay(nodeOrigin, -rotationAxis[axis]);
				collides = projectNdcToPlane(Input::getSingleton().getMousePositionNdc(), axisPlane, newPivotPoint);
				if(!collides)
				{
					ANKI_LOGW("Can't determin the pivot point");
					newPivotPoint = nodeOrigin;
				}
			}

			// Compute the angle between the points
			const Vec3 dir0 = (newPivotPoint - nodeOrigin).normalize();
			const Vec3 dir1 = (m_objectPicking.m_pivotPoint - nodeOrigin).normalize();
			F32 angle = acos(saturate(dir1.dot(dir0)));
			if(m_toolbox.m_rotationSnappingDeg > kEpsilonf)
			{
				const F32 rad = toRad(m_toolbox.m_rotationSnappingDeg);
				angle = round(angle / rad) * rad;
			}

			Vec3 cross = dir1.cross(dir0);

			if(cross != Vec3(0.0f))
			{
				cross = cross.normalize();
				const F32 angleSign = (cross.dot(rotationAxis[axis]) < 1.0f) ? -1.0f : 1.0f;
				angle *= angleSign;

				// Apply the angle
				if(m_objectPicking.m_rotationAxisSelected == 0)
				{
					m_sceneHierarchyWindow.m_selectedNode->rotateLocalX(angle);
				}
				else if(m_objectPicking.m_rotationAxisSelected == 1)
				{
					m_sceneHierarchyWindow.m_selectedNode->rotateLocalY(angle);
				}
				else
				{
					m_sceneHierarchyWindow.m_selectedNode->rotateLocalZ(angle);
				}

				// Use the snapped angle to adjust the pivot point
				Mat3 rot;
				rot.setZAxis(rotationAxis[axis]);
				rot.setXAxis(dir1);
				rot.setYAxis(rotationAxis[axis].cross(dir1).normalize());
				rot.rotateZAxis(angle);
				newPivotPoint = nodeOrigin + rot.getXAxis() * (newPivotPoint - nodeOrigin).length();

				m_objectPicking.m_pivotPoint = newPivotPoint;
			}
		}
	}

	if(m_sceneHierarchyWindow.m_selectedNode)
	{
		Renderer::getSingleton().getDbg().enableGizmos(m_sceneHierarchyWindow.m_selectedNode->getWorldTransform(), true);
	}
	else
	{
		Renderer::getSingleton().getDbg().enableGizmos(Transform::getIdentity(), false);
	}
}

DynamicArray<CString> EditorUi::gatherResourceFilenames(CString filenameContains)
{
	DynamicArray<CString> out;
	ResourceFilesystem::getSingleton().iterateAllFilenames([&](CString fname) {
		if(fname.find(filenameContains) != CString::kNpos)
		{
			out.emplaceBack(fname);
		}

		return FunctorContinue::kContinue;
	});

	return out;
}

} // end namespace anki
