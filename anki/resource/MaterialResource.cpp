// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MaterialResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/TextureResource.h>
#include <anki/util/Xml.h>

namespace anki
{

static const Array<CString, U32(BuiltinMutatorId::COUNT)> BUILTIN_MUTATOR_NAMES = {
	{"NONE", "ANKI_INSTANCE_COUNT", "ANKI_PASS", "ANKI_LOD", "ANKI_BONES", "ANKI_VELOCITY"}};

class BuiltinVarInfo
{
public:
	const char* m_name;
	ShaderVariableDataType m_type;
	Bool m_instanced;
};

static const Array<BuiltinVarInfo, U(BuiltinMaterialVariableId::COUNT)> BUILTIN_INFOS = {
	{{"NONE", ShaderVariableDataType::NONE, false},
	 {"m_ankiMvp", ShaderVariableDataType::MAT4, true},
	 {"m_ankiPreviousMvp", ShaderVariableDataType::MAT4, true},
	 {"m_ankiModelMatrix", ShaderVariableDataType::MAT4, true},
	 {"m_ankiViewMatrix", ShaderVariableDataType::MAT4, false},
	 {"m_ankiProjectionMatrix", ShaderVariableDataType::MAT4, false},
	 {"m_ankiModelViewMatrix", ShaderVariableDataType::MAT4, true},
	 {"m_ankiViewProjectionMatrix", ShaderVariableDataType::MAT4, false},
	 {"m_ankiNormalMatrix", ShaderVariableDataType::MAT3, true},
	 {"m_ankiRotationMatrix", ShaderVariableDataType::MAT3, true},
	 {"m_ankiCameraRotationMatrix", ShaderVariableDataType::MAT3, false},
	 {"m_ankiCameraPosition", ShaderVariableDataType::VEC3, false},
	 {"u_ankiGlobalSampler", ShaderVariableDataType::SAMPLER, false}}};

static ANKI_USE_RESULT Error checkBuiltin(CString name, ShaderVariableDataType dataType, Bool instanced,
										  BuiltinMaterialVariableId& outId)
{
	outId = BuiltinMaterialVariableId::NONE;

	for(BuiltinMaterialVariableId id : EnumIterable<BuiltinMaterialVariableId>())
	{
		if(id == BuiltinMaterialVariableId::NONE)
		{
			continue;
		}

		if(BUILTIN_INFOS[id].m_name == name)
		{
			outId = id;

			if(BUILTIN_INFOS[id].m_type != dataType)
			{
				ANKI_RESOURCE_LOGE("Incorect type for builtin: %s", name.cstr());
				return Error::USER_DATA;
			}

			if(instanced && !BUILTIN_INFOS[id].m_instanced)
			{
				ANKI_RESOURCE_LOGE("Variable %s be instanced: %s",
								   (BUILTIN_INFOS[id].m_instanced) ? "should" : "shouldn't", name.cstr());
				return Error::USER_DATA;
			}

			break;
		}
	}

	if(outId == BuiltinMaterialVariableId::NONE && (name.find("m_anki") == 0 || name.find("u_anki") == 0))
	{
		ANKI_RESOURCE_LOGE("Unknown builtin var: %s", name.cstr());
		return Error::USER_DATA;
	}

	return Error::NONE;
}

// This is some trickery to select calling between XmlElement::getAttributeNumber and XmlElement::getAttributeNumbers
namespace
{

template<typename T>
class IsShaderVarDataTypeAnArray
{
public:
	static constexpr Bool VALUE = false;
};

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	template<> \
	class IsShaderVarDataTypeAnArray<type> \
	{ \
	public: \
		static constexpr Bool VALUE = rowCount * columnCount > 1; \
	};
#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

template<typename T, Bool isArray = IsShaderVarDataTypeAnArray<T>::VALUE>
class GetAttribute
{
public:
	Error operator()(const XmlElement& el, T& out)
	{
		return el.getAttributeNumbers("value", out);
	}
};

template<typename T>
class GetAttribute<T, false>
{
public:
	Error operator()(const XmlElement& el, T& out)
	{
		return el.getAttributeNumber("value", out);
	}
};

} // namespace

class GpuMaterialTexture
{
public:
	const char* m_name;
	U32 m_textureSlot;
};

static const Array<GpuMaterialTexture, TEXTURE_CHANNEL_COUNT> GPU_MATERIAL_TEXTURES = {
	{{"TEXTURE_CHANNEL_DIFFUSE", TEXTURE_CHANNEL_DIFFUSE},
	 {"TEXTURE_CHANNEL_NORMAL", TEXTURE_CHANNEL_NORMAL},
	 {"TEXTURE_CHANNEL_ROUGHNESS_METALNESS", TEXTURE_CHANNEL_ROUGHNESS_METALNESS},
	 {"TEXTURE_CHANNEL_EMISSION", TEXTURE_CHANNEL_EMISSION},
	 {"TEXTURE_CHANNEL_HEIGHT", TEXTURE_CHANNEL_HEIGHT},
	 {"TEXTURE_CHANNEL_AUX_0", TEXTURE_CHANNEL_AUX_0},
	 {"TEXTURE_CHANNEL_AUX_1", TEXTURE_CHANNEL_AUX_1},
	 {"TEXTURE_CHANNEL_AUX_2", TEXTURE_CHANNEL_AUX_2}}};

class GpuMaterialFloats
{
public:
	const char* m_name;
	U32 m_offsetof;
	U32 m_floatCount;
};

static const Array<GpuMaterialFloats, 5> GPU_MATERIAL_FLOATS = {
	{{"diffuseColor", offsetof(MaterialGpuDescriptor, m_diffuseColor), 3},
	 {"specularColor", offsetof(MaterialGpuDescriptor, m_specularColor), 3},
	 {"emissiveColor", offsetof(MaterialGpuDescriptor, m_emissiveColor), 3},
	 {"roughness", offsetof(MaterialGpuDescriptor, m_roughness), 1},
	 {"metalness", offsetof(MaterialGpuDescriptor, m_metalness), 1}}};

MaterialVariable::MaterialVariable()
{
	m_Mat4 = Mat4::getZero();
}

MaterialVariable::~MaterialVariable()
{
}

MaterialResource::MaterialResource(ResourceManager* manager)
	: ResourceObject(manager)
{
	memset(&m_materialGpuDescriptor, 0, sizeof(m_materialGpuDescriptor));
	memset(&m_rtShaderGroupHandleIndices, 0xFF, sizeof(m_rtShaderGroupHandleIndices));
}

MaterialResource::~MaterialResource()
{
	for(Pass p : EnumIterable<Pass>())
	{
		for(U32 l = 0; l < MAX_LOD_COUNT; ++l)
		{
			for(U32 inst = 0; inst < MAX_INSTANCE_GROUPS; ++inst)
			{
				for(U32 skinned = 0; skinned <= 1; ++skinned)
				{
					for(U32 vel = 0; vel <= 1; ++vel)
					{
						MaterialVariant& variant = m_variantMatrix[p][l][inst][skinned][vel];
						variant.m_blockInfos.destroy(getAllocator());
						variant.m_opaqueBindings.destroy(getAllocator());
					}
				}
			}
		}
	}

	for(MaterialVariable& var : m_vars)
	{
		var.m_name.destroy(getAllocator());
	}
	m_vars.destroy(getAllocator());

	m_nonBuiltinsMutation.destroy(getAllocator());
}

Error MaterialResource::load(const ResourceFilename& filename, Bool async)
{
	XmlDocument doc;
	XmlElement el;
	Bool present = false;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <material>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("material", rootEl));

