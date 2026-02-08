// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/EditorUiNode.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

#if ANKI_WITH_EDITOR

EditorUiNode::EditorUiNode(const SceneNodeInitInfo& inf)
	: SceneNode(inf)
{
	UiComponent* uic = newComponent<UiComponent>();
	uic->init(
		[](UiCanvas& canvas, void* ud) {
			static_cast<EditorUiNode*>(ud)->m_editorUi.draw(canvas);
		},
		this);

	uic->setEnabled(g_cvarCoreShowEditor);

	setSerialization(false);
	setUpdateOnPause(true);
}

void EditorUiNode::update(SceneNodeUpdateInfo& info)
{
	getFirstComponentOfType<UiComponent>().setEnabled(g_cvarCoreShowEditor);
	m_editorUi.m_dt = info.m_dt;
}

#endif // #if ANKI_WITH_EDITOR

} // end namespace anki
