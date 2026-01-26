// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/SceneNodePropertiesUi.h>
#include <AnKi/Scene.h>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

void SceneNodePropertiesUi::drawWindow(SceneNode* node, const SceneGraphView& sceneGraphView, Vec2 initialPos, Vec2 initialSize,
									   ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("SceneNode Props", &m_open, windowFlags) && node)
	{
		I32 id = 0;

		if(m_currentSceneNodeUuid != node->getUuid())
		{
			// Node changed, reset a few things
			m_currentSceneNodeUuid = node->getUuid();
			m_textEditorOpen = false;
			m_scriptComponentThatHasTheTextEditorOpen = 0;
			m_textEditorTxt.destroy();
		}

		// Name
		{
			Array<Char, kMaxTextInputLen> name;
			std::strncpy(name.getBegin(), node->getName().getBegin(), name.getSize());
			if(ImGui::InputText("Name", name.getBegin(), name.getSize(), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				node->setName(&name[0]);
			}
		}

		// Parent
		{
			const SceneGraphViewScene* sceneView = nullptr;
			for(const SceneGraphViewScene& v : sceneGraphView.m_scenes)
			{
				if(v.m_scene->getSceneUuid() == node->getSceneUuid())
				{
					sceneView = &v;
					break;
				}
			}
			ANKI_ASSERT(sceneView);

			CString parentName;
			if(node->getParent())
			{
				parentName = node->getParent()->getName();
			}
			else
			{
				parentName = "No Parent";
			}

			// ImGui::SetNextItemWidth(-1.0f);
			U32 newSelected = 0;
			const Bool selected = comboWithFilter("Parent", sceneView->m_nodeNames, parentName, newSelected, m_nodeParentFilter);
			if(selected)
			{
				SceneNode* newParent = sceneView->m_nodes[newSelected];
				node->setParent(newParent);
			}

			ImGui::TextUnformatted(" ");
		}

		// Local transform
		{
			dummyButton(id++);

			F32 localOrigin[3] = {node->getLocalOrigin().x, node->getLocalOrigin().y, node->getLocalOrigin().z};
			if(ImGui::DragFloat3(ICON_MDI_AXIS_ARROW " Origin", localOrigin, 0.025f, -1000000.0f, 1000000.0f))
			{
				node->setLocalOrigin(Vec3(&localOrigin[0]));
			}
		}

		// Local scale
		{
			ImGui::PushID(id++);
			if(ImGui::Button(m_uniformScale ? ICON_MDI_LOCK : ICON_MDI_LOCK_OPEN))
			{
				m_uniformScale = !m_uniformScale;
			}
			ImGui::SetItemTooltip("Uniform/Non-uniform scale");
			ImGui::PopID();
			ImGui::SameLine();

			F32 localScale[3] = {node->getLocalScale().x, node->getLocalScale().y, node->getLocalScale().z};
			if(ImGui::DragFloat3(ICON_MDI_ARROW_EXPAND_ALL " Scale", localScale, 0.0025f, 0.01f, 1000000.0f))
			{
				if(!m_uniformScale)
				{
					node->setLocalScale(Vec3(&localScale[0]));
				}
				else
				{
					// The component that have changed wins
					F32 scale = localScale[2];
					if(localScale[0] != node->getLocalScale().x)
					{
						scale = localScale[0];
					}
					else if(localScale[1] != node->getLocalScale().y)
					{
						scale = localScale[1];
					}
					node->setLocalScale(Vec3(scale));
				}
			}
		}

		// Local rotation
		{
			dummyButton(id++);

			const Euler rot(node->getLocalRotation());
			F32 localRotation[3] = {toDegrees(rot.x), toDegrees(rot.y), toDegrees(rot.z)};
			if(ImGui::DragFloat3(ICON_MDI_ROTATE_ORBIT " Rotation", localRotation, 0.25f, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
			{
				const Euler rot(toRad(localRotation[0]), toRad(localRotation[1]), toRad(localRotation[2]));
				node->setLocalRotation(Mat3(rot));
			}

			ImGui::Text(" ");
		}

		// Component creation
		{
			if(ImGui::Button(ICON_MDI_PLUS_BOX))
			{
				switch(m_selectedSceneComponentType)
				{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	case SceneComponentType::k##name: \
		node->newComponent<name##Component>(); \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
				default:
					ANKI_ASSERT(0);
				}
			}
			ImGui::SetItemTooltip("Add new component");

			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1.0f);

			SceneComponentType n = SceneComponentType::kFirst;
			if(ImGui::BeginCombo(" ", kSceneComponentTypeInfos[m_selectedSceneComponentType].m_name))
			{
				for(const SceneComponentTypeInfo& inf : kSceneComponentTypeInfos)
				{
					const Bool isSelected = (m_selectedSceneComponentType == n);
					if(ImGui::Selectable(inf.m_name, isSelected))
					{
						m_selectedSceneComponentType = n;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if(isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}

					++n;
				}
				ImGui::EndCombo();
			}

			ImGui::Text(" ");
		}

		// Components
		{
			ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextAlign, Vec2(0.5f));

			U32 count = 0;
			node->iterateComponents([&](SceneComponent& comp) {
				ImGui::PushID(comp.getUuid());
				const F32 alpha = 0.1f;
				ImGui::PushStyleColor(ImGuiCol_ChildBg, (count & 1) ? Vec4(0.0, 0.0f, 1.0f, alpha) : Vec4(1.0, 0.0f, 0.0f, alpha));

				if(ImGui::BeginChild("Child", Vec2(0.0f), ImGuiChildFlags_AutoResizeY))
				{
					const F32 labelSize = 12.0f;
					const F32 labelWidthBase = ImGui::GetFontSize() * labelSize; // Some amount of width for label, based on font size
					const F32 labelWidthMax = ImGui::GetContentRegionAvail().x * 0.40f; // ...but always leave some room for framed widgets
					ImGui::PushItemWidth(-min(labelWidthBase, labelWidthMax));

					// Find the icon
					CString icon = ICON_MDI_TOY_BRICK;
					switch(comp.getType())
					{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon_, serializable) \
	case SceneComponentType::k##name: \
		icon = ANKI_CONCATENATE(ICON_MDI_, icon_); \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
					default:
						ANKI_ASSERT(0);
					}

					// Header
					{
						String label;
						label.sprintf(" %s %s (%u)", icon.cstr(), kSceneComponentTypeInfos[comp.getType()].m_name, comp.getUuid());
						ImGui::SeparatorText(label.cstr());

						if(ImGui::Button(ICON_MDI_MINUS_BOX))
						{
							ANKI_LOGW("TODO");
						}
						ImGui::SetItemTooltip("Delete Component");
						ImGui::SameLine();

						if(ImGui::Button(ICON_MDI_EYE))
						{
							ANKI_LOGW("TODO");
						}
						ImGui::SetItemTooltip("Disable component");
					}

					switch(comp.getType())
					{
					case SceneComponentType::kMove:
					case SceneComponentType::kUi:
						// Nothing
						break;
					case SceneComponentType::kScript:
						scriptComponent(static_cast<ScriptComponent&>(comp));
						break;
					case SceneComponentType::kMaterial:
						materialComponent(static_cast<MaterialComponent&>(comp));
						break;
					case SceneComponentType::kMesh:
						meshComponent(static_cast<MeshComponent&>(comp));
						break;
					case SceneComponentType::kSkin:
						skinComponent(static_cast<SkinComponent&>(comp));
						break;
					case SceneComponentType::kParticleEmitter2:
						particleEmitterComponent(static_cast<ParticleEmitter2Component&>(comp));
						break;
					case SceneComponentType::kLight:
						lightComponent(static_cast<LightComponent&>(comp));
						break;
					case SceneComponentType::kBody:
						bodyComponent(static_cast<BodyComponent&>(comp));
						break;
					case SceneComponentType::kJoint:
						jointComponent(static_cast<JointComponent&>(comp));
						break;
					case SceneComponentType::kDecal:
						decalComponent(static_cast<DecalComponent&>(comp));
						break;
					case SceneComponentType::kCamera:
						cameraComponent(static_cast<CameraComponent&>(comp));
						break;
					case SceneComponentType::kSkybox:
						skyboxComponent(static_cast<SkyboxComponent&>(comp));
						break;
					default:
						ImGui::Text("TODO");
					}

					ImGui::PopItemWidth();
				}
				ImGui::EndChild();
				ImGui::Text(" ");
				++count;

				ImGui::PopID();
				ImGui::PopStyleColor();
				return FunctorContinue::kContinue;
			});

			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}

void SceneNodePropertiesUi::scriptComponent(ScriptComponent& comp)
{
	// Play button
	{
		ImGui::SameLine();
		if(ImGui::Button(ICON_MDI_PLAY "##ScriptComponentResourceFilename"))
		{
			ANKI_LOGV("TODO");
		}
		ImGui::SetItemTooltip("Play script");
	}

	// Clear button
	{
		if(ImGui::Button(ICON_MDI_DELETE "##ScriptComponentResourceFilename"))
		{
			comp.setScriptResourceFilename("");
		}
		ImGui::SetItemTooltip("Clear");
		ImGui::SameLine();
	}

	// Filename input
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".lua");

		const String currentFilename = (comp.hasScriptResource()) ? comp.getScriptResourceFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != filenames[newSelectedFilename])
		{
			comp.setScriptResourceFilename(filenames[newSelectedFilename]);
		}
	}

	ImGui::Text(" -- or --");

	// Clear button
	{
		if(ImGui::Button(ICON_MDI_DELETE "##ScriptComponentScriptText"))
		{
			comp.setScriptText("");
		}
		ImGui::SetItemTooltip("Unset");
		ImGui::SameLine();
	}

	// Button
	{
		String buttonTxt;
		buttonTxt.sprintf(ICON_MDI_LANGUAGE_LUA " Embedded Script (%s)", comp.hasScriptText() ? "Set" : "Unset");
		const Bool showEditor = ImGui::Button(buttonTxt.cstr(), Vec2(-1.0f, 0.0f));
		if(showEditor && (m_scriptComponentThatHasTheTextEditorOpen == 0 || m_scriptComponentThatHasTheTextEditorOpen == comp.getUuid()))
		{
			m_textEditorOpen = true;
			m_scriptComponentThatHasTheTextEditorOpen = comp.getUuid();
			m_textEditorTxt =
				(comp.hasScriptText()) ? comp.getScriptText() : "function update(info) --info: SceneComponentUpdateInfo\n    -- Your code here\nend";
		}
	}

	if(m_textEditorOpen && m_scriptComponentThatHasTheTextEditorOpen == comp.getUuid())
	{
		if(textEditorWindow(String().sprintf("ScriptComponent %u", comp.getUuid()), &m_textEditorOpen, m_monospaceFont, m_textEditorTxt))
		{
			ANKI_LOGV("Updating ScriptComponent");
			comp.setScriptText(m_textEditorTxt);
		}

		if(!m_textEditorOpen)
		{
			m_scriptComponentThatHasTheTextEditorOpen = 0;
			m_textEditorTxt.destroy();
		}
	}
}