	// shaderProgram
	CString fname;
	ANKI_CHECK(rootEl.getAttributeText("shaderProgram", fname));
	ANKI_CHECK(getManager().loadResource(fname, m_prog, async));

	// Good time to create the vars
	ANKI_CHECK(createVars());

	// shadow
	ANKI_CHECK(rootEl.getAttributeNumberOptional("shadow", m_shadow, present));
	m_shadow = m_shadow != 0;

	// forwardShading
	ANKI_CHECK(rootEl.getAttributeNumberOptional("forwardShading", m_forwardShading, present));
	m_forwardShading = m_forwardShading != 0;

	// <mutation>
	XmlElement mutatorsEl;
	ANKI_CHECK(rootEl.getChildElementOptional("mutation", mutatorsEl));
	if(mutatorsEl)
	{
		ANKI_CHECK(parseMutators(mutatorsEl));
	}

	// The rest of the mutators
	ANKI_CHECK(findBuiltinMutators());

	// <inputs>
	ANKI_CHECK(rootEl.getChildElementOptional("inputs", el));
	if(el)
	{
		ANKI_CHECK(parseInputs(el, async));
	}

	// <rtMaterial>
	XmlElement rtMaterialEl;
	ANKI_CHECK(doc.getChildElementOptional("rtMaterial", rtMaterialEl));
	if(rtMaterialEl && getManager().getGrManager().getDeviceCapabilities().m_rayTracingEnabled)
	{
		ANKI_CHECK(parseRtMaterial(rtMaterialEl));
	}

	return Error::NONE;
}

