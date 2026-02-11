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

void MaterialEditorUi::drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags)
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
	if(ImGui::Begin("Material Editor", &m_open, windowFlags))
	{
		if(ImGui::BeginChild("Toolbox", Vec2(0.0f), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY,
							 ImGuiWindowFlags_None))
		{
			if(ImGui::Button(ICON_MDI_CONTENT_SAVE " Save"))
			{
				if(saveCache())
				{
					ANKI_LOGE("Unnable to save the particles file. Ignoring save");
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

				if(inp.m_type == ShaderVariableDataType::kVec3)
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
					Bool bImage = !!inp.m_image;
					ImGui::Checkbox("Image", &bImage);

					ImGui::SameLine();

					if(bImage)
					{
						ImageResourcePtr img = inp.m_image ? inp.m_image : m_placeholderImage;
						inp.m_image = img;

						// Image dropdown
						const String currentFilepath = img->getFilename();
						U32 newSelectedFilepath;
						const Bool selected = comboWithFilter("##Filenames", textureFilepaths, currentFilepath, newSelectedFilepath, m_tempFilter);
						if(selected && currentFilepath != textureFilepaths[newSelectedFilepath])
						{
							inp.m_image.reset(nullptr);
							if(ResourceManager::getSingleton().loadResource(textureFilepaths[newSelectedFilepath], inp.m_image))
							{
								inp.m_image = m_placeholderImage;
							}
						}

						// Name of the input
						ImGui::SameLine();
						ImGui::TextUnformatted(inp.m_name.cstr());

						// Show the image
						ImTextureID id;
						id.m_texture = &img->getTexture();
						ImGui::Image(id, Vec2(128.0f));
					}
					else
					{
						inp.m_image.reset(nullptr);

						I32 val = I32(inp.m_U32);
						if(ImGui::InputInt(inp.m_name.cstr(), &val))
						{
							val = max(0, val);
						}
						inp.m_U32 = U32(val);
					}
				}
				else
				{
					ImGui::Text("TODO: Unhandled type for: %s", inp.m_name.cstr());
				}

				ImGui::PopID();
			}

			ImGui::EndChild();
		}
	}
	ImGui::End();

	if(cacheDirty)
	{
		rebuildCache(m_currentlySelectedProgram);
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
	m_cachedMutators.resize(mtl.getPartialMutation().getSize());
	U32 count = 0;
	for(const PartialMutation& mutation : mtl.getPartialMutation())
	{
		m_cachedMutators[count++] = mutation;
	}

	m_cachedInputs.resize(mtl.getVariables().getSize());
	count = 0;
	for(const MaterialVariable& var : mtl.getVariables())
	{
		if(var.tryGetImageResource())
		{
			// Image
			m_cachedInputs[count].m_image.reset(var.tryGetImageResource());
		}
		else
		{
			// Non-image

			switch(var.getDataType())
			{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
		m_cachedInputs[count].m_##type = var.getValue<type>(); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.def.h>
			default:
				ANKI_ASSERT(0);
			}
		}

		m_cachedInputs[count].m_type = var.getDataType();
		m_cachedInputs[count].m_name = var.getName();
		++count;
	}

	m_currentlySelectedProgram = getBasename(mtl.getShaderProgramResource().getFilename());
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

	m_cachedInputs.destroy();
	m_cachedInputs.resize(prog->m_inputs.getSize());
	U32 count = 0;
	for(const ShaderBinaryStructMember& m : prog->m_inputs)
	{
		m_cachedInputs[count].m_Mat4 = Mat4::getZero();
		m_cachedInputs[count].m_name = m.m_name.getBegin();
		m_cachedInputs[count].m_type = m.m_type;

		String lowerName = m.m_name.getBegin();
		lowerName.toLower();

		const Bool mightBeTexture = m.m_type == ShaderVariableDataType::kU32 && lowerName.find("tex") != String::kNpos;
		if(mightBeTexture)
		{
			m_cachedInputs[count].m_image = m_placeholderImage;
		}

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

		PartialMutation& pmut = *m_cachedMutators.emplaceBack();

		pmut.m_mutator = &m;
		pmut.m_value = m.m_values[0];
	}

	ANKI_ASSERT(prog);
}

Error MaterialEditorUi::saveCache()
{
	File file;
	ANKI_CHECK(file.open(m_filepath, FileOpenFlag::kWrite));

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
		if(inp.m_type == ShaderVariableDataType::kF32)
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
			if(inp.m_image)
			{
				value = inp.m_image->getFilename();
			}
			else
			{
				value.sprintf("%u", inp.m_U32);
			}
		}
		else
		{
			ANKI_LOGI("Unhandled case");
		}

		ANKI_CHECK(file.writeTextf("\t\t<input name=\"%s\" value=\"%s\"/>\n", inp.m_name.cstr(), value.cstr()));
	}
	ANKI_CHECK(file.writeText("\t</inputs>\n"));

	ANKI_CHECK(file.writeText("</material>\n"));

	return Error::kNone;
}

} // end namespace anki
