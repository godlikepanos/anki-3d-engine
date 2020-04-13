// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MaterialResource2.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/TextureResource.h>
#include <anki/util/Xml.h>

namespace anki
{

class BuiltinVarInfo
{
public:
	const char* m_name;
	ShaderVariableDataType m_type;
	Bool m_instanced;
};

static const Array<BuiltinVarInfo, U(BuiltinMaterialVariableId2::COUNT)> BUILTIN_INFOS = {
	{{"NONE", ShaderVariableDataType::NONE, false},
		{"m_ankiMvp", ShaderVariableDataType::MAT4, true},
		{"m_ankiViewMatrix", ShaderVariableDataType::MAT4, true},
		{"m_ankiModelMatrix", ShaderVariableDataType::MAT4, true},
		{"m_ankiProjectionMatrix", ShaderVariableDataType::MAT4, false},
		{"m_ankiViewMatrix", ShaderVariableDataType::MAT4, false},
		{"m_ankiNormalMatrix", ShaderVariableDataType::MAT3, true},
		{"m_ankiRotationMatrix", ShaderVariableDataType::MAT3, true},
		{"m_ankiCameraRotationMatrix", ShaderVariableDataType::MAT3, false},
		{"m_ankiCameraPosition", ShaderVariableDataType::VEC3, false},
		{"m_ankiPreviousMvp", ShaderVariableDataType::MAT4, true},
		{"m_ankiGlobalSampler", ShaderVariableDataType::SAMPLER, false}}};

MaterialVariable2::MaterialVariable2()
{
	m_mat4 = Mat4::getZero();
	m_mat4(3, 3) = NO_VALUE; // Add a random value
}

MaterialVariable2::~MaterialVariable2()
{
}

MaterialResource2::MaterialResource2(ResourceManager* manager)
	: ResourceObject(manager)
{
}

MaterialResource2::~MaterialResource2()
{
	// TODO
}

Error MaterialResource2::load(const ResourceFilename& filename, Bool async)
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

	// <mutators>
	XmlElement mutatorsEl;
	ANKI_CHECK(rootEl.getChildElementOptional("mutators", mutatorsEl));
	if(mutatorsEl)
	{
		ANKI_CHECK(parseMutators(mutatorsEl));
	}

	// <inputs>
	ANKI_CHECK(rootEl.getChildElementOptional("inputs", el));
	if(el)
	{
		ANKI_CHECK(parseInputs(el, async));
	}

	// Check that all non-builtin vars have a value
	for(const MaterialVariable2& var : m_vars)
	{
		if(!var.isBuildin() && !var.valueSetByMaterial())
		{
			ANKI_RESOURCE_LOGE("Missing value for variable %s", var.m_name.cstr());
			return Error::USER_DATA;
		}
	}

	return Error::NONE;
}