void SceneNodePropertiesUi::materialComponent(MaterialComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Filename
	{
		const DynamicArray<CString> mtlFilenames = gatherResourceFilenames(".ankimtl");

		const String currentFilename = (comp.hasMaterialResource()) ? comp.getMaterialFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames", mtlFilenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != mtlFilenames[newSelectedFilename])
		{
			comp.setMaterialFilename(mtlFilenames[newSelectedFilename]);
		}
	}

	// Submesh ID
	{
		I32 value = comp.getSubmeshIndex();
		if(ImGui::InputInt(ICON_MDI_VECTOR_POLYGON " Submesh ID", &value, 1, 1, 0))
		{
			comp.setSubmeshIndex(value);
		}
	}
}

void SceneNodePropertiesUi::meshComponent(MeshComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Filename
	{
		const DynamicArray<CString> meshFilenames = gatherResourceFilenames(".ankimesh");

		const String currentFilename = (comp.hasMeshResource()) ? comp.getMeshFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames", meshFilenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != meshFilenames[newSelectedFilename])
		{
			comp.setMeshFilename(meshFilenames[newSelectedFilename]);
		}
	}
}

void SceneNodePropertiesUi::skinComponent(SkinComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankiskel");

		const String currentFilename = (comp.hasSkeletonResource()) ? comp.getSkeletonFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != filenames[newSelectedFilename])
		{
			comp.setSkeletonFilename(filenames[newSelectedFilename]);
		}
	}
}

