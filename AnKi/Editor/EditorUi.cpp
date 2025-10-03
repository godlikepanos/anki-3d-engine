// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/EditorUi.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

void EditorUi::draw(UiCanvas& canvas)
{
	if(!m_font)
	{
		m_font = canvas.addFont("EngineAssets/UbuntuRegular.ttf");
	}

	ImGui::PushFont(m_font, 20);

	const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.0f;

	ImGui::Begin("MainWindow", nullptr,
				 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
					 | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	ImGui::SetWindowPos(Vec2(0.0f, 0.0f));
	ImGui::SetWindowSize(Vec2(F32(canvas.getWidth()), F32(canvas.getHeight())));

	buildMainMenu(canvas);
	buildSceneHierarchyWindow(canvas);

	ImGui::End();

	ImGui::PopFont();
	m_firstBuild = false;
}

void EditorUi::buildMainMenu(UiCanvas& canvas)
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Assets Browser");
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Quit"))
		{
			m_quit = true;
			ImGui::EndMenu();
		}

		const Vec2 size = ImGui::GetWindowSize();
		m_menuHeight = size.y();

		ImGui::EndMainMenuBar();
	}
}

void EditorUi::buildSceneNode(SceneNode& node)
{
#if 0
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushID(U32(node.getUuid()));
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
		for (SceneNode* child : node.getChildren())
		{
			buildSceneNode(*child);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
#endif
}

void EditorUi::buildSceneHierarchyWindow(UiCanvas& canvas)
{
	if(m_firstBuild)
	{
		ImGui::SetNextWindowPos(Vec2(kMargin, m_menuHeight + kMargin));
		ImGui::SetNextWindowSize(Vec2(F32(canvas.getWidth()) * 25.0f / 100.f, F32(canvas.getHeight()) - m_menuHeight - kMargin * 2.0f));
	}

	ImGui::Begin("Scene Hierarchy", nullptr, 0);

#if 0
	if(ImGui::BeginChild("##tree", Vec2(0.0f), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
	{
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
		ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
		if(ImGui::InputTextWithHint("##Filter", "incl,-excl", m_sceneHierarchyWindow.m_filter.InputBuf,
									IM_ARRAYSIZE(m_sceneHierarchyWindow.m_filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
		{
			m_sceneHierarchyWindow.m_filter.Build();
		}
		ImGui::PopItemFlag();

		if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
		{
			SceneGraph::getSingleton().visitNodes([this](SceneNode& node) {
				if (!node.getParent() && m_sceneHierarchyWindow.m_filter.PassFilter(node.getName().cstr()))
				{
					buildSceneNode(node);
				}
				return true;
			});

			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
#endif

	ImGui::End();
}

} // end namespace anki
