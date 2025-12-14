// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Editor/EditorUi.h>

namespace anki {

// Contains the Editor UI. If the UI is visible then it steals the input so otheres need to disable input handling.
class EditorUiNode : public SceneNode
{
public:
	EditorUi m_editorUi;

	EditorUiNode(CString name)
		: SceneNode(name)
	{
		UiComponent* uic = newComponent<UiComponent>();
		uic->init(
			[](UiCanvas& canvas, void* ud) {
				static_cast<EditorUiNode*>(ud)->m_editorUi.draw(canvas);
			},
			this);
	}

private:
	void frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime) override;
};

} // end namespace anki