void SceneNodePropertiesUi::particleEmitterComponent(ParticleEmitter2Component& comp)
{
	// Filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankiparts");

		const String currentFilename = (comp.hasParticleEmitterResource()) ? comp.getParticleEmitterFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != filenames[newSelectedFilename])
		{
			comp.setParticleEmitterFilename(filenames[newSelectedFilename]);
		}
	}

	// Geometry type
	{
		if(ImGui::BeginCombo("Geometry Type", kParticleEmitterGeometryTypeName[comp.getParticleGeometryType()]))
		{
			for(ParticleGeometryType n : EnumIterable<ParticleGeometryType>())
			{
				const Bool isSelected = (comp.getParticleGeometryType() == n);
				if(ImGui::Selectable(kParticleEmitterGeometryTypeName[n], isSelected))
				{
					comp.setParticleGeometryType(n);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if(isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
}

void SceneNodePropertiesUi::lightComponent(LightComponent& comp)
{
	// Light type
	if(ImGui::BeginCombo("Type", kLightComponentTypeNames[comp.getLightComponentType()]))
	{
		for(LightComponentType type : EnumIterable<LightComponentType>())
		{
			const Bool selected = type == comp.getLightComponentType();
			if(ImGui::Selectable(kLightComponentTypeNames[type], selected))
			{
				comp.setLightComponentType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Diffuse color
	Vec4 diffuseCol = comp.getDiffuseColor();
	if(ImGui::InputFloat4("Diffuse Color", &diffuseCol[0]))
	{
		comp.setDiffuseColor(diffuseCol.max(0.01f));
	}

	// Shadow
	Bool shadow = comp.getShadowEnabled();
	if(ImGui::Checkbox("Shadow", &shadow))
	{
		comp.setShadowEnabled(shadow);
	}

	if(comp.getLightComponentType() == LightComponentType::kPoint)
	{
		// Radius
		F32 radius = comp.getRadius();
		if(ImGui::SliderFloat("Radius", &radius, 0.01f, 100.0f))
		{
			comp.setRadius(max(radius, 0.01f));
		}
	}
	else if(comp.getLightComponentType() == LightComponentType::kSpot)
	{
		// Radius
		F32 distance = comp.getDistance();
		if(ImGui::SliderFloat("Distance", &distance, 0.01f, 100.0f))
		{
			comp.setDistance(max(distance, 0.01f));
		}

		// Inner & outter angles
		Vec2 angles(comp.getInnerAngle(), comp.getOuterAngle());
		angles[0] = toDegrees(angles[0]);
		angles[1] = toDegrees(angles[1]);
		if(ImGui::SliderFloat2("Inner & Outer Angles", &angles[0], 1.0f, 89.0f))
		{
			angles[0] = clamp(toRad(angles[0]), 1.0_degrees, 80.0_degrees);
			angles[1] = clamp(toRad(angles[1]), angles[0], 89.0_degrees);

			comp.setInnerAngle(angles[0]);
			comp.setOuterAngle(angles[1]);
		}
	}
	else
	{
		ANKI_ASSERT(comp.getLightComponentType() == LightComponentType::kDirectional);

		// Day of month
		I32 month, day;
		F32 hour;
		comp.getTimeOfDay(month, day, hour);
		Bool fieldChanged = false;
		if(ImGui::SliderInt("Month", &month, 0, 11))
		{
			fieldChanged = true;
		}

		if(ImGui::SliderInt("Day", &day, 0, 30))
		{
			fieldChanged = true;
		}

		if(ImGui::SliderFloat("Hour (0-24)", &hour, 0.0f, 24.0f))
		{
			fieldChanged = true;
		}

		if(fieldChanged)
		{
			comp.setDirectionFromTimeOfDay(month, day, hour);
		}
	}
}

void SceneNodePropertiesUi::jointComponent(JointComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Joint type
	if(ImGui::BeginCombo("Type", kJointComponentTypeName[comp.getJointType()]))
	{
		for(JointComponentyType type : EnumIterable<JointComponentyType>())
		{
			const Bool selected = type == comp.getJointType();
			if(ImGui::Selectable(kBodyComponentCollisionShapeTypeNames[type], selected))
			{
				comp.setJointType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void SceneNodePropertiesUi::bodyComponent(BodyComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	// Shape type
	if(ImGui::BeginCombo("Type", kBodyComponentCollisionShapeTypeNames[comp.getCollisionShapeType()]))
	{
		for(BodyComponentCollisionShapeType type : EnumIterable<BodyComponentCollisionShapeType>())
		{
			const Bool selected = type == comp.getCollisionShapeType();
			if(ImGui::Selectable(kBodyComponentCollisionShapeTypeNames[type], selected))
			{
				comp.setCollisionShapeType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Mass
	F32 mass = comp.getMass();
	if(ImGui::SliderFloat("Mass", &mass, 0.0f, 100.0f))
	{
		comp.setMass(mass);
	}

	if(comp.getCollisionShapeType() == BodyComponentCollisionShapeType::kAabb)
	{
		Vec3 extend = comp.getBoxExtend();
		if(ImGui::SliderFloat3("Box Extend", &extend[0], 0.01f, 100.0f))
		{
			comp.setBoxExtend(extend);
		}
	}
	else if(comp.getCollisionShapeType() == BodyComponentCollisionShapeType::kSphere)
	{
		F32 radius = comp.getSphereRadius();
		if(ImGui::SliderFloat("Radius", &radius, 0.01f, 100.0f))
		{
			comp.setSphereRadius(radius);
		}
	}
}

void SceneNodePropertiesUi::decalComponent(DecalComponent& comp)
{
	if(!comp.isValid())
	{
		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_ALERT);
		ImGui::SetItemTooltip("Component not valid");
	}

	ImGui::SeparatorText("Diffuse");

	// Diffuse filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankitex");

		const String currentFilename = (comp.hasDiffuseImageResource()) ? comp.getDiffuseImageFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != filenames[newSelectedFilename])
		{
			comp.setDiffuseImageFilename(filenames[newSelectedFilename]);
		}
	}

	// Diffuse factor
	ImGui::SetNextItemWidth(-1.0f);
	F32 diffFactor = comp.getDiffuseBlendFactor();
	if(ImGui::SliderFloat("##Factor0", &diffFactor, 0.0f, 1.0f))
	{
		comp.setDiffuseBlendFactor(diffFactor);
	}
	ImGui::SetItemTooltip("Blend Factor");

	ImGui::SeparatorText("Roughness and Metallic");

	// Roughness/metallic filename
	{
		const DynamicArray<CString> filenames = gatherResourceFilenames(".ankitex");

		const String currentFilename = (comp.hasRoughnessMetalnessImageResource()) ? comp.getRoughnessMetalnessImageFilename() : "";
		U32 newSelectedFilename = kMaxU32;

		ImGui::SetNextItemWidth(-1.0f);
		const Bool selected = comboWithFilter("##Filenames2", filenames, currentFilename, newSelectedFilename, m_tempFilter);

		if(selected && currentFilename != filenames[newSelectedFilename])
		{
			comp.setRoughnessMetalnessImageFilename(filenames[newSelectedFilename]);
		}
	}

	// Roughness/metallic factor
	ImGui::SetNextItemWidth(-1.0f);
	F32 rmFactor = comp.getRoughnessMetalnessBlendFactor();
	if(ImGui::SliderFloat("##Factor1", &rmFactor, 0.0f, 1.0f))
	{
		comp.setRoughnessMetalnessBlendFactor(rmFactor);
	}
	ImGui::SetItemTooltip("Blend Factor");
}

void SceneNodePropertiesUi::cameraComponent(CameraComponent& comp)
{
	F32 near = comp.getNear();
	if(ImGui::SliderFloat("Near", &near, 0.1f, 10.0f))
	{
		comp.setNear(near);
	}

	F32 far = comp.getFar();
	if(ImGui::SliderFloat("Far", &far, 10.2f, 10000.0f))
	{
		comp.setFar(far);
	}

	F32 fovX = toDegrees(comp.getFovX());
	if(ImGui::SliderFloat("FovX", &fovX, 10.0f, 200.0f))
	{
		comp.setFovX(toRad(fovX));
	}

	Bool fovYDirivedFromAspect = comp.getFovYDerivesByRendererAspect();
	if(ImGui::Checkbox("FovX derived by aspect", &fovYDirivedFromAspect))
	{
		comp.setFovYDerivesByRendererAspect(fovYDirivedFromAspect);
	}

	if(!comp.getFovYDerivesByRendererAspect())
	{
		F32 fovY = toDegrees(comp.getFovY());
		if(ImGui::SliderFloat("FovY", &fovY, 10.0f, 200.0f))
		{
			comp.setFovY(toRad(fovY));
		}
	}
}

void SceneNodePropertiesUi::skyboxComponent(SkyboxComponent& comp)
{
	// Type
	if(ImGui::BeginCombo("Type", kSkyboxComponentTypeNames[comp.getSkyboxComponentType()]))
	{
		for(SkyboxComponentType type : EnumIterable<SkyboxComponentType>())
		{
			const Bool selected = type == comp.getSkyboxComponentType();
			if(ImGui::Selectable(kSkyboxComponentTypeNames[type], selected))
			{
				comp.setSkyboxComponentType(type);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if(selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if(comp.getSkyboxComponentType() == SkyboxComponentType::kSolidColor)
	{
		Vec3 color = comp.getSkySolidColor();
		if(ImGui::InputFloat3("Solid Color", &color.x))
		{
			comp.setSkySolidColor(color);
		}
	}
	else if(comp.getSkyboxComponentType() == SkyboxComponentType::kImage2D)
	{
		// Filenames combo
		{
			const DynamicArray<CString> filenames = gatherResourceFilenames(".ankitex");

			const String currentFilename = (comp.hasSkyImageFilename()) ? comp.getSkyImageFilename() : "";
			U32 newSelectedFilename = kMaxU32;

			ImGui::SetNextItemWidth(-1.0f);
			const Bool selected = comboWithFilter("##Filenames", filenames, currentFilename, newSelectedFilename, m_tempFilter);

			if(selected && currentFilename != filenames[newSelectedFilename])
			{
				comp.setSkyImageFilename(filenames[newSelectedFilename]);
			}
		}

		Vec3 bias = comp.getSkyImageColorBias();
		if(ImGui::SliderFloat3("Image Color Bias", &bias.x, 0.0f, 1000.0f))
		{
			comp.setSkyImageColorBias(bias);
		}

		Vec3 scale = comp.getSkyImageColorScale();
		if(ImGui::SliderFloat3("Image Color Scale", &scale.x, 0.0f, 1000.0f))
		{
			comp.setSkyImageColorScale(scale);
		}
	}

	// Fog stuff:

#define ANKI_INPUT(methodName, labelTxt) \
	{ \
		F32 value = comp.get##methodName(); \
		if(ImGui::SliderFloat(labelTxt, &value, 0.0f, 1000.0f)) \
		{ \
			comp.set##methodName(value); \
		} \
	}

	ImGui::TextUnformatted("Fog:");

	ANKI_INPUT(MinFogDensity, "Min Density")
	ANKI_INPUT(MaxFogDensity, "Max Density")
	ANKI_INPUT(HeightOfMinFogDensity, "Height of Min Density")
	ANKI_INPUT(HeightOfMaxFogDensity, "Height of Max Density")
	ANKI_INPUT(FogScatteringCoefficient, "Scattering Coeff")
	ANKI_INPUT(FogAbsorptionCoefficient, "Absorption Coeff")

#undef ANKI_INPUT

	Vec3 fogColor = comp.getFogDiffuseColor();
	if(ImGui::SliderFloat3("Color", &fogColor.x, 0.0f, 100.0f))
	{
		comp.setFogDiffuseColor(fogColor);
	}
}

} // end namespace anki
