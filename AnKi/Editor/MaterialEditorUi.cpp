// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/MaterialEditorUi.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Resource/MaterialResource.h>

namespace anki {

void MaterialEditorUi::open(const MaterialResource& resource)
{
	if(m_programs.getSize() == 0)
	{
		gatherPrograms();
	}

	if(!m_placeholderImage)
	{
		ANKI_CHECKF(ResourceManager::getSingleton().loadResource("EngineAssets/PlaceholderOrange.png", m_placeholderImage));
	}

	m_filepath = resource.getFilename();

	rebuildCache(resource);
	m_open = true;
}

void MaterialEditorUi::drawWindow(Vec2 initialPos, Vec2 initialSize, String& resourceToLocate, ImGuiWindowFlags windowFlags)
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

	Bool cacheDirty = false;
	if(ImGui::Begin(ICON_MDI_TEXT_BOX " Material Editor", &m_open, windowFlags))
	{
		if(ImGui::BeginChild("Toolbox", Vec2(0.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
							 ImGuiWindowFlags_None))
		{
			if(ImGui::Button(ICON_MDI_CONTENT_SAVE " Save"))
			{
				if(saveCache())
				{
					ANKI_LOGE("Unable to save the material file. Ignoring save");
				}
				ResourceManager::getSingleton().refreshFileUpdateTimes();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Separator();
			ImGui::EndChild();
		}

		if(ImGui::BeginChild("Content", Vec2(-1.0f, -1.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX, windowFlags))
		{
			drawContent(cacheDirty, resourceToLocate);
			ImGui::EndChild();
		}
	}
	ImGui::End();

	if(cacheDirty)
	{
		rebuildCache(m_currentlySelectedProgram);
	}
}

void MaterialEditorUi::drawContent(Bool& cacheDirty, String& resourceToLocate)
{
	if(ImGui::BeginCombo("Program", (m_currentlySelectedProgram.getLength()) ? m_currentlySelectedProgram.cstr() : nullptr,
						 ImGuiComboFlags_HeightLarge))
	{
		for(U32 i = 0; i < m_programs.getSize(); ++i)
		{
			const Bool isSelected = (m_programs[i].m_name == m_currentlySelectedProgram);
			if(ImGui::Selectable(m_programs[i].m_name.cstr(), isSelected))
			{
				cacheDirty = cacheDirty || m_currentlySelectedProgram != m_programs[i].m_name;
				m_currentlySelectedProgram = m_programs[i].m_name;
			}

			if(isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Mutators
	ImGui::SeparatorText("Mutation");
	for(PartialMutation& mutator : m_cachedMutators)
	{
		String value;
		value.toString(mutator.m_value);
		if(ImGui::BeginCombo(mutator.m_mutator->m_name.getBegin(), value.cstr()))
		{
			for(MutatorValue mval : mutator.m_mutator->m_values)
			{
				const Bool selected = mutator.m_value == mval;
				String value2;
				value2.toString(mval);
				if(ImGui::Selectable(value2.cstr(), selected))
				{
					mutator.m_value = mval;
				}

				if(selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
	}

	// Input variables
	DynamicArray<CString> textureFilepaths = gatherResourceFilenames(".ankitex");
	DynamicArray<CString> textureFilepaths2 = gatherResourceFilenames(".png");
	for(CString other : textureFilepaths2)
	{
		textureFilepaths.emplaceBack(other);
	}
	ImGui::SeparatorText("Shader Input");
	for(U32 i = 0; i < m_cachedInputs.getSize(); ++i)
	{
		ImGui::PushID(i);

		Data& inp = m_cachedInputs[i];

		if(inp.m_isTexture)
		{
			Bool hasImage = !!inp.m_image;

			// Locate the image
			ImGui::BeginDisabled(!hasImage);
			if(ImGui::Button(ICON_MDI_TARGET))
			{
				resourceToLocate = inp.m_image->getFilename();
			}
			ImGui::EndDisabled();
			ImGui::SetItemTooltip("Locate resource in asset browser");
			ImGui::SameLine();

			// Clear image dropdown
			if(ImGui::Button(ICON_MDI_DELETE))
			{
				hasImage = false;
				inp.m_image.reset(nullptr);
			}
			ImGui::SetItemTooltip("Clear the image");
			ImGui::SameLine();

			// Image dropdown
			const String currentFilepath = (hasImage) ? inp.m_image->getFilename() : "";
			U32 newSelectedFilepath;
			const Bool selected = comboWithFilter("##Filenames", textureFilepaths, currentFilepath, newSelectedFilepath, m_tempFilter);
			if(selected && currentFilepath != textureFilepaths[newSelectedFilepath])
			{
				ImageResourcePtr newImg;
				if(!ResourceManager::getSingleton().loadResource(textureFilepaths[newSelectedFilepath], newImg))
				{
					inp.m_image = newImg;
				}
			}

			// Drag and drop
			if(ImGui::BeginDragDropTarget())
			{
				if(const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(kTextureAssetDragDropPayload))
				{
					ANKI_ASSERT(pl->Data && pl->DataSize > 0);
					const CString droppedName(static_cast<const char*>(pl->Data));

					ImageResourcePtr newImg;
					if(!ResourceManager::getSingleton().loadResource(droppedName, newImg))
					{
						inp.m_image = newImg;
					}
				}
				ImGui::EndDragDropTarget();
			}

			// Name of the input
			ImGui::SameLine();
			ImGui::TextUnformatted(inp.m_name.cstr());

			// Show the image
			if(inp.m_image)
			{
				ImTextureID id;
				id.m_texture = &inp.m_image->getTexture();
				ImGui::Image(id, Vec2(128.0f));
			}
		}
		else if(inp.m_type == ShaderVariableDataType::kVec3)
		{
			ImGui::InputFloat3(inp.m_name.cstr(), &inp.m_Vec3[0]);
		}
		else if(inp.m_type == ShaderVariableDataType::kVec4)
		{
			ImGui::InputFloat4(inp.m_name.cstr(), &inp.m_Vec4[0]);
		}
		else if(inp.m_type == ShaderVariableDataType::kF32)
		{
			ImGui::InputFloat(inp.m_name.cstr(), &inp.m_F32);
		}
		else if(inp.m_type == ShaderVariableDataType::kU32)
		{
			I32 val = inp.m_U32;
			ImGui::InputInt(inp.m_name.cstr(), &val);
			inp.m_U32 = max(val, 0);
		}
		else
		{
			ImGui::Text("TODO: Unhandled type for: %s", inp.m_name.cstr());
		}

		ImGui::PopID();
	}
}

void MaterialEditorUi::gatherPrograms()
{
	m_programs.destroy();

	ResourceFilesystem::getSingleton().iterateAllFilenames([this](CString filename) {
		const String ext = getFileExtension(filename);
		if(ext == "ankiprogbin")
		{
			ResourceFilePtr file;
			ShaderBinary* binary = nullptr;
			if(ResourceFilesystem::getSingleton().openFile(filename, file)
			   || deserializeShaderBinaryFromAnyFile(*file, binary, DefaultMemoryPool::getSingleton()))
			{
				ANKI_LOGE("Failed to de-serialize file: %s", filename.cstr());
			}
			else
			{
				for(const auto& strct : binary->m_structs)
				{
					if(CString(strct.m_name.getBegin()).find("AnKiLocalConstants") == 0)
					{
						// Found an interesting program, stash it

						const String basename = getBasename(filename);

						Program& prog = *m_programs.emplaceBack();
						prog.m_name = basename;
						prog.m_programBinary = binary;

						prog.m_inputs = strct.m_members;
						prog.m_mutators = binary->m_mutators;

						binary = nullptr; // To avoid deletion a few lines bellow
					}
				}

				DefaultMemoryPool::getSingleton().free(binary);
			}
		}
		return FunctorContinue::kContinue;
	});
}

void MaterialEditorUi::rebuildCache(const MaterialResource& mtl)
{
	// Create a default cache
	m_currentlySelectedProgram = getBasename(mtl.getShaderProgramResource().getFilename());
	rebuildCache(m_currentlySelectedProgram);

	// Build the partial mutation
	for(const PartialMutation& mutation : mtl.getPartialMutation())
	{
		// Find the mutator in the cache and change its value to what the material has
		[[maybe_unused]] Bool found = false;
		for(PartialMutation& m : m_cachedMutators)
		{
			if(CString(m.m_mutator->m_name.getBegin()) == mutation.m_mutator->m_name.getBegin())
			{
				m.m_value = mutation.m_value;
				found = true;
				break;
			}
		}
		ANKI_ASSERT(found);
	}

	for(const MaterialVariable& var : mtl.getVariables())
	{
		// Find the var in the cache
		[[maybe_unused]] Bool found = false;
		for(Data& d : m_cachedInputs)
		{
			if(d.m_name == var.getName())
			{
				if(var.tryGetImageResource())
				{
					d.m_image.reset(var.tryGetImageResource());
				}
				else
				{
					switch(var.getDataType())
					{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
		d.m_##type = var.getValue<type>(); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.def.h>
					default:
						ANKI_ASSERT(0);
					}
				}

				ANKI_ASSERT(d.m_type == var.getDataType());
				found = true;
				break;
			}
		}
		ANKI_ASSERT(found);
	}
}

void MaterialEditorUi::rebuildCache(CString programName)
{
	m_currentlySelectedProgram = programName;

	const Program* prog = nullptr;
	for(const Program& p : m_programs)
	{
		if(p.m_name == programName)
		{
			prog = &p;
			break;
		}
	}
	ANKI_ASSERT(prog);

	m_cachedInputs.resize(prog->m_inputs.getSize());
	U32 count = 0;
	for(const ShaderBinaryStructMember& m : prog->m_inputs)
	{
		// Set a default value from the binary
		m_cachedInputs[count].m_Vec4 = Vec4(0.0f);
		switch(m.m_type)
		{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
		ANKI_ASSERT(sizeof(m_cachedInputs[count].m_Vec4) == m.m_defaultValues.getSizeInBytes()); \
		memcpy(&m_cachedInputs[count].m_Vec4, m.m_defaultValues.getBegin(), m.m_defaultValues.getSizeInBytes()); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.def.h>
		default:
			ANKI_ASSERT(0);
		}

		m_cachedInputs[count].m_name = m.m_name.getBegin();
		m_cachedInputs[count].m_type = m.m_type;
		m_cachedInputs[count].m_image.reset(nullptr);
		m_cachedInputs[count].m_isTexture = m.m_isTexture;

		++count;
	}

	m_cachedMutators.destroy();
	for(const ShaderBinaryMutator& m : prog->m_mutators)
	{
		if(CString(m.m_name.getBegin()).find("ANKI_") == 0)
		{
			// Builtin, skip it
			continue;
		}

		m_cachedMutators.emplaceBack(PartialMutation{.m_mutator = &m, .m_value = m.m_values[0]});
	}
}

Error MaterialEditorUi::saveCache()
{
	const ResourceString diskFilepath = ResourceFilesystem::getSingleton().getFileFullPath(m_filepath);

	File file;
	ANKI_CHECK(file.open(diskFilepath, FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeText("<!-- This file is generated by the editor -->\n"));
	ANKI_CHECK(file.writeText("<material shadows=\"1\">\n"));

	ANKI_CHECK(file.writeTextf("\t<shaderProgram name=\"%s\">\n", m_currentlySelectedProgram.cstr()));

	ANKI_CHECK(file.writeText("\t\t<mutation>\n"));
	for(const PartialMutation& pmut : m_cachedMutators)
	{
		ANKI_CHECK(file.writeTextf("\t\t\t<mutator name=\"%s\" value=\"%d\"/>\n", pmut.m_mutator->m_name.getBegin(), pmut.m_value));
	}
	ANKI_CHECK(file.writeText("\t\t</mutation>\n"));

	ANKI_CHECK(file.writeText("\t</shaderProgram>\n"));

	ANKI_CHECK(file.writeText("\t<inputs>\n"));
	for(const Data& inp : m_cachedInputs)
	{
		String value;
		if(inp.m_isTexture)
		{
			if(!inp.m_image)
			{
				// No image assigned, skip it
				continue;
			}

			value = inp.m_image->getFilename();
		}
		else if(inp.m_type == ShaderVariableDataType::kF32)
		{
			value.sprintf("%f", inp.m_F32);
		}
		else if(inp.m_type == ShaderVariableDataType::kVec3)
		{
			value.sprintf("%f %f %f", inp.m_Vec3.x, inp.m_Vec3.y, inp.m_Vec3.z);
		}
		else if(inp.m_type == ShaderVariableDataType::kVec4)
		{
			value.sprintf("%f %f %f %f", inp.m_Vec4.x, inp.m_Vec4.y, inp.m_Vec4.z, inp.m_Vec4.w);
		}
		else if(inp.m_type == ShaderVariableDataType::kU32)
		{
			value.sprintf("%u", inp.m_U32);
		}
		else
		{
			ANKI_LOGW("Unhandled input type, not saving: %s", inp.m_name.cstr());
			continue;
		}

		ANKI_CHECK(file.writeTextf("\t\t<input name=\"%s\" value=\"%s\"/>\n", inp.m_name.cstr(), value.cstr()));
	}
	ANKI_CHECK(file.writeText("\t</inputs>\n"));

	ANKI_CHECK(file.writeText("</material>\n"));

	return Error::kNone;
}

} // end namespace anki