Error MaterialResource::parseMutators(XmlElement mutatorsEl)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	//
	// Process the non-builtin mutators
	//
	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);
	m_nonBuiltinsMutation.create(getAllocator(), mutatorCount);
	mutatorCount = 0;

	do
	{
		SubMutation& smutation = m_nonBuiltinsMutation[mutatorCount];

		// name
		CString mutatorName;
		ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
		if(mutatorName.isEmpty())
		{
			ANKI_RESOURCE_LOGE("Mutator name is empty");
			return Error::USER_DATA;
		}

		for(BuiltinMutatorId id : EnumIterable<BuiltinMutatorId>())
		{
			if(id == BuiltinMutatorId::NONE)
			{
				continue;
			}

			if(mutatorName == BUILTIN_MUTATOR_NAMES[id])
			{
				ANKI_RESOURCE_LOGE("Cannot list builtin mutator: %s", mutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		if(mutatorName.find("ANKI_") == 0)
		{
			ANKI_RESOURCE_LOGE("Mutators can't start with ANKI_: %s", mutatorName.cstr());
			return Error::USER_DATA;
		}

		// value
		ANKI_CHECK(mutatorEl.getAttributeNumber("value", smutation.m_value));

		// Find mutator
		smutation.m_mutator = m_prog->tryFindMutator(mutatorName);

		if(!smutation.m_mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::USER_DATA;
		}

		if(!smutation.m_mutator->valueExists(smutation.m_value))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", smutation.m_value, &mutatorName[0]);
			return Error::USER_DATA;
		}

		// Advance
		++mutatorCount;
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	ANKI_ASSERT(mutatorCount == m_nonBuiltinsMutation.getSize());

	return Error::NONE;
}

Error MaterialResource::findBuiltinMutators()
{
	// INSTANCE_COUNT
	U builtinMutatorCount = 0;

	m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT] =
		m_prog->tryFindMutator(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::INSTANCE_COUNT]);
	if(m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT])
	{
		if(m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT]->m_values.getSize() != MAX_INSTANCE_GROUPS)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have %u values in the program",
							   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::INSTANCE_COUNT].cstr(), MAX_INSTANCE_GROUPS);
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < MAX_INSTANCE_GROUPS; ++i)
		{
			if(m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT]->m_values[i] != (1 << i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::INSTANCE_COUNT].cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	// PASS
	m_builtinMutators[BuiltinMutatorId::PASS] = m_prog->tryFindMutator(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::PASS]);
	if(m_builtinMutators[BuiltinMutatorId::PASS] && m_forwardShading)
	{
		ANKI_RESOURCE_LOGE("Mutator is not required for forward shading: %s",
						   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::PASS].cstr());
		return Error::USER_DATA;
	}
	else if(!m_builtinMutators[BuiltinMutatorId::PASS] && !m_forwardShading)
	{
		ANKI_RESOURCE_LOGE("Mutator is required for opaque shading: %s",
						   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::PASS].cstr());
		return Error::USER_DATA;
	}

	if(m_builtinMutators[BuiltinMutatorId::PASS])
	{
		if(m_builtinMutators[BuiltinMutatorId::PASS]->m_values.getSize() != U32(Pass::COUNT) - 1)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have %u values in the program",
							   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::PASS].cstr(), U32(Pass::COUNT) - 1);
			return Error::USER_DATA;
		}

		U32 count = 0;
		for(Pass p : EnumIterable<Pass>())
		{
			if(p == Pass::FS)
			{
				continue;
			}

			if(m_builtinMutators[BuiltinMutatorId::PASS]->m_values[count++] != I(p))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::PASS].cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	if(!m_forwardShading && !m_builtinMutators[BuiltinMutatorId::PASS])
	{
		ANKI_RESOURCE_LOGE("%s mutator is required", BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::PASS].cstr());
		return Error::USER_DATA;
	}

	// LOD
	m_builtinMutators[BuiltinMutatorId::LOD] = m_prog->tryFindMutator(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::LOD]);
	if(m_builtinMutators[BuiltinMutatorId::LOD])
	{
		if(m_builtinMutators[BuiltinMutatorId::LOD]->m_values.getSize() > MAX_LOD_COUNT)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have at least %u values in the program",
							   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::LOD].cstr(), U32(MAX_LOD_COUNT));
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < m_builtinMutators[BuiltinMutatorId::LOD]->m_values.getSize(); ++i)
		{
			if(m_builtinMutators[BuiltinMutatorId::LOD]->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::LOD].cstr());
				return Error::USER_DATA;
			}
		}

		m_lodCount = U8(m_builtinMutators[BuiltinMutatorId::LOD]->m_values.getSize());
		++builtinMutatorCount;
	}

	// BONES
	m_builtinMutators[BuiltinMutatorId::BONES] = m_prog->tryFindMutator(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES]);
	if(m_builtinMutators[BuiltinMutatorId::BONES])
	{
		if(m_builtinMutators[BuiltinMutatorId::BONES]->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program",
							   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES].cstr());
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < m_builtinMutators[BuiltinMutatorId::BONES]->m_values.getSize(); ++i)
		{
			if(m_builtinMutators[BuiltinMutatorId::BONES]->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES].cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;

		// Find the binding of the transforms
		const ShaderProgramBinary& binary = m_prog->getBinary();
		for(const ShaderProgramBinaryBlock& block : binary.m_storageBlocks)
		{
			if(block.m_name.getBegin() == CString("b_ankiBoneTransforms"))
			{
				if(block.m_set != m_descriptorSetIdx)
				{
					ANKI_RESOURCE_LOGE("The set of b_ankiBoneTransforms should be %u", m_descriptorSetIdx);
					return Error::USER_DATA;
				}

				m_boneTrfsBinding = block.m_binding;
			}
			else if(block.m_name.getBegin() == CString("b_ankiPrevFrameBoneTransforms"))
			{
				if(block.m_set != m_descriptorSetIdx)
				{
					ANKI_RESOURCE_LOGE("The set of b_ankiPrevFrameBoneTransforms should be %u", m_descriptorSetIdx);
					return Error::USER_DATA;
				}

				m_prevFrameBoneTrfsBinding = block.m_binding;
			}
		}

		if(m_boneTrfsBinding == MAX_U32 || m_prevFrameBoneTrfsBinding == MAX_U32)
		{
			ANKI_RESOURCE_LOGE("The program is using the %s mutator but b_ankiBoneTransforms or "
							   "b_ankiPrevFrameBoneTransforms was not found",
							   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES].cstr());
			return Error::NONE;
		}
	}

	// VELOCITY
	m_builtinMutators[BuiltinMutatorId::VELOCITY] =
		m_prog->tryFindMutator(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::VELOCITY]);
	if(m_builtinMutators[BuiltinMutatorId::VELOCITY])
	{
		if(m_builtinMutators[BuiltinMutatorId::VELOCITY]->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program",
							   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::VELOCITY].cstr());
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < m_builtinMutators[BuiltinMutatorId::VELOCITY]->m_values.getSize(); ++i)
		{
			if(m_builtinMutators[BuiltinMutatorId::VELOCITY]->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::VELOCITY].cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	if(m_nonBuiltinsMutation.getSize() + builtinMutatorCount != m_prog->getMutators().getSize())
	{
		ANKI_RESOURCE_LOGE("Some mutatators are unacounted for");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error MaterialResource::parseVariable(CString fullVarName, Bool& instanced, U32& idx, CString& name)
{
	idx = 0;

	if(fullVarName.find("u_ankiPerDraw") != CString::NPOS)
	{
		instanced = false;
	}
	else if(fullVarName.find("u_ankiPerInstance") != CString::NPOS)
	{
		instanced = true;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Wrong variable name: %s", fullVarName.cstr());
		return Error::USER_DATA;
	}

	const PtrSize leftBracket = fullVarName.find("[");
	const PtrSize rightBracket = fullVarName.find("]");

	if(instanced)
	{
		const Bool correct =
			(leftBracket == CString::NPOS && rightBracket == CString::NPOS)
			|| (leftBracket != CString::NPOS && rightBracket != CString::NPOS && rightBracket > leftBracket);
		if(!correct)
		{
			ANKI_RESOURCE_LOGE("Wrong variable name: %s", fullVarName.cstr());
			return Error::USER_DATA;
		}

		if(leftBracket != CString::NPOS)
		{
			Array<char, 8> idxStr = {};
			for(PtrSize i = leftBracket + 1; i < rightBracket; ++i)
			{
				idxStr[i - (leftBracket + 1)] = fullVarName[i];
			}

			ANKI_CHECK(CString(idxStr.getBegin()).toNumber(idx));
		}
		else
		{
			idx = 0;
		}
	}
	else
	{
		if(leftBracket != CString::NPOS || rightBracket != CString::NPOS)
		{
			ANKI_RESOURCE_LOGE("Can't support non instanced array variables: %s", fullVarName.cstr());
			return Error::USER_DATA;
		}
	}

	const PtrSize dot = fullVarName.find(".");
	if(dot == CString::NPOS)
	{
		ANKI_RESOURCE_LOGE("Wrong variable name: %s", fullVarName.cstr());
		return Error::USER_DATA;
	}

	name = fullVarName.getBegin() + dot + 1;

	return Error::NONE;
}

Error MaterialResource::createVars()
{
	const ShaderProgramBinary& binary = m_prog->getBinary();

	// Create the uniform vars
	U32 descriptorSet = MAX_U32;
	U32 maxDescriptorSet = 0;
	for(const ShaderProgramBinaryBlock& block : binary.m_uniformBlocks)
	{
		maxDescriptorSet = max(maxDescriptorSet, block.m_set);

		if(block.m_name.getBegin() != CString("b_ankiMaterial"))
		{
			continue;
		}

		descriptorSet = block.m_set;
		m_uboIdx = U32(&block - binary.m_uniformBlocks.getBegin());
		m_uboBinding = block.m_binding;

		for(const ShaderProgramBinaryVariable& var : block.m_variables)
		{
			Bool instanced;
			U32 idx;
			CString name;
			ANKI_CHECK(parseVariable(var.m_name.getBegin(), instanced, idx, name));
			ANKI_ASSERT(name.getLength() > 0 && (instanced || idx == 0));

			if(idx > 0)
			{
				if(idx > MAX_INSTANCES)
				{
					ANKI_RESOURCE_LOGE("Array variable exceeds the instance count: %s", var.m_name.getBegin());
					return Error::USER_DATA;
				}

				if(idx == 1)
				{
					// Find the idx==0
					MaterialVariable* other = tryFindVariable(name);
					ANKI_ASSERT(other);
					ANKI_ASSERT(other->m_indexInBinary2ndElement == MAX_U32);
					other->m_indexInBinary2ndElement = U32(&var - block.m_variables.getBegin());
				}

				// Skip var
				continue;
			}

			const MaterialVariable* other = tryFindVariable(name);
			if(other)
			{
				ANKI_RESOURCE_LOGE("Variable found twice: %s", name.cstr());
				return Error::USER_DATA;
			}

			MaterialVariable& in = *m_vars.emplaceBack(getAllocator());
			in.m_name.create(getAllocator(), name);
			in.m_index = m_vars.getSize() - 1;
			in.m_indexInBinary = U32(&var - block.m_variables.getBegin());
			in.m_constant = false;
			in.m_instanced = instanced;
			in.m_dataType = var.m_type;

			// Check if it's builtin
			ANKI_CHECK(checkBuiltin(name, in.m_dataType, instanced, in.m_builtin));
		}
	}

	if(descriptorSet == MAX_U32)
	{
		ANKI_RESOURCE_LOGE("The b_ankiMaterial UBO is missing");
		return Error::USER_DATA;
	}

	// Continue with the opaque if it's a material shader program
	for(const ShaderProgramBinaryOpaque& o : binary.m_opaques)
	{
		maxDescriptorSet = max(maxDescriptorSet, o.m_set);

		if(o.m_set != descriptorSet)
		{
			continue;
		}

		MaterialVariable& in = *m_vars.emplaceBack(getAllocator());
		in.m_name.create(getAllocator(), o.m_name.getBegin());
		in.m_index = m_vars.getSize() - 1;
		in.m_indexInBinary = U32(&o - binary.m_opaques.getBegin());
		in.m_constant = false;
		in.m_instanced = false;
		in.m_dataType = o.m_type;

		// Check if it's builtin
		ANKI_CHECK(checkBuiltin(in.m_name, in.m_dataType, false, in.m_builtin));
	}

	if(descriptorSet != maxDescriptorSet)
	{
		ANKI_RESOURCE_LOGE("All bindings of a material shader should be in the highest descriptor set");
		return Error::USER_DATA;
	}

	m_descriptorSetIdx = U8(descriptorSet);

	// Consts
	for(const ShaderProgramResourceConstant& c : m_prog->getConstants())
	{
		MaterialVariable& in = *m_vars.emplaceBack(getAllocator());
		in.m_name.create(getAllocator(), c.m_name);
		in.m_index = m_vars.getSize() - 1;
		in.m_constant = true;
		in.m_instanced = false;
		in.m_dataType = c.m_dataType;
	}

	return Error::NONE;
}

Error MaterialResource::parseInputs(XmlElement inputsEl, Bool async)
{
	// Connect the input variables
	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElementOptional("input", inputEl));
	while(inputEl)
	{
		// Get var name
		CString varName;
		ANKI_CHECK(inputEl.getAttributeText("shaderVar", varName));

		// Try find var
		MaterialVariable* foundVar = tryFindVariable(varName);

		if(foundVar == nullptr)
		{
			ANKI_RESOURCE_LOGE("Variable \"%s\" not found", varName.cstr());
			return Error::USER_DATA;
		}

		if(foundVar->m_builtin != BuiltinMaterialVariableId::NONE)
		{
			ANKI_RESOURCE_LOGE("Shouldn't list builtin vars: %s", varName.cstr());
			return Error::USER_DATA;
		}

		// A value will be set
		foundVar->m_numericValueIsSet = true;

		// Process var
		if(foundVar->isConstant())
		{
			// Const

			switch(foundVar->getDataType())
			{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	case ShaderVariableDataType::capital: \
		ANKI_CHECK(GetAttribute<type>()(inputEl, foundVar->ANKI_CONCATENATE(m_, type))); \
		break;
#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

			default:
				ANKI_ASSERT(0);
				break;
			}
		}
		else
		{
			// Not built-in

			if(foundVar->isInstanced())
			{
				ANKI_RESOURCE_LOGE("Only some builtin variables can be instanced: %s", foundVar->getName().cstr());
				return Error::USER_DATA;
			}

			switch(foundVar->getDataType())
			{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	case ShaderVariableDataType::capital: \
		ANKI_CHECK(GetAttribute<type>()(inputEl, foundVar->ANKI_CONCATENATE(m_, type))); \
		break;
#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

			case ShaderVariableDataType::TEXTURE_2D:
			case ShaderVariableDataType::TEXTURE_2D_ARRAY:
			case ShaderVariableDataType::TEXTURE_3D:
			case ShaderVariableDataType::TEXTURE_CUBE:
			{
				CString texfname;
				ANKI_CHECK(inputEl.getAttributeText("value", texfname));
				ANKI_CHECK(getManager().loadResource(texfname, foundVar->m_tex, async));
				break;
			}

			default:
				ANKI_ASSERT(0);
				break;
			}
		}

		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	}

	return Error::NONE;
}

const MaterialVariant& MaterialResource::getOrCreateVariant(const RenderingKey& key_) const
{
	RenderingKey key = key_;
	key.setLod(min<U32>(m_lodCount - 1, key.getLod()));

	if(!isInstanced())
	{
		ANKI_ASSERT(key.getInstanceCount() == 1);
	}

	ANKI_ASSERT(!key.isSkinned() || m_builtinMutators[BuiltinMutatorId::BONES]);
	ANKI_ASSERT(!key.hasVelocity() || m_builtinMutators[BuiltinMutatorId::VELOCITY]);

	key.setInstanceCount(1 << getInstanceGroupIdx(key.getInstanceCount()));

	MaterialVariant& variant = m_variantMatrix[key.getPass()][key.getLod()][getInstanceGroupIdx(key.getInstanceCount())]
											  [key.isSkinned()][key.hasVelocity()];

	// Check if it's initialized
	{
		RLockGuard<RWMutex> lock(m_variantMatrixMtx);
		if(variant.m_prog.isCreated())
		{
			return variant;
		}
	}

	// Not initialized, init it
	WLockGuard<RWMutex> lock(m_variantMatrixMtx);

	// Check again
	if(variant.m_prog.isCreated())
	{
		return variant;
	}

	ShaderProgramResourceVariantInitInfo initInfo(m_prog);

	for(const SubMutation& m : m_nonBuiltinsMutation)
	{
		initInfo.addMutation(m.m_mutator->m_name, m.m_value);
	}

	if(m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT])
	{
		initInfo.addMutation(m_builtinMutators[BuiltinMutatorId::INSTANCE_COUNT]->m_name, key.getInstanceCount());
	}

	if(m_builtinMutators[BuiltinMutatorId::PASS])
	{
		initInfo.addMutation(m_builtinMutators[BuiltinMutatorId::PASS]->m_name, MutatorValue(key.getPass()));
	}

	if(m_builtinMutators[BuiltinMutatorId::LOD])
	{
		initInfo.addMutation(m_builtinMutators[BuiltinMutatorId::LOD]->m_name, MutatorValue(key.getLod()));
	}

	if(m_builtinMutators[BuiltinMutatorId::BONES])
	{
		initInfo.addMutation(m_builtinMutators[BuiltinMutatorId::BONES]->m_name, key.isSkinned() != 0);
	}

	if(m_builtinMutators[BuiltinMutatorId::VELOCITY])
	{
		initInfo.addMutation(m_builtinMutators[BuiltinMutatorId::VELOCITY]->m_name, key.hasVelocity() != 0);
	}

	for(const MaterialVariable& var : m_vars)
	{
		if(!var.isConstant())
		{
			continue;
		}

		if(!var.valueSetByMaterial())
		{
			continue;
		}

		switch(var.m_dataType)
		{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	case ShaderVariableDataType::capital: \
		initInfo.addConstant(var.getName(), var.getValue<type>()); \
		break;
#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

		default:
			ANKI_ASSERT(0);
		}
	}

	const ShaderProgramResourceVariant* progVariant;
	m_prog->getOrCreateVariant(initInfo, progVariant);

	// Init the variant
	initVariant(*progVariant, variant, key.getInstanceCount());

	return variant;
}