Error MaterialResource2::parseMutators(XmlElement mutatorsEl)
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

		if(mutatorName == "ANKI_INSTANCE_COUNT" || mutatorName == "ANKI_PASS" || mutatorName == "ANKI_LOD"
			|| mutatorName == "ANKI_BONES" || mutatorName == "ANKI_VELOCITY")
		{
			ANKI_RESOURCE_LOGE("Cannot list builtin mutator \"%s\"", &mutatorName[0]);
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

	//
	// Find the builtin mutators
	//
	U builtinMutatorCount = 0;

	m_instancingMutator = m_prog->tryFindMutator("ANKI_INSTANCE_COUNT");
	if(m_instancingMutator)
	{
		if(m_instancingMutator->m_values.getSize() != MAX_INSTANCE_GROUPS)
		{
			ANKI_RESOURCE_LOGE("Mutator ANKI_INSTANCE_COUNT should have %u values in the program", MAX_INSTANCE_GROUPS);
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < MAX_INSTANCE_GROUPS; ++i)
		{
			if(m_instancingMutator->m_values[i] != (1 << i))
			{
				ANKI_RESOURCE_LOGE("Values of the ANKI_INSTANCE_COUNT mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	m_passMutator = m_prog->tryFindMutator("ANKI_PASS");
	if(m_passMutator)
	{
		if(m_passMutator->m_values.getSize() != U32(Pass::COUNT))
		{
			ANKI_RESOURCE_LOGE("Mutator ANKI_PASS should have %u values in the program", U(Pass::COUNT));
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < U(Pass::COUNT); ++i)
		{
			if(m_passMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the ANKI_PASS mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	if(!m_forwardShading && !m_passMutator)
	{
		ANKI_RESOURCE_LOGE("ANKI_PASS mutator is required");
		return Error::USER_DATA;
	}

	m_lodMutator = m_prog->tryFindMutator("ANKI_LOD");
	if(m_lodMutator)
	{
		if(m_lodMutator->m_values.getSize() > MAX_LOD_COUNT)
		{
			ANKI_RESOURCE_LOGE("Mutator ANKI_LOD should have at least %u values in the program", U(MAX_LOD_COUNT));
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < m_lodMutator->m_values.getSize(); ++i)
		{
			if(m_lodMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the ANKI_LOD mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		m_lodCount = U8(m_lodMutator->m_values.getSize());
		++builtinMutatorCount;
	}

	m_bonesMutator = m_prog->tryFindMutator("ANKI_BONES");
	if(m_bonesMutator)
	{
		if(m_bonesMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator ANKI_BONES should have 2 values in the program");
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < m_bonesMutator->m_values.getSize(); ++i)
		{
			if(m_bonesMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the BONES mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	m_velocityMutator = m_prog->tryFindMutator("ANKI_VELOCITY");
	if(m_velocityMutator)
	{
		if(m_velocityMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator ANKI_VELOCITY should have 2 values in the program");
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < m_velocityMutator->m_values.getSize(); ++i)
		{
			if(m_velocityMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the ANKI_VELOCITY mutator in the program are not the expected");
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

Error MaterialResource2::parseVariable(CString fullVarName, Bool& instanced, U32& idx, CString& name)
{
	if(fullVarName.find("u_ankiPerDraw") == CString::NPOS)
	{
		instanced = false;
	}
	else if(fullVarName.find("u_ankiPerInstance") == CString::NPOS)
	{
		instanced = true;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Wrong variable name: %s", fullVarName.cstr());
		return Error::USER_DATA;
	}

	if(instanced)
	{
		const PtrSize leftBracket = fullVarName.find("[");
		const PtrSize rightBracket = fullVarName.find("]");

		if(leftBracket == CString::NPOS || rightBracket == CString::NPOS || rightBracket <= leftBracket)
		{
			ANKI_RESOURCE_LOGE("Wrong variable name: %s", fullVarName.cstr());
			return Error::USER_DATA;
		}

		Array<char, 8> idxStr;
		ANKI_ASSERT(rightBracket - leftBracket < idxStr.getSize() - 1);
		for(PtrSize i = leftBracket; i <= rightBracket; ++i)
		{
			idxStr[i - leftBracket] = fullVarName[i];
		}
		idxStr[rightBracket - leftBracket + 1] = '\0';

		ANKI_CHECK(CString(idxStr.getBegin()).toNumber(idx));
	}

	const PtrSize dot = fullVarName.find(".");
	if(dot == CString::NPOS)
	{
		ANKI_RESOURCE_LOGE("Wrong variable name: %s", fullVarName.cstr());
		return Error::USER_DATA;
	}

	name = fullVarName.getBegin() + dot;

	return Error::NONE;
}

Error MaterialResource2::createVars()
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

		for(const ShaderProgramBinaryVariable& var : block.m_variables)
		{
			Bool instanced;
			U32 idx;
			CString name;
			ANKI_CHECK(parseVariable(var.m_name.getBegin(), instanced, idx, name));

			if(idx > 0)
			{
				continue;
			}

			MaterialVariable2& in = *m_vars.emplaceBack(getAllocator());
			in.m_name.create(getAllocator(), name);
			in.m_index = m_vars.getSize() - 1;
			in.m_indexInBinary = U32(&var - block.m_variables.getBegin());
			in.m_constant = false;
			in.m_instanced = instanced;
			in.m_dataType = var.m_type;

			// Check if it's builtin
			for(BuiltinMaterialVariableId2 id : EnumIterable<BuiltinMaterialVariableId2>())
			{
				if(id == BuiltinMaterialVariableId2::NONE)
				{
					continue;
				}

				if(BUILTIN_INFOS[id].m_name == name)
				{
					in.m_builtin = id;

					if(BUILTIN_INFOS[id].m_type != in.m_dataType)
					{
						ANKI_RESOURCE_LOGE("Incorect type for builtin: %s", name.cstr());
						return Error::USER_DATA;
					}

					if(BUILTIN_INFOS[id].m_instanced == instanced)
					{
						ANKI_RESOURCE_LOGE("Variable %s be instanced: %s",
							(BUILTIN_INFOS[id].m_instanced) ? "should" : "shouldn't",
							name.cstr());
						return Error::USER_DATA;
					}

					break;
				}
			}
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

		MaterialVariable2& in = *m_vars.emplaceBack(getAllocator());
		in.m_name.create(getAllocator(), o.m_name.getBegin());
		in.m_index = m_vars.getSize() - 1;
		in.m_indexInBinary = U32(&o - binary.m_opaques.getBegin());
		in.m_constant = false;
		in.m_instanced = false;
		in.m_dataType = o.m_type;
	}

	if(descriptorSet != maxDescriptorSet)
	{
		ANKI_RESOURCE_LOGE("All bindings of a material shader should be in the highest descriptor set");
		return Error::USER_DATA;
	}

	m_descriptorSetIdx = U8(descriptorSet);

	// Consts
	for(const ShaderProgramResourceConstant2& c : m_prog->getConstants())
	{
		MaterialVariable2& in = *m_vars.emplaceBack(getAllocator());
		in.m_name.create(getAllocator(), c.m_name);
		in.m_index = m_vars.getSize() - 1;
		in.m_constant = true;
		in.m_instanced = false;
		in.m_dataType = c.m_dataType;
	}

	return Error::NONE;
}

Error MaterialResource2::parseInputs(XmlElement inputsEl, Bool async)
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
		MaterialVariable2* foundVar = nullptr;
		for(MaterialVariable2& v : m_vars)
		{
			if(v.m_name == varName)
			{
				foundVar = &v;
				break;
			}
		}

		if(foundVar == nullptr)
		{
			ANKI_RESOURCE_LOGE("Variable \"%s\" not found", varName.cstr());
			return Error::USER_DATA;
		}

		if(foundVar->m_builtin == BuiltinMaterialVariableId2::NONE)
		{
			ANKI_RESOURCE_LOGE("Shouldn't list builtin vars: %s", varName.cstr());
			return Error::USER_DATA;
		}

		// Process var
		if(foundVar->isConstant())
		{
			// Const

			switch(foundVar->getDataType())
			{
			case ShaderVariableDataType::INT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", foundVar->m_int));
				break;
			case ShaderVariableDataType::IVEC2:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_ivec2));
				break;
			case ShaderVariableDataType::IVEC3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_ivec3));
				break;
			case ShaderVariableDataType::IVEC4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_ivec4));
				break;
			case ShaderVariableDataType::UINT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", foundVar->m_uint));
				break;
			case ShaderVariableDataType::UVEC2:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_uvec2));
				break;
			case ShaderVariableDataType::UVEC3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_uvec3));
				break;
			case ShaderVariableDataType::UVEC4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_uvec4));
				break;
			case ShaderVariableDataType::FLOAT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", foundVar->m_float));
				break;
			case ShaderVariableDataType::VEC2:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_vec2));
				break;
			case ShaderVariableDataType::VEC3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_vec3));
				break;
			case ShaderVariableDataType::VEC4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_vec4));
				break;
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
			case ShaderVariableDataType::INT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", foundVar->m_int));
				break;
			case ShaderVariableDataType::IVEC2:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_ivec2));
				break;
			case ShaderVariableDataType::IVEC3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_ivec3));
				break;
			case ShaderVariableDataType::IVEC4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_ivec4));
				break;
			case ShaderVariableDataType::UINT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", foundVar->m_uint));
				break;
			case ShaderVariableDataType::UVEC2:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_uvec2));
				break;
			case ShaderVariableDataType::UVEC3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_uvec3));
				break;
			case ShaderVariableDataType::UVEC4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_uvec4));
				break;
			case ShaderVariableDataType::FLOAT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", foundVar->m_float));
				break;
			case ShaderVariableDataType::VEC2:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_vec2));
				break;
			case ShaderVariableDataType::VEC3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_vec3));
				break;
			case ShaderVariableDataType::VEC4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_vec4));
				break;
			case ShaderVariableDataType::MAT3:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_mat3));
				break;
			case ShaderVariableDataType::MAT4:
				ANKI_CHECK(inputEl.getAttributeNumbers("value", foundVar->m_mat4));
				break;
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

