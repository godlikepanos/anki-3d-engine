// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Scene/Common.h>
#include <AnKi/Editor/EditorUtils.h>

namespace anki {

// Draws the scene hierarchy with the list of all the nodes
class SceneHierarchyUi
{
public:
	Bool m_open = true;

	// focusOnSelectedNode: Scroll to the selectedNode
	// selectedNode: It's inout. There might be some selected node on entering the method and another node when exiting
	void drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags, Bool focusOnSelectedNode, SceneNode*& selectedNode,
					Bool& deleteSelectedNode);

private:
	ImGuiTextFilter m_nodeNamesFilter;

	U32 m_selectedSceneUuid = 0;

	void doSceneNode(Bool focusOnSelectedNode, SceneNode& node, SceneNode*& selectedNode, Bool& deleteSelectedNode);
};

} // end namespace anki
