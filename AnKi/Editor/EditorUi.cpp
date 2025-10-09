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

#define ANKI_IMGUI_PUSH_POP(func, ...) \
	ImGui::Push##func(__VA_ARGS__); \
	DeferredPop ANKI_CONCATENATE(pop, __LINE__)([] { \
		ImGui::Pop##func(); \
	})

EditorUi::EditorUi()
{
	Logger::getSingleton().addMessageHandler(this, loggerMessageHandler);
}

EditorUi::~EditorUi()
{
	Logger::getSingleton().removeMessageHandler(this, loggerMessageHandler);
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

	buildMainMenu();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	buildSceneHierarchyWindow();
	buildSceneNodePropertiesWindow();
	buildConsoleWindow();
	buildAssetsWindow();

	buildCVarsEditorWindow();

	ImGui::End();

	ImGui::PopStyleVar();
	ImGui::PopFont();

	m_canvas = nullptr;
}

void EditorUi::buildMainMenu()
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

		if(ImGui::BeginMenu(ICON_MDI_EYE " Windows"))
		{
			if(ImGui::MenuItem(ICON_MDI_EYE " Console"))
			{
				m_showConsoleWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_EYE " SceneNode Props"))
			{
				m_showSceneNodePropsWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_EYE " Scene Hierarchy"))
			{
				m_showSceneHierarcyWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_EYE " Assets"))
			{
				m_showAssetsWindow = true;
			}

			if(ImGui::MenuItem(ICON_MDI_EYE " CVars Editor"))
			{
				m_showCVarEditorWindow = true;
			}

			ImGui::EndMenu();
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

void EditorUi::buildSceneNode(SceneNode& node)
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
			buildSceneNode(*child);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void EditorUi::buildSceneHierarchyWindow()
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

	if(ImGui::Begin("Scene Hierarchy", &m_showSceneHierarcyWindow, 0))
	{
		buildFilter(m_sceneHierarchyWindow.m_filter);

		if(ImGui::BeginChild("##tree", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_None))
		{
			if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
			{
				SceneGraph::getSingleton().visitNodes([this](SceneNode& node) {
					if(!node.getParent() && m_sceneHierarchyWindow.m_filter.PassFilter(node.getName().cstr()))
					{
						buildSceneNode(node);
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

void EditorUi::buildSceneNodePropertiesWindow()
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
		const F32 initialWidth = 400.0f;
		ImGui::SetNextWindowPos(Vec2(viewportSize.x() - initialWidth, viewportPos.y()), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(Vec2(initialWidth, viewportSize.y() - kConsoleHeight), ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("SceneNode Props", &m_showSceneNodePropsWindow, 0) && m_sceneHierarchyWindow.m_visibleNode)
	{
		U32 id = 0;
		SceneNode& node = *m_sceneHierarchyWindow.m_visibleNode;

		if(state.m_currentSceneNodeUuid != node.getUuid())
		{
			// Node changed, reset a few things
			state.m_currentSceneNodeUuid = node.getUuid();
			state.m_textEditorOpen = false;
			state.m_scriptComponentThatHasTheTextEditorOpen = 0;
			state.m_textEditorTxt.destroy();
		}

		auto dummyButton = [&]() {
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
			ImGui::PushID(id++);
			ImGui::Button(ICON_MDI_LOCK);
			ImGui::PopID();
			ImGui::PopStyleVar();
			ImGui::SameLine();
		};

		const F32 labelWidthBase = ImGui::GetFontSize() * 12; // Some amount of width for label, based on font size.
		const F32 labelWidthMax = ImGui::GetContentRegionAvail().x * 0.40f; // ...but always leave some room for framed widgets.
		ANKI_IMGUI_PUSH_POP(ItemWidth, -min(labelWidthBase, labelWidthMax));

		// Name
		{
			dummyButton();

			Array<Char, 256> name;
			std::strncpy(name.getBegin(), node.getName().getBegin(), name.getSize());
			if(ImGui::InputText("Name", name.getBegin(), name.getSize(), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				node.setName(&name[0]);
			}
		}

		// Local transform
		{
			dummyButton();

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
			dummyButton();

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
			ANKI_IMGUI_PUSH_POP(StyleVar, ImGuiStyleVar_SeparatorTextAlign, Vec2(0.5f));

			U32 count = 0;
			node.iterateComponents([&](SceneComponent& comp) {
				ANKI_IMGUI_PUSH_POP(ID, comp.getUuid());
				const F32 alpha = 0.1f;
				ANKI_IMGUI_PUSH_POP(StyleColor, ImGuiCol_ChildBg, (count & 1) ? Vec4(0.0, 0.0f, 1.0f, alpha) : Vec4(1.0, 0.0f, 0.0f, alpha));

				if(ImGui::BeginChild("Child", Vec2(0.0f), ImGuiChildFlags_AutoResizeY))
				{
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

						if(ImGui::Button(ICON_MDI_DELETE))
						{
						}
						ImGui::SetItemTooltip("Delete Component");
						ImGui::SameLine();

						if(ImGui::Button(ICON_MDI_EYE))
						{
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
						buildScriptComponent(static_cast<ScriptComponent&>(comp));
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

void EditorUi::buildScriptComponent(ScriptComponent& comp)
{
	auto& state = m_sceneNodePropsWindow;

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
		if(buildTextEditorWindow(String().sprintf("ScriptComponent %u", comp.getUuid()), &state.m_textEditorOpen, state.m_textEditorTxt))
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

void EditorUi::buildCVarsEditorWindow()
{
	if(!m_showCVarEditorWindow)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const Vec2 initialSize = Vec2(800.0f, m_canvas->getSizef().y() * 0.8f);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Once, Vec2(0.5f));
	}

	if(ImGui::Begin("CVars Editor", &m_showCVarEditorWindow, 0))
	{
		buildFilter(m_cvarsEditorWindow.m_filter);

		if(ImGui::BeginChild("TableInternals", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if(ImGui::BeginTable("Table", 2, 0))
			{
				I32 id = 0;
				CVarSet::getSingleton().iterateCVars([&](CVar& cvar) {
					if(!m_cvarsEditorWindow.m_filter.PassFilter(cvar.getName().cstr()))
					{
						return false;
					}

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(cvar.getName().cstr());

					ImGui::TableNextColumn();

					ImGui::PushID(id++);

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

					return false;
				});

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	ImGui::End();
}

void EditorUi::buildConsoleWindow()
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

	if(ImGui::Begin("Console", &m_showConsoleWindow, 0))
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

void EditorUi::buildAssetsWindow()
{
	if(!m_showAssetsWindow)
	{
		return;
	}

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

	if(ImGui::Begin("Assets", &m_showAssetsWindow, 0))
	{
	}

	ImGui::End();
}

void EditorUi::buildFilter(ImGuiTextFilter& filter)
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

Bool EditorUi::buildTextEditorWindow(CString extraWindowTitle, Bool* pOpen, String& inout)
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

} // end namespace anki