const MaterialVariant2& MaterialResource2::getOrCreateVariant(const RenderingKey& key_) const
{
	RenderingKey key = key_;
	key.setLod(min<U32>(m_lodCount - 1, key.getLod()));

	if(!isInstanced())
	{
		ANKI_ASSERT(key.getInstanceCount() == 1);
	}

	ANKI_ASSERT(!key.isSkinned() || m_bonesMutator);
	ANKI_ASSERT(!key.hasVelocity() || m_velocityMutator);

	key.setInstanceCount(1 << getInstanceGroupIdx(key.getInstanceCount()));

	MaterialVariant2& variant = m_variantMatrix[key.getPass()][key.getLod()][getInstanceGroupIdx(
		key.getInstanceCount())][key.isSkinned()][key.hasVelocity()];

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

	ShaderProgramResourceVariantInitInfo2 initInfo(m_prog);

	for(const SubMutation& m : m_nonBuiltinsMutation)
	{
		initInfo.addMutation(m.m_mutator->m_name, m.m_value);
	}

	if(m_instancingMutator)
	{
		initInfo.addMutation(m_instancingMutator->m_name, key.getInstanceCount());
	}

	if(m_passMutator)
	{
		initInfo.addMutation(m_passMutator->m_name, MutatorValue(key.getPass()));
	}

	if(m_lodMutator)
	{
		initInfo.addMutation(m_lodMutator->m_name, MutatorValue(key.getLod()));
	}

	if(m_bonesMutator)
	{
		initInfo.addMutation(m_bonesMutator->m_name, key.isSkinned() != 0);
	}

	if(m_velocityMutator)
	{
		initInfo.addMutation(m_velocityMutator->m_name, key.hasVelocity() != 0);
	}

	for(const MaterialVariable2& var : m_vars)
	{
		if(!var.isConstant())
		{
			continue;
		}

		switch(var.m_dataType)
		{
		case ShaderVariableDataType::INT:
			initInfo.addConstant(var.getName(), var.getValue<I32>());
			break;
		case ShaderVariableDataType::IVEC2:
			initInfo.addConstant(var.getName(), var.getValue<IVec2>());
			break;
		case ShaderVariableDataType::IVEC3:
			initInfo.addConstant(var.getName(), var.getValue<IVec3>());
			break;
		case ShaderVariableDataType::IVEC4:
			initInfo.addConstant(var.getName(), var.getValue<IVec4>());
			break;
		case ShaderVariableDataType::FLOAT:
			initInfo.addConstant(var.getName(), var.getValue<F32>());
			break;
		case ShaderVariableDataType::VEC2:
			initInfo.addConstant(var.getName(), var.getValue<Vec2>());
			break;
		case ShaderVariableDataType::VEC3:
			initInfo.addConstant(var.getName(), var.getValue<Vec3>());
			break;
		case ShaderVariableDataType::VEC4:
			initInfo.addConstant(var.getName(), var.getValue<Vec4>());
			break;
		default:
			ANKI_ASSERT(0);
		}
	}

	const ShaderProgramResourceVariant2* progVariant;
	m_prog->getOrCreateVariant(initInfo, progVariant);

	// Init the variant
	initVariant(*progVariant, variant);

	return variant;
}

