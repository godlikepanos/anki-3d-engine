// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MaterialResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/misc/Xml.h>

namespace anki
{

struct BuiltinVarInfo
{
	const char* m_name;
	ShaderVariableDataType m_type;
	Bool m_instanced;
};

static const Array<BuiltinVarInfo, U(BuiltinMaterialVariableId::COUNT) - 1> BUILTIN_INFOS = {
	{{"MODEL_VIEW_PROJECTION_MATRIX", ShaderVariableDataType::MAT4, true},
		{"MODEL_VIEW_MATRIX", ShaderVariableDataType::MAT4, true},
		{"MODEL_MATRIX", ShaderVariableDataType::MAT4, true},
		{"VIEW_PROJECTION_MATRIX", ShaderVariableDataType::MAT4, false},
		{"VIEW_MATRIX", ShaderVariableDataType::MAT4, false},
		{"NORMAL_MATRIX", ShaderVariableDataType::MAT3, true},
		{"ROTATION_MATRIX", ShaderVariableDataType::MAT3, true},
		{"CAMERA_ROTATION_MATRIX", ShaderVariableDataType::MAT3, false},
		{"CAMERA_POSITION", ShaderVariableDataType::VEC3, false},
		{"PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX", ShaderVariableDataType::MAT4, true}}};

MaterialVariable::MaterialVariable()
{
	m_mat4 = Mat4::getZero();
}

MaterialVariable::~MaterialVariable()
{
}

MaterialResource::MaterialResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

MaterialResource::~MaterialResource()
{
	m_vars.destroy(getAllocator());
	m_constValues.destroy(getAllocator());
	m_mutations.destroy(getAllocator());
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
	m_descriptorSetIdx = m_prog->getDescriptorSetIndex();

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

	return Error::NONE;
}

Error MaterialResource::parseMutators(XmlElement mutatorsEl)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);
	m_mutations.create(getAllocator(), mutatorCount);
	mutatorCount = 0;

	do
	{
		ShaderProgramResourceMutation& mutation = m_mutations[mutatorCount];

		// name
		CString mutatorName;
		ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
		if(mutatorName.isEmpty())
		{
			ANKI_RESOURCE_LOGE("Mutator name is empty");
			return Error::USER_DATA;
		}

		if(mutatorName == "INSTANCE_COUNT" || mutatorName == "PASS" || mutatorName == "LOD" || mutatorName == "BONES"
			|| mutatorName == "VELOCITY")
		{
			ANKI_RESOURCE_LOGE("Cannot list builtin mutator \"%s\"", &mutatorName[0]);
			return Error::USER_DATA;
		}

		// value
		ANKI_CHECK(mutatorEl.getAttributeNumber("value", mutation.m_value));

		// Find mutator
		mutation.m_mutator = m_prog->tryFindMutator(mutatorName);

		if(!mutation.m_mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::USER_DATA;
		}

		if(!mutation.m_mutator->valueExists(mutation.m_value))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", mutation.m_value, &mutatorName[0]);
			return Error::USER_DATA;
		}

		// Advance
		++mutatorCount;
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	// Find the builtin mutators
	U builtinMutatorCount = 0;

	m_instanceMutator = m_prog->getInstancingMutator();
	if(m_instanceMutator)
	{
		if(m_instanceMutator->getName() != "INSTANCE_COUNT")
		{
			ANKI_RESOURCE_LOGE("If program is instanced then the instance mutator should be the INSTANCE_COUNT");
			return Error::USER_DATA;
		}

		if(m_instanceMutator->getValues().getSize() != MAX_INSTANCE_GROUPS)
		{
			ANKI_RESOURCE_LOGE("Mutator INSTANCE_COUNT should have %u values in the program", MAX_INSTANCE_GROUPS);
			return Error::USER_DATA;
		}

		for(U i = 0; i < MAX_INSTANCE_GROUPS; ++i)
		{
			if(m_instanceMutator->getValues()[i] != (1 << i))
			{
				ANKI_RESOURCE_LOGE("Values of the INSTANCE_COUNT mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	m_passMutator = m_prog->tryFindMutator("PASS");
	if(m_passMutator)
	{
		if(m_passMutator->getValues().getSize() != U(Pass::COUNT))
		{
			ANKI_RESOURCE_LOGE("Mutator PASS should have %u values in the program", U(Pass::COUNT));
			return Error::USER_DATA;
		}

		for(U i = 0; i < U(Pass::COUNT); ++i)
		{
			if(m_passMutator->getValues()[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the PASS mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	if(!m_forwardShading && !m_passMutator)
	{
		ANKI_RESOURCE_LOGE("PASS mutator is required");
		return Error::USER_DATA;
	}

	m_lodMutator = m_prog->tryFindMutator("LOD");
	if(m_lodMutator)
	{
		if(m_lodMutator->getValues().getSize() > MAX_LOD_COUNT)
		{
			ANKI_RESOURCE_LOGE("Mutator LOD should have at least %u values in the program", U(MAX_LOD_COUNT));
			return Error::USER_DATA;
		}

		for(U i = 0; i < m_lodMutator->getValues().getSize(); ++i)
		{
			if(m_lodMutator->getValues()[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the LOD mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		m_lodCount = m_lodMutator->getValues().getSize();
		++builtinMutatorCount;
	}

	m_bonesMutator = m_prog->tryFindMutator("BONES");
	if(m_bonesMutator)
	{
		if(m_bonesMutator->getValues().getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator BONES should have 2 values in the program");
			return Error::USER_DATA;
		}

		for(U i = 0; i < m_bonesMutator->getValues().getSize(); ++i)
		{
			if(m_bonesMutator->getValues()[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the BONES mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	m_velocityMutator = m_prog->tryFindMutator("VELOCITY");
	if(m_velocityMutator)
	{
		if(m_velocityMutator->getValues().getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator VELOCITY should have 2 values in the program");
			return Error::USER_DATA;
		}

		for(U i = 0; i < m_velocityMutator->getValues().getSize(); ++i)
		{
			if(m_velocityMutator->getValues()[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the VELOCITY mutator in the program are not the expected");
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	if(m_mutations.getSize() + builtinMutatorCount != m_prog->getMutators().getSize())
	{
		ANKI_RESOURCE_LOGE("Some mutatators are unacounted for");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error MaterialResource::parseInputs(XmlElement inputsEl, Bool async)
{
	// Iterate the program's variables and get counts
	U constInputCount = 0;
	U nonConstInputCount = 0;
	for(const ShaderProgramResourceInputVariable& in : m_prog->getInputVariables())
	{
		if(!in.acceptAllMutations(m_mutations))
		{
			// Will not be used by this material at all
			continue;
		}

		if(in.isConstant())
		{
			++constInputCount;
		}
		else
		{
			++nonConstInputCount;
		}
	}

	m_vars.create(getAllocator(), nonConstInputCount);
	m_constValues.create(getAllocator(), constInputCount);

	// Connect the input variables
	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElementOptional("input", inputEl));
	while(inputEl)
	{
		// Get var name
		CString varName;
		ANKI_CHECK(inputEl.getAttributeText("shaderInput", varName));

		// Try find var
		const ShaderProgramResourceInputVariable* foundVar = m_prog->tryFindInputVariable(varName);
		if(foundVar == nullptr)
		{
			ANKI_RESOURCE_LOGE("Variable \"%s\" not found", &varName[0]);
			return Error::USER_DATA;
		}

		if(!foundVar->acceptAllMutations(m_mutations))
		{
			ANKI_RESOURCE_LOGE("Variable \"%s\" is not needed by the material's mutations");
			return Error::USER_DATA;
		}

		// Process var
		if(foundVar->isConstant())
		{
			// Const

			ShaderProgramResourceConstantValue& constVal = m_constValues[--constInputCount];
			constVal.m_variable = foundVar;

			switch(foundVar->getShaderVariableDataType())
			{
			case ShaderVariableDataType::INT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", constVal.m_int));
				break;
			case ShaderVariableDataType::IVEC2:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_ivec2));
				break;
			case ShaderVariableDataType::IVEC3:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_ivec3));
				break;
			case ShaderVariableDataType::IVEC4:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_ivec4));
				break;
			case ShaderVariableDataType::UINT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", constVal.m_uint));
				break;
			case ShaderVariableDataType::UVEC2:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_uvec2));
				break;
			case ShaderVariableDataType::UVEC3:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_uvec3));
				break;
			case ShaderVariableDataType::UVEC4:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_uvec4));
				break;
			case ShaderVariableDataType::FLOAT:
				ANKI_CHECK(inputEl.getAttributeNumber("value", constVal.m_float));
				break;
			case ShaderVariableDataType::VEC2:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_vec2));
				break;
			case ShaderVariableDataType::VEC3:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_vec3));
				break;
			case ShaderVariableDataType::VEC4:
				ANKI_CHECK(inputEl.getAttributeVector("value", constVal.m_vec4));
				break;
			default:
				ANKI_ASSERT(0);
				break;
			}
		}
		else
		{
			// Builtin or not

			Bool builtinPresent = false;
			CString builtinStr;
			ANKI_CHECK(inputEl.getAttributeTextOptional("builtin", builtinStr, builtinPresent));
			if(builtinPresent)
			{
				// Builtin

				U i;
				for(i = 0; i < BUILTIN_INFOS.getSize(); ++i)
				{
					if(builtinStr == BUILTIN_INFOS[i].m_name)
					{
						break;
					}
				}

				if(i == BUILTIN_INFOS.getSize())
				{
					ANKI_RESOURCE_LOGE("Incorrect builtin variable: %s", &builtinStr[0]);
					return Error::USER_DATA;
				}

				if(BUILTIN_INFOS[i].m_type != foundVar->getShaderVariableDataType())
				{
					ANKI_RESOURCE_LOGE(
						"The type of the builtin variable in the shader program is not the correct one: %s",
						&builtinStr[0]);
					return Error::USER_DATA;
				}

				if(foundVar->isInstanced() && !BUILTIN_INFOS[i].m_instanced)
				{
					ANKI_RESOURCE_LOGE("Builtin variable %s cannot be instanced", BUILTIN_INFOS[i].m_name);
					return Error::USER_DATA;
				}

				MaterialVariable& mtlVar = m_vars[--nonConstInputCount];
				mtlVar.m_input = foundVar;
				mtlVar.m_builtin = BuiltinMaterialVariableId(i + 1);
			}
			else
			{
				// Not built-in

				if(foundVar->isInstanced())
				{
					ANKI_RESOURCE_LOGE("Only some builtin variables can be instanced: %s", &foundVar->getName()[0]);
					return Error::USER_DATA;
				}

				MaterialVariable& mtlVar = m_vars[--nonConstInputCount];
				mtlVar.m_input = foundVar;

				switch(foundVar->getShaderVariableDataType())
				{
				case ShaderVariableDataType::INT:
					ANKI_CHECK(inputEl.getAttributeNumber("value", mtlVar.m_int));
					break;
				case ShaderVariableDataType::IVEC2:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_ivec2));
					break;
				case ShaderVariableDataType::IVEC3:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_ivec3));
					break;
				case ShaderVariableDataType::IVEC4:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_ivec4));
					break;
				case ShaderVariableDataType::UINT:
					ANKI_CHECK(inputEl.getAttributeNumber("value", mtlVar.m_uint));
					break;
				case ShaderVariableDataType::UVEC2:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_uvec2));
					break;
				case ShaderVariableDataType::UVEC3:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_uvec3));
					break;
				case ShaderVariableDataType::UVEC4:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_uvec4));
					break;
				case ShaderVariableDataType::FLOAT:
					ANKI_CHECK(inputEl.getAttributeNumber("value", mtlVar.m_float));
					break;
				case ShaderVariableDataType::VEC2:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_vec2));
					break;
				case ShaderVariableDataType::VEC3:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_vec3));
					break;
				case ShaderVariableDataType::VEC4:
					ANKI_CHECK(inputEl.getAttributeVector("value", mtlVar.m_vec4));
					break;
				case ShaderVariableDataType::MAT3:
					ANKI_CHECK(inputEl.getAttributeMatrix("value", mtlVar.m_mat3));
					break;
				case ShaderVariableDataType::MAT4:
					ANKI_CHECK(inputEl.getAttributeMatrix("value", mtlVar.m_mat4));
					break;
				case ShaderVariableDataType::SAMPLER_2D:
				case ShaderVariableDataType::SAMPLER_2D_ARRAY:
				case ShaderVariableDataType::SAMPLER_3D:
				case ShaderVariableDataType::SAMPLER_CUBE:
				{
					CString texfname;
					ANKI_CHECK(inputEl.getAttributeText("value", texfname));
					ANKI_CHECK(getManager().loadResource(texfname, mtlVar.m_tex, async));
					break;
				}

				default:
					ANKI_ASSERT(0);
					break;
				}
			}
		}

		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	}

	if(nonConstInputCount != 0)
	{
		ANKI_RESOURCE_LOGE("Forgot to list %u shader program inputs in this material", nonConstInputCount);
		return Error::USER_DATA;
	}

	if(constInputCount != 0)
	{
		ANKI_RESOURCE_LOGE("Forgot to list %u constant shader program variables in this material", constInputCount);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

const MaterialVariant& MaterialResource::getOrCreateVariant(const RenderingKey& key_) const
{
	RenderingKey key = key_;
	key.m_lod = min<U>(m_lodCount - 1, key.m_lod);

	if(!isInstanced())
	{
		ANKI_ASSERT(key.m_instanceCount == 1);
	}

	ANKI_ASSERT(!key.m_skinned || m_bonesMutator);
	ANKI_ASSERT(!key.m_velocity || m_velocityMutator);

	key.m_instanceCount = 1 << getInstanceGroupIdx(key.m_instanceCount);

	MaterialVariant& variant = m_variantMatrix[U(key.m_pass)][key.m_lod][getInstanceGroupIdx(key.m_instanceCount)]
											  [key.m_skinned][key.m_velocity];
	LockGuard<SpinLock> lock(m_variantMatrixMtx);

	if(variant.m_variant == nullptr)
	{
		const U mutatorCount = m_mutations.getSize() + ((m_instanceMutator) ? 1 : 0) + ((m_passMutator) ? 1 : 0)
							   + ((m_lodMutator) ? 1 : 0) + ((m_bonesMutator) ? 1 : 0) + ((m_velocityMutator) ? 1 : 0);

		DynamicArrayAuto<ShaderProgramResourceMutation> mutations(getTempAllocator());
		mutations.create(mutatorCount);
		U count = m_mutations.getSize();
		if(m_mutations.getSize())
		{
			memcpy(&mutations[0], &m_mutations[0], m_mutations.getSize() * sizeof(m_mutations[0]));
		}

		if(m_instanceMutator)
		{
			mutations[count].m_mutator = m_instanceMutator;
			mutations[count].m_value = key.m_instanceCount;
			++count;
		}

		if(m_passMutator)
		{
			mutations[count].m_mutator = m_passMutator;
			mutations[count].m_value = I(key.m_pass);
			++count;
		}

		if(m_lodMutator)
		{
			mutations[count].m_mutator = m_lodMutator;
			mutations[count].m_value = I(key.m_lod);
			++count;
		}

		if(m_bonesMutator)
		{
			mutations[count].m_mutator = m_bonesMutator;
			mutations[count].m_value = key.m_skinned != 0;
			++count;
		}

		if(m_velocityMutator)
		{
			mutations[count].m_mutator = m_velocityMutator;
			mutations[count].m_value = key.m_velocity != 0;
			++count;
		}

		m_prog->getOrCreateVariant(
			ConstWeakArray<ShaderProgramResourceMutation>(mutations.getSize() ? &mutations[0] : nullptr, count),
			ConstWeakArray<ShaderProgramResourceConstantValue>(
				(m_constValues.getSize()) ? &m_constValues[0] : nullptr, m_constValues.getSize()),
			variant.m_variant);
	}

	return variant;
}

U MaterialResource::getInstanceGroupIdx(U instanceCount)
{
	ANKI_ASSERT(instanceCount > 0);
	instanceCount = nextPowerOfTwo(instanceCount);
	ANKI_ASSERT(instanceCount <= MAX_INSTANCES);
	return log2(instanceCount);
}

} // end namespace anki
