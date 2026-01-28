// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/EditorUtils.h>
#include <AnKi/Window/Input.h>

namespace anki {

SceneGraphView gatherAllSceneNodes()
{
	SceneGraphView view;

	SceneGraph& scene = SceneGraph::getSingleton();

	view.m_scenes.resize(scene.getSceneCount());

	U32 count = 0;
	SceneGraph::getSingleton().visitScenes([&](Scene& scene) {
		SceneGraphViewScene& sceneView = view.m_scenes[count++];

		sceneView.m_scene = &scene;

		sceneView.m_nodeNames.resize(scene.getSceneNodeCount());
		sceneView.m_nodeUuids.resize(scene.getSceneNodeCount());
		sceneView.m_nodes.resize(scene.getSceneNodeCount());

		U32 count2 = 0;
		scene.visitNodes([&](SceneNode& node) {
			sceneView.m_nodeNames[count2] = node.getName();
			sceneView.m_nodeUuids[count2] = node.getUuid();
			sceneView.m_nodes[count2] = &node;
			++count2;
			return FunctorContinue::kContinue;
		});

		return FunctorContinue::kContinue;
	});

	return view;
}

Bool textEditorWindow(CString windowTitle, Bool* pOpen, ImFont* textFont, String& inout)
{
	Bool save = false;

	const Vec2 windowSize(600.0f, 800.0f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(Vec2(ImGui::GetMainViewport()->WorkSize) / 2.0f - windowSize / 2.0f, ImGuiCond_FirstUseEver);

	const String title = String().sprintf("Text Editor: %s", windowTitle.cstr());
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

		if(textFont)
		{
			ImGui::PushFont(textFont, 0.0f);
		}

		if(ImGui::InputTextMultiline("##", buffer.getBegin(), buffer.getSize(), Vec2(-1.0f),
									 ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackEdit, replaceTabCallback))
		{
			inout = buffer.getBegin();
		}

		if(textFont)
		{
			ImGui::PopFont();
		}
	}
	ImGui::End();

	return save;
}

DynamicArray<CString> gatherResourceFilenames(CString filenameContains)
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

void drawfilteredText(ImGuiTextFilter& filter)
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

} // end namespace anki
