// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/SceneHierarchyUi.h>

namespace anki {

void SceneHierarchyUi::drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags, Bool focusOnSelectedNode, SceneNode*& selectedNode,
								  Bool& deleteSelectedNode)
{
	deleteSelectedNode = false;

	if(!m_open)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		// Viewport is one frame delay so do that when frame >1
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin(ICON_MDI_CURTAINS " Scene Hierarchy", &m_open, windowFlags))
	{
		// Scene selector
		{
			const Scene& activeScene = SceneGraph::getSingleton().getActiveScene();
			Scene* newActiveScene = nullptr;

			if(ImGui::BeginCombo("##Scene", activeScene.getName().cstr()))
			{
				SceneGraph::getSingleton().visitScenes([&](Scene& scene) {
					const Bool isSelected = (&scene == &activeScene);
					if(ImGui::Selectable(scene.getName().cstr(), isSelected))
					{
						newActiveScene = &scene;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if(isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}

					return FunctorContinue::kContinue;
				});

				ImGui::EndCombo();
			}

			ImGui::SetItemTooltip("Change active scene");

			if(newActiveScene)
			{
				SceneGraph::getSingleton().setActiveScene(newActiveScene);
			}
		}

		// Delete active scene
		ImGui::SameLine();
		if(ImGui::Button(ICON_MDI_MINUS_BOX))
		{
			SceneGraph::getSingleton().deleteScene(&SceneGraph::getSingleton().getActiveScene());
		}

		// Scene node filter
		drawfilteredText(m_nodeNamesFilter);

		// Do the node tree
		if(ImGui::BeginChild("##tree", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_None))
		{
			if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
			{
				SceneGraph::getSingleton().getActiveScene().visitNodes([&](SceneNode& node) {
					if(!node.getParent() && m_nodeNamesFilter.PassFilter(node.getName().cstr()))
					{
						doSceneNode(focusOnSelectedNode, node, selectedNode, deleteSelectedNode);
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

void SceneHierarchyUi::doSceneNode(Bool focusOnSelectedNode, SceneNode& node, SceneNode*& selectedNode, Bool& deleteSelectedNode)
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

	const Bool selected = &node == selectedNode;
	if(selected)
	{
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if(!node.getChildrenCount())
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	}

	String componentsString;
	for(SceneComponentType sceneComponentType : EnumBitsIterable<SceneComponentType, SceneComponentTypeMask>(node.getSceneComponentMask()))
	{
		switch(sceneComponentType)
		{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	case SceneComponentType::k##name: \
		componentsString += ICON_MDI_##icon; \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
		default:
			ANKI_ASSERT(0);
		}
	}

	// If one if the children of this node is selected and we need to focus on it then open the tree thingy
	if(focusOnSelectedNode)
	{
		node.visitAllChildren([&](SceneNode& child) {
			if(&child == selectedNode)
			{
				ImGui::SetNextItemOpen(true);
				return FunctorContinue::kStop;
			}

			return FunctorContinue::kContinue;
		});
	}

	const Bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s  %s", node.getName().cstr(), componentsString.cstr());

	// Right click popup menu
	{
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
		{
			ImGui::OpenPopup("Scene Node");

			if(!selected)
			{
				selectedNode = &node;
			}
		}

		if(ImGui::BeginPopup("Scene Node"))
		{
			if(ImGui::Button(ICON_MDI_DELETE_FOREVER " Delete"))
			{
				deleteSelectedNode = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	if(ImGui::IsItemClicked())
	{
		selectedNode = &node;
	}

	// Scroll and focus on the selected node
	if(selected && focusOnSelectedNode)
	{
		ImGui::SetScrollHereY();
		ImGui::SetItemDefaultFocus();
	}

	if(nodeOpen)
	{
		for(SceneNode* child : node.getChildren())
		{
			doSceneNode(focusOnSelectedNode, *child, selectedNode, deleteSelectedNode);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();
}

} // end namespace anki