void MaterialResource::initVariant(const ShaderProgramResourceVariant& progVariant, MaterialVariant& variant,
								   U32 instanceCount) const
{
	// Find the block instance
	const ShaderProgramBinary& binary = m_prog->getBinary();
	const ShaderProgramBinaryVariant& binaryVariant = progVariant.getBinaryVariant();
	const ShaderProgramBinaryBlockInstance* binaryBlockInstance = nullptr;
	for(const ShaderProgramBinaryBlockInstance& instance : binaryVariant.m_uniformBlocks)
	{
		if(instance.m_index == m_uboIdx)
		{
			binaryBlockInstance = &instance;
			break;
		}
	}

	if(binaryBlockInstance == nullptr)
	{
		ANKI_RESOURCE_LOGF("The uniform block doesn't appear to be active for variant. Material: %s",
						   getFilename().cstr());
	}

	// Some init
	variant.m_prog = progVariant.getProgram();
	variant.m_blockInfos.create(getAllocator(), m_vars.getSize());
	variant.m_opaqueBindings.create(getAllocator(), m_vars.getSize(), -1);
	variant.m_uniBlockSize = binaryBlockInstance->m_size;
	ANKI_ASSERT(variant.m_uniBlockSize > 0);

	// Initialize the block infos, active vars and bindings
	for(const MaterialVariable& var : m_vars)
	{
		if(var.m_constant)
		{
			for(const ShaderProgramResourceConstant& c : m_prog->getConstants())
			{
				if(c.m_name == var.m_name)
				{
					variant.m_activeVars.set(var.m_index, progVariant.isConstantActive(c));
					break;
				}
			}
		}
		else if(var.inBlock())
		{
			for(const ShaderProgramBinaryVariableInstance& instance : binaryBlockInstance->m_variables)
			{
				if(instance.m_index == var.m_indexInBinary)
				{
					variant.m_activeVars.set(var.m_index, true);
					variant.m_blockInfos[var.m_index] = instance.m_blockInfo;

					if(var.m_instanced)
					{
						variant.m_blockInfos[var.m_index].m_arraySize = I16(instanceCount);
					}
					else
					{
						break;
					}
				}
				else if(instance.m_index == var.m_indexInBinary2ndElement)
				{
					// Then we need to update the stride
					ANKI_ASSERT(variant.m_blockInfos[var.m_index].m_offset >= 0);

					const I16 stride = I16(instance.m_blockInfo.m_offset - variant.m_blockInfos[var.m_index].m_offset);
					ANKI_ASSERT(stride >= 4);
					variant.m_blockInfos[var.m_index].m_arrayStride = stride;
				}
			}
		}
		else
		{
			ANKI_ASSERT(var.isSampler() || var.isTexture());
			for(const ShaderProgramBinaryOpaqueInstance& instance : binaryVariant.m_opaques)
			{
				if(instance.m_index == var.m_indexInBinary)
				{
					variant.m_activeVars.set(var.m_index, true);
					variant.m_opaqueBindings[var.m_index] = I16(binary.m_opaques[instance.m_index].m_binding);
					break;
				}
			}
		}
	}

	// No make sure that the vars that are active
	for(const MaterialVariable& var : m_vars)
	{
		if(var.m_builtin == BuiltinMaterialVariableId::NONE && variant.m_activeVars.get(var.m_index)
		   && !var.valueSetByMaterial())
		{
			ANKI_RESOURCE_LOGF("An active variable doesn't have its value set by the material: %s", var.m_name.cstr());
		}

		ANKI_ASSERT(!(var.m_instanced && var.m_indexInBinary2ndElement == MAX_U32));
	}

// Debug print
#if 0
	ANKI_RESOURCE_LOGI("binary variant idx %u\n", U32(&binaryVariant - binary.m_variants.getBegin()));
	for(const MaterialVariable& var : m_vars)
	{

		ANKI_RESOURCE_LOGI(
			"Var %s %s\n", var.m_name.cstr(), variant.m_activeVars.get(var.m_index) ? "active" : "inactive");
		if(var.inBlock() && variant.m_activeVars.get(var.m_index))
		{
			const ShaderVariableBlockInfo& inf = variant.m_blockInfos[var.m_index];
			ANKI_RESOURCE_LOGI(
				"\tblockInfo %d,%d,%d,%d\n", inf.m_offset, inf.m_arraySize, inf.m_arrayStride, inf.m_matrixStride);
		}
	}
#endif
}

