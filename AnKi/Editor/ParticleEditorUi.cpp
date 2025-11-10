// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/ParticleEditorUi.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Filesystem.h>
#include <ThirdParty/ImGui/Extra/IconsMaterialDesignIcons.h> // See all icons in https://pictogrammers.com/library/mdi/

namespace anki {

void ParticleEditorUi::open(const ParticleEmitterResource2& resource)
{
	if(m_programs.getSize() == 0)
	{
		gatherParticlePrograms();
	}

	rebuildCache(resource);
	m_filename = ResourceFilesystem::getSingleton().getFileFullPath(resource.getFilename());
	m_open = true;
}

void ParticleEditorUi::drawWindow([[maybe_unused]] UiCanvas& canvas, Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	if(m_programs.getSize() == 0)
	{
		gatherParticlePrograms();
	}

	if(ImGui::GetFrameCount() > 1)
	{
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
	}

	// Do the window now
	Bool cacheDirty = false;
	if(ImGui::Begin("Particle Editor", &m_open, windowFlags))
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
			ImGui::SeparatorText("Common Properties");

			// <shaderProgram>
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

			// <particleCount>
			I32 particleCount = I32(m_commonProps.m_particleCount);
			if(ImGui::InputInt("Particle Count", &particleCount))
			{
				m_commonProps.m_particleCount = U32(max(0, particleCount));
			}

			// <emissionPeriod>
			if(ImGui::InputFloat("Emission Period", &m_commonProps.m_emissionPeriod, 0.01f))
			{
				m_commonProps.m_emissionPeriod = max(0.0f, m_commonProps.m_emissionPeriod);
			}

			// <particlesPerEmission>
			I32 particlesPerEmission = I32(m_commonProps.m_particlesPerEmission);
			if(ImGui::InputInt("Particles Per Emission", &particlesPerEmission))
			{
				m_commonProps.m_particlesPerEmission = U32(clamp(particlesPerEmission, 0, particleCount));
			}

			ImGui::SeparatorText("Other Properties");

			// Other props
			for(Prop& prop : m_otherProps)
			{
				[[maybe_unused]] Bool valueChanged = false;
				switch(prop.m_type)
				{
				case ShaderVariableDataType::kU32:
					valueChanged = ImGui::InputInt(prop.m_name.cstr(), &prop.m_I32);
					prop.m_I32 = max(prop.m_I32, 0);
					break;
				case ShaderVariableDataType::kUVec2:
					valueChanged = ImGui::InputInt2(prop.m_name.cstr(), &prop.m_I32);
					prop.m_IVec2 = prop.m_IVec2.max(0);
					break;
				case ShaderVariableDataType::kUVec3:
					valueChanged = ImGui::InputInt3(prop.m_name.cstr(), &prop.m_I32);
					prop.m_IVec3 = prop.m_IVec3.max(0);
					break;
				case ShaderVariableDataType::kUVec4:
					valueChanged = ImGui::InputInt4(prop.m_name.cstr(), &prop.m_I32);
					prop.m_IVec4 = prop.m_IVec4.max(0);
					break;
				case ShaderVariableDataType::kF32:
					valueChanged = ImGui::InputFloat(prop.m_name.cstr(), &prop.m_F32, 1.0f);
					break;
				case ShaderVariableDataType::kVec2:
					valueChanged = ImGui::InputFloat2(prop.m_name.cstr(), &prop.m_F32);
					break;
				case ShaderVariableDataType::kVec3:
					valueChanged = ImGui::InputFloat3(prop.m_name.cstr(), &prop.m_F32);
					break;
				case ShaderVariableDataType::kVec4:
					valueChanged = ImGui::InputFloat4(prop.m_name.cstr(), &prop.m_F32);
					break;
				default:
					ANKI_ASSERT(!"TODO");
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	if(cacheDirty)
	{
		rebuildCache(m_currentlySelectedProgram);
	}
}

void ParticleEditorUi::gatherParticlePrograms()
{
	ResourceFilesystem::getSingleton().iterateAllFilenames([this](CString filename) {
		String ext;
		getFilepathExtension(filename, ext);
		const CString extension = "ankiprogbin";
		if(ext == extension)
		{
			ResourceFilePtr file;
			ShaderBinary* binary = nullptr;
			if(ResourceFilesystem::getSingleton().openFile(filename, file)
			   || deserializeShaderBinaryFromAnyFile(*file, binary, DefaultMemoryPool::getSingleton()))
			{
				ANKI_LOGE("Failed to load particles file. Ignoring this file: %s", filename.cstr());
			}
			else
			{
				for(const auto& strct : binary->m_structs)
				{
					if(CString(strct.m_name.getBegin()).find("AnKiParticleEmitterProperties") == 0)
					{
						String basename;
						getFilepathFilename(filename, basename);
						ANKI_ASSERT(basename.getLength() > extension.getLength() + 1);
						basename = String(basename.getBegin(), basename.getBegin() + basename.getLength() - extension.getLength() - 1);

						ParticleProgram& prog = *m_programs.emplaceBack();
						prog.m_filename = filename;
						prog.m_name = basename;
						prog.m_props.resize(strct.m_members.getSize());

						for(U32 i = 0; i < prog.m_props.getSize(); ++i)
						{
							prog.m_props[i].m_name = strct.m_members[i].m_name.getBegin();
							prog.m_props[i].m_type = strct.m_members[i].m_type;

							switch(prog.m_props[i].m_type)
							{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
	{ \
		memcpy(&prog.m_props[i].m_F32, strct.m_members[i].m_defaultValues.getBegin(), sizeof(type)); \
		break; \
	}
#include <AnKi/Gr/ShaderVariableDataType.def.h>
							default:
								ANKI_ASSERT(0);
								break;
							}
						}
					}
				}

				DefaultMemoryPool::getSingleton().free(binary);
			}
		}
		return FunctorContinue::kContinue;
	});
}

void ParticleEditorUi::rebuildCache(const ParticleEmitterResource2& rsrc)
{
	m_commonProps = rsrc.getCommonProperties();

	m_currentlySelectedProgram = rsrc.getShaderProgramResource().getFilename();
	getFilepathFilename(m_currentlySelectedProgram, m_currentlySelectedProgram);
	const CString extension = "ankiprogbin";
	m_currentlySelectedProgram = String(m_currentlySelectedProgram.getBegin(),
										m_currentlySelectedProgram.getBegin() + m_currentlySelectedProgram.getLength() - (extension.getLength() + 1));

	m_otherProps.resize(rsrc.getOtherProperties().getSize());
	for(U32 i = 0; i < m_otherProps.getSize(); ++i)
	{
		m_otherProps[i].m_name = rsrc.getOtherProperties()[i].getName();
		m_otherProps[i].m_type = rsrc.getOtherProperties()[i].getDataType();

		switch(m_otherProps[i].m_type)
		{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
	{ \
		m_otherProps[i].m_##type = rsrc.getOtherProperties()[i].getValue<type>(); \
		break; \
	}
#include <AnKi/Gr/ShaderVariableDataType.def.h>

		default:
			ANKI_ASSERT(0);
			break;
		}
	}
}

void ParticleEditorUi::rebuildCache(CString particleProgramName)
{
	const ParticleProgram* foundProg = nullptr;
	for(const ParticleProgram& prog : m_programs)
	{
		if(particleProgramName == prog.m_name)
		{
			foundProg = &prog;
			break;
		}
	}

	ANKI_ASSERT(foundProg);

	m_commonProps = {};
	m_currentlySelectedProgram = particleProgramName;

	m_otherProps.resize(foundProg->m_props.getSize());
	for(U32 i = 0; i < m_otherProps.getSize(); ++i)
	{
		m_otherProps[i].m_name = foundProg->m_props[i].m_name;
		m_otherProps[i].m_type = foundProg->m_props[i].m_type;
		m_otherProps[i].m_Mat4 = foundProg->m_props[i].m_Mat4;
	}
}

Error ParticleEditorUi::saveCache()
{
	File file;
	ANKI_CHECK(file.open(m_filename, FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeText("<particleEmitter>\n"));

	ANKI_CHECK(file.writeTextf("\t<shaderProgram name=\"%s\"/>\n", m_currentlySelectedProgram.cstr()));

	ANKI_CHECK(file.writeTextf("\t<particleCount value=\"%u\"/>\n", m_commonProps.m_particleCount));
	ANKI_CHECK(file.writeTextf("\t<emissionPeriod value=\"%f\"/>\n", m_commonProps.m_emissionPeriod));
	ANKI_CHECK(file.writeTextf("\t<particlesPerEmission value=\"%u\"/>\n", m_commonProps.m_particlesPerEmission));

	ANKI_CHECK(file.writeText("\t<inputs>\n"));

	for(const Prop& prop : m_otherProps)
	{
		String value;
		switch(prop.m_type)
		{
		case ShaderVariableDataType::kU32:
			value.toString(prop.m_U32);
			break;
		case ShaderVariableDataType::kUVec2:
			value = prop.m_UVec2.toString();
			break;
		case ShaderVariableDataType::kUVec3:
			value = prop.m_UVec3.toString();
			break;
		case ShaderVariableDataType::kUVec4:
			value = prop.m_UVec4.toString();
			break;
		case ShaderVariableDataType::kF32:
			value.toString(prop.m_F32);
			break;
		case ShaderVariableDataType::kVec2:
			value = prop.m_Vec2.toString();
			break;
		case ShaderVariableDataType::kVec3:
			value = prop.m_Vec3.toString();
			break;
		case ShaderVariableDataType::kVec4:
			value = prop.m_Vec4.toString();
			break;
		default:
			ANKI_ASSERT(!"TODO");
		}

		ANKI_CHECK(file.writeTextf("\t\t<input name=\"%s\" value=\"%s\"/>\n", prop.m_name.cstr(), value.cstr()));
	}

	ANKI_CHECK(file.writeText("\t</inputs>\n"));

	ANKI_CHECK(file.writeText("</particleEmitter>\n"));

	return Error::kNone;
}

} // end namespace anki
