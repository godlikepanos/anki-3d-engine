// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ParticleEmitterResource2.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

static Bool mutatorValueExists(const ShaderBinaryMutator& m, MutatorValue val)
{
	for(MutatorValue v : m.m_values)
	{
		if(v == val)
		{
			return true;
		}
	}

	return false;
}

Error ParticleEmitterResource2::load(const ResourceFilename& filename, Bool async)
{
	ResourceXmlDocument doc;
	XmlElement el;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <material>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("particleEmitter", rootEl));

	// <shaderPrograms>
	XmlElement shaderProgramEl;
	ANKI_CHECK(rootEl.getChildElement("shaderProgram", shaderProgramEl));
	ANKI_CHECK(parseShaderProgram(shaderProgramEl, async));

	// Inputs
	ANKI_CHECK(rootEl.getChildElementOptional("inputs", el));
	if(el)
	{
		XmlElement inputEl;
		ANKI_CHECK(el.getChildElement("input", inputEl));
		do
		{
			ANKI_CHECK(parseInput(inputEl));
			ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
		} while(inputEl);
	}

	return Error::kNone;
}

Error ParticleEmitterResource2::parseShaderProgram(XmlElement shaderProgramEl, Bool async)
{
	// name
	CString shaderName;
	ANKI_CHECK(shaderProgramEl.getAttributeText("name", shaderName));

	ResourceString fname;
	fname.sprintf("ShaderBinaries/%s.ankiprogbin", shaderName.cstr());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource(fname, m_prog, async));

	// <mutation>
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	XmlElement mutatorsEl;
	ANKI_CHECK(shaderProgramEl.getChildElementOptional("mutation", mutatorsEl));
	if(mutatorsEl)
	{
		ANKI_CHECK(parseMutators(mutatorsEl, variantInitInfo));
	}

	// Create the GR prog
	variantInitInfo.addMutation("ANKI_WAVE_SIZE", GrManager::getSingleton().getDeviceCapabilities().m_maxWaveSize);
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);

	if(!variant)
	{
		ANKI_RESOURCE_LOGE("Shader variant creation failed");
		return Error::kUserData;
	}

	m_grProg.reset(&variant->getProgram());

	// Create the properties
	{
		// Find struct
		const ShaderBinary& binary = m_prog->getBinary();
		const ShaderBinaryStruct* propsStruct = nullptr;
		for(const ShaderBinaryStruct& strct : binary.m_structs)
		{
			if(CString(strct.m_name.getBegin()) == "AnKiParticleEmitterProperties")
			{
				propsStruct = &strct;
				break;
			}
		}

		if(!propsStruct)
		{
			ANKI_RESOURCE_LOGE("AnKiParticleEmitterProperties struct not found");
			return Error::kUserData;
		}

		m_prefilledAnKiParticleEmitterProperties.resize(propsStruct->m_size, 0_U8);

		for(U32 i = 0; i < propsStruct->m_members.getSize(); ++i)
		{
			const ShaderBinaryStructMember& member = propsStruct->m_members[i];
			const U32 memberSize = getShaderVariableDataTypeInfo(member.m_type).m_size;

			ANKI_ASSERT(member.m_offset + memberSize <= propsStruct->m_size);
			memcpy(m_prefilledAnKiParticleEmitterProperties.getBegin() + member.m_offset, member.m_defaultValues.getBegin(), memberSize);
		}
	}

	return Error::kNone;
}

Error ParticleEmitterResource2::parseMutators(XmlElement mutatorsEl, ShaderProgramResourceVariantInitInfo& variantInitInfo)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);

	do
	{
		// name
		CString mutatorName;
		ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
		if(mutatorName.isEmpty())
		{
			ANKI_RESOURCE_LOGE("Mutator name is empty");
			return Error::kUserData;
		}

		if(mutatorName.find("ANKI_") == 0)
		{
			ANKI_RESOURCE_LOGE("Mutators can't start with ANKI_: %s", mutatorName.cstr());
			return Error::kUserData;
		}

		// value
		MutatorValue mval;
		ANKI_CHECK(mutatorEl.getAttributeNumber("value", mval));

		// Find mutator
		const ShaderBinaryMutator* mutator = m_prog->tryFindMutator(mutatorName);

		if(!mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::kUserData;
		}

		if(!mutatorValueExists(*mutator, mval))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", mval, mutatorName.cstr());
			return Error::kUserData;
		}

		variantInitInfo.addMutation(mutatorName, mval);

		// Advance
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	return Error::kNone;
}

Error ParticleEmitterResource2::parseInput(XmlElement inputEl)
{
	// Get var name
	CString varName;
	ANKI_CHECK(inputEl.getAttributeText("name", varName));

	// Find struct
	const ShaderBinary& binary = m_prog->getBinary();
	const ShaderBinaryStruct* propsStruct = nullptr;
	for(const ShaderBinaryStruct& strct : binary.m_structs)
	{
		if(CString(strct.m_name.getBegin()) == "AnKiParticleEmitterProperties")
		{
			propsStruct = &strct;
			break;
		}
	}
	ANKI_ASSERT(propsStruct);

	// Find the member
	const ShaderBinaryStructMember* foundMember = nullptr;
	for(U32 i = 0; i < propsStruct->m_members.getSize(); ++i)
	{
		const ShaderBinaryStructMember& member = propsStruct->m_members[i];
		const CString memberName = member.m_name.getBegin();
		if(memberName == varName)
		{
			foundMember = &member;
			break;
		}
	}

	if(!foundMember)
	{
		ANKI_RESOURCE_LOGE("Input not found in AnKiParticleEmitterProperties: %s", varName.cstr());
		return Error::kUserData;
	}

	const U32 memberSize = getShaderVariableDataTypeInfo(foundMember->m_type).m_size;
	const U32 memberOffset = foundMember->m_offset;

	// Set the value
	switch(foundMember->m_type)
	{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
	{ \
		Array<baseType, rowCount * columnCount> arr; \
		ANKI_CHECK(inputEl.getNumbers(arr)); \
		ANKI_ASSERT(memberOffset + memberSize <= m_prefilledAnKiParticleEmitterProperties.getSize()); \
		ANKI_ASSERT(memberSize == sizeof(arr)); \
		memcpy(m_prefilledAnKiParticleEmitterProperties.getBegin() + memberOffset, arr.getBegin(), memberSize); \
		break; \
	}
#include <AnKi/Gr/ShaderVariableDataType.def.h>

	default:
		ANKI_ASSERT(0);
		break;
	}

	return Error::kNone;
}

} // end namespace anki