U32 MaterialResource::getInstanceGroupIdx(U32 instanceCount)
{
	ANKI_ASSERT(instanceCount > 0);
	instanceCount = nextPowerOfTwo(instanceCount);
	ANKI_ASSERT(instanceCount <= MAX_INSTANCES);
	return U32(std::log2(F32(instanceCount)));
}

Error MaterialResource::parseRtMaterial(XmlElement rtMaterialEl)
{
	// rayType
	XmlElement rayTypeEl;
	ANKI_CHECK(rtMaterialEl.getChildElement("rayType", rayTypeEl));
	do
	{
		// type
		CString typeStr;
		ANKI_CHECK(rayTypeEl.getAttributeText("type", typeStr));
		RayType type = RayType::COUNT;
		if(typeStr == "shadows")
		{
			type = RayType::SHADOWS;
		}
		else if(typeStr == "gi")
		{
			type = RayType::GI;
		}
		else if(typeStr == "reflections")
		{
			type = RayType::REFLECTIONS;
		}
		else if(typeStr == "pathTracing")
		{
			type = RayType::PATH_TRACING;
		}
		else
		{
			ANKI_RESOURCE_LOGE("Uknown ray tracing type: %s", typeStr.cstr());
			return Error::USER_DATA;
		}

		if(m_rtPrograms[type].isCreated())
		{
			ANKI_RESOURCE_LOGE("Ray tracing type already set: %s", typeStr.cstr());
			return Error::USER_DATA;
		}

		m_rayTypes |= RayTypeBit(1 << type);

		// shaderProgram
		CString fname;
		ANKI_CHECK(rayTypeEl.getAttributeText("shaderProgram", fname));
		ANKI_CHECK(getManager().loadResource(fname, m_rtPrograms[type], false));

		// mutation
		XmlElement mutationEl;
		ANKI_CHECK(rayTypeEl.getChildElementOptional("mutation", mutationEl));
		DynamicArrayAuto<SubMutation> mutatorValues(getTempAllocator());
		if(mutationEl)
		{
			XmlElement mutatorEl;
			ANKI_CHECK(mutationEl.getChildElement("mutator", mutatorEl));
			U32 mutatorCount = 0;
			ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
			++mutatorCount;

			mutatorValues.resize(mutatorCount);

			mutatorCount = 0;
			do
			{
				// name
				CString mutatorName;
				ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
				if(mutatorName.isEmpty())
				{
					ANKI_RESOURCE_LOGE("Mutator name is empty");
					return Error::USER_DATA;
				}

				// value
				MutatorValue mutatorValue;
				ANKI_CHECK(mutatorEl.getAttributeNumber("value", mutatorValue));

				// Check
				const ShaderProgramResourceMutator* mutatorPtr = m_rtPrograms[type]->tryFindMutator(mutatorName);
				if(mutatorPtr == nullptr)
				{
					ANKI_RESOURCE_LOGE("Mutator not found: %s", mutatorName.cstr());
					return Error::USER_DATA;
				}

				if(!mutatorPtr->valueExists(mutatorValue))
				{
					ANKI_RESOURCE_LOGE("Mutator value doesn't exist: %s", mutatorName.cstr());
					return Error::USER_DATA;
				}

				// All good
				mutatorValues[mutatorCount].m_mutator = mutatorPtr;
				mutatorValues[mutatorCount].m_value = mutatorValue;

				// Advance
				++mutatorCount;
				ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
			} while(mutatorEl);

			ANKI_ASSERT(mutatorCount == mutatorValues.getSize());
		}

		if(mutatorValues.getSize() != m_rtPrograms[type]->getMutators().getSize())
		{
			ANKI_RESOURCE_LOGE("Forgot to set all mutators on some RT mutation");
			return Error::USER_DATA;
		}

		// Get the shader group handle
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_rtPrograms[type]);
		for(const SubMutation& subMutation : mutatorValues)
		{
			variantInitInfo.addMutation(subMutation.m_mutator->m_name, subMutation.m_value);
		}

		const ShaderProgramResourceVariant* progVariant;
		m_rtPrograms[type]->getOrCreateVariant(variantInitInfo, progVariant);
		m_rtShaderGroupHandleIndices[type] = progVariant->getHitShaderGroupHandleIndex();

		// Advance
		ANKI_CHECK(rayTypeEl.getNextSiblingElement("rayType", rayTypeEl));
	} while(rayTypeEl);

	// input
	XmlElement inputsEl;
	ANKI_CHECK(rtMaterialEl.getChildElementOptional("inputs", inputsEl));
	if(inputsEl)
	{
		XmlElement inputEl;
		ANKI_CHECK(inputsEl.getChildElement("input", inputEl));

		do
		{
			// name
			CString inputName;
			ANKI_CHECK(inputEl.getAttributeText("name", inputName));

			// Check if texture
			Bool found = false;
			for(U32 i = 0; i < GPU_MATERIAL_TEXTURES.getSize(); ++i)
			{
				if(GPU_MATERIAL_TEXTURES[i].m_name == inputName)
				{
					// Found, load the texture

					CString fname;
					ANKI_CHECK(inputEl.getAttributeText("value", fname));

					const U32 textureIdx = GPU_MATERIAL_TEXTURES[i].m_textureSlot;
					ANKI_CHECK(getManager().loadResource(fname, m_textureResources[textureIdx], false));

					m_textureViews[m_textureViewCount] = m_textureResources[textureIdx]->getGrTextureView();

					m_materialGpuDescriptor.m_bindlessTextureIndices[textureIdx] =
						U16(m_textureViews[m_textureViewCount]->getOrCreateBindlessTextureIndex());

					++m_textureViewCount;
					found = true;
					break;
				}
			}

			// Check floats
			if(!found)
			{
				for(U32 i = 0; i < GPU_MATERIAL_FLOATS.getSize(); ++i)
				{
					if(GPU_MATERIAL_FLOATS[i].m_name == inputName)
					{
						// Found it, set the value

						if(GPU_MATERIAL_FLOATS[i].m_floatCount == 3)
						{
							Vec3 val;
							ANKI_CHECK(inputEl.getAttributeNumbers("value", val));
							memcpy(reinterpret_cast<U8*>(&m_materialGpuDescriptor) + GPU_MATERIAL_FLOATS[i].m_offsetof,
								   &val, sizeof(val));
						}
						else
						{
							ANKI_ASSERT(GPU_MATERIAL_FLOATS[i].m_floatCount == 1);
							F32 val;
							ANKI_CHECK(inputEl.getAttributeNumber("value", val));
							memcpy(reinterpret_cast<U8*>(&m_materialGpuDescriptor) + GPU_MATERIAL_FLOATS[i].m_offsetof,
								   &val, sizeof(val));
						}

						found = true;
						break;
					}
				}
			}

			if(!found)
			{
				ANKI_RESOURCE_LOGE("Input name is incorrect: %s", inputName.cstr());
				return Error::USER_DATA;
			}

			// Advance
			ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
		} while(inputEl);
	}

	return Error::NONE;
}

} // end namespace anki
