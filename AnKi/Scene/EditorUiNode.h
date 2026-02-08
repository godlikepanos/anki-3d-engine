// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Editor/EditorUi.h>
#include <AnKi/Core/App.h>

namespace anki {

#if ANKI_WITH_EDITOR

// Contains the Editor UI. If the UI is visible then it steals the input so otheres need to disable input handling.
class EditorUiNode : public SceneNode
{
public:
	EditorUi m_editorUi;

	EditorUiNode(const SceneNodeInitInfo& inf);

private:
	void update(SceneNodeUpdateInfo& info) override;
};

#endif // #if ANKI_WITH_EDITOR

} // end namespace anki