void MaterialResource2::initVariant(const ShaderProgramResourceVariant2& progVariant, MaterialVariant2& variant) const
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
		ANKI_RESOURCE_LOGF(
			"The uniform block doesn't appear to be active for variant. Material: %s", getFilename().cstr());
	}

	// Some init
	variant.m_prog = progVariant.getProgram();
	variant.m_blockInfos.create(getAllocator(), m_vars.getSize());
	variant.m_opaqueBindings.create(getAllocator(), m_vars.getSize(), -1);
	variant.m_uniBlockSize = binaryBlockInstance->m_size;

	// Initialize the block infos, active vars and bindings
	for(const MaterialVariable2& var : m_vars)
	{
		if(var.m_constant)
		{
			for(const ShaderProgramResourceConstant2& c : m_prog->getConstants())
			{
				if(c.m_name == var.m_name)
				{
					variant.m_activeVars.set(var.m_index, progVariant.isConstantActive(c));
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
					break;
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
}

U32 MaterialResource2::getInstanceGroupIdx(U32 instanceCount)
{
	ANKI_ASSERT(instanceCount > 0);
	instanceCount = nextPowerOfTwo(instanceCount);
	ANKI_ASSERT(instanceCount <= MAX_INSTANCES);
	return U32(std::log2(F32(instanceCount)));
}

} // end namespace anki
