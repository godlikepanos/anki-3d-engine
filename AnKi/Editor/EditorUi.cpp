// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/EditorUi.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

EditorUi::EditorUi()
{
}

EditorUi::~EditorUi()
{
}

void EditorUi::draw(UiCanvas& canvas)
{
	m_canvas = &canvas;

	if(!m_font)
	{
		const Array<CString, 2> fnames = {"EngineAssets/UbuntuRegular.ttf", "EngineAssets/Editor/materialdesignicons-webfont.ttf"};
		m_font = canvas.addFonts(fnames);
	}

	ImGui::PushFont(m_font, m_fontSize);

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

	buildCVarsEditorWindow();

	ImGui::End();
	ImGui::PopFont();

	m_canvas = nullptr;
}

void EditorUi::buildMainMenu()
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu(ICON_MDI_FOLDER_OUTLINE " File"))
		{
			if(ImGui::MenuItem(ICON_MDI_CLOSE_OUTLINE " Quit"))
			{
				m_quit = true;
			}
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu(ICON_MDI_EYE " View"))
		{
			if(ImGui::MenuItem(ICON_MDI_EYE " CVars Editor"))
			{
				m_showCVarEditorWindow = true;
			}
			ImGui::EndMenu();
		}

		// Quit bnt
		{
			const Char* text = ICON_MDI_CLOSE_OUTLINE;
			const Vec2 textSize = ImGui::CalcTextSize(text);

			m_mainMenu.m_quitBtnHovered = false;
			const F32 menuBarWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(menuBarWidth - textSize.x() - ImGui::GetStyle().FramePadding.x * 2.0f - kMargin);

			const Bool changeColor = m_mainMenu.m_quitBtnHovered;
			if(changeColor)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, Vec4(1.0f, 0.0f, 0.0f, 1.0f));
			}

			if(ImGui::Button(text))
			{
				m_quit = true;
			}

			m_mainMenu.m_quitBtnHovered = ImGui::IsItemHovered();

			if(changeColor)
			{
				ImGui::PopStyleColor();
			}
		}

		ImGui::EndMainMenuBar();
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
	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		ImGui::SetNextWindowPos(viewportPos + kMargin, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(Vec2(400.0f, viewportSize.y() - kMargin * 2.0f), ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Scene Hierarchy", nullptr, 0))
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
	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
		const F32 initialWidth = 400.0f;
		ImGui::SetNextWindowPos(Vec2(viewportSize.x() - kMargin - initialWidth, viewportPos.y() + kMargin), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(Vec2(initialWidth, viewportSize.y() - 2.0f * kMargin), ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Scene Node Props", nullptr, 0) && m_sceneHierarchyWindow.m_visibleNode)
	{
		SceneNode& node = *m_sceneHierarchyWindow.m_visibleNode;

		// Name
		Array<Char, 256> name;
		std::strncpy(name.getBegin(), node.getName().getBegin(), name.getSize());
		if(ImGui::InputText("Name", name.getBegin(), name.getSize(), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			node.setName(&name[0]);
		}

		// Local transform
		F32 localOrigin[3] = {node.getLocalOrigin().x(), node.getLocalOrigin().y(), node.getLocalOrigin().z()};
		if(ImGui::DragFloat3(ICON_MDI_ARROW_ALL " Origin", localOrigin, 0.25f, -1000000.0f, 1000000.0f))
		{
			node.setLocalOrigin(Vec3(&localOrigin[0]));
		}

		// Local scale
		F32 localScale[3] = {node.getLocalScale().x(), node.getLocalScale().y(), node.getLocalScale().z()};
		if(ImGui::DragFloat3(ICON_MDI_ARROW_EXPAND_ALL " Scale", localScale, 0.25f, -1000000.0f, 1000000.0f))
		{
			node.setLocalScale(Vec3(&localScale[0]));
		}

		// Local rotation
		const Euler rot(node.getLocalRotation());
		F32 localRotation[3] = {toDegrees(rot.x()), toDegrees(rot.y()), toDegrees(rot.z())};
		if(ImGui::DragFloat3(ICON_MDI_ROTATE_ORBIT " Rotation", localRotation, 0.25f, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			const Euler rot(toRad(localRotation[0]), toRad(localRotation[1]), toRad(localRotation[2]));
			node.setLocalRotation(Mat3(rot));
		}
	}

	ImGui::End();
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

void EditorUi::buildFilter(ImGuiTextFilter& filter)
{
	ImGui::Text(ICON_MDI_MAGNIFY);
	ImGui::SameLine();

	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
	ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
	if(ImGui::InputTextWithHint("##Filter", "incl,-excl", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
	{
		filter.Build();
	}
	ImGui::PopItemFlag();
}

} // end namespace anki
