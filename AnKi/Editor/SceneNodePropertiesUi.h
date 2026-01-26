// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Scene/Common.h>
#include <AnKi/Editor/EditorUtils.h>

namespace anki {

// Builds a window that has the properties of a scene node, its components etc etc.
class SceneNodePropertiesUi
{
public:
	Bool m_open = true;

	void drawWindow(SceneNode* node, const SceneGraphView& sceneView, Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags = 0);

private:
	static constexpr U32 kMaxTextInputLen = 256;

	SceneComponentType m_selectedSceneComponentType = SceneComponentType::kFirst;
	Bool m_uniformScale = false;

	U32 m_scriptComponentThatHasTheTextEditorOpen = 0;
	Bool m_textEditorOpen = false;
	String m_textEditorTxt;

	U32 m_currentSceneNodeUuid = 0;

	ImGuiTextFilter m_nodeParentFilter;

	ImFont* m_monospaceFont = nullptr;

	ImGuiTextFilter m_tempFilter;

	void scriptComponent(ScriptComponent& comp);
	void materialComponent(MaterialComponent& comp);
	void meshComponent(MeshComponent& comp);
	void skinComponent(SkinComponent& comp);
	void particleEmitterComponent(ParticleEmitter2Component& comp);
	void lightComponent(LightComponent& comp);
	void jointComponent(JointComponent& comp);
	void bodyComponent(BodyComponent& comp);
	void decalComponent(DecalComponent& comp);
	void cameraComponent(CameraComponent& comp);
	void skyboxComponent(SkyboxComponent& comp);

	static void dummyButton(I32 id)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
		ImGui::PushID(id);
		ImGui::Button(ICON_MDI_LOCK);
		ImGui::PopID();
		ImGui::PopStyleVar();
		ImGui::SameLine();
	};
};

} // end namespace anki
