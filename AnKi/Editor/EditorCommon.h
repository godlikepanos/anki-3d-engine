// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Ui.h>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

// The base class of all editor UI related classes
class EditorUiBase
{
protected:
	static constexpr U32 kMaxTextInputLen = 256;

	class SceneGraphViewScene
	{
	public:
		const Scene* m_scene = nullptr;

		DynamicArray<CString> m_nodeNames;
		DynamicArray<U32> m_nodeUuids;
		DynamicArray<SceneNode*> m_nodes;
	};

	// This is a view of the current state of the scenegraph in a format that is optimal for the EditorUi to work with
	class SceneGraphView
	{
	public:
		DynamicArray<SceneGraphViewScene> m_scenes;
	};

	// See SceneGraphView
	static SceneGraphView gatherAllSceneNodes();

	// Draw a rudimentary text editor.
	// Returns true if the save button is pressed and the inout text has changed
	static Bool textEditorWindow(CString windowTitle, Bool* pOpen, ImFont* textFont, String& inout);

	// Iterate all resource filenames and gather the filenames
	static DynamicArray<CString> gatherResourceFilenames(CString filenameContains);

	// Draw a filter textbox
	static void drawfilteredText(ImGuiTextFilter& filter);

	// Draws a combo box and adds a filter for search
	// Returns true if something was selected and in that case selectedItemOut has the selected item
	template<typename TItemArray>
	[[nodiscard]] static Bool comboWithFilter(CString text, const TItemArray& items, CString selectedItemIn, U32& selectedItemOut,
											  ImGuiTextFilter& filter)
	{
		Bool somethingSelected = false;

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
					somethingSelected = true;
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if(isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return somethingSelected;
	}
};

} // end namespace anki
