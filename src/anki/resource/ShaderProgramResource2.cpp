// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramResource2.h>
#include <anki/resource/ResourceManager.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/GrManager.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Functions.h>

namespace anki
{

ShaderProgramResourceVariant2::ShaderProgramResourceVariant2()
{
}

ShaderProgramResourceVariant2::~ShaderProgramResourceVariant2()
{
}

ShaderProgramResource2::ShaderProgramResource2(ResourceManager* manager)
	: ResourceObject(manager)
	, m_binary(getAllocator())
{
}

ShaderProgramResource2::~ShaderProgramResource2()
{
	m_mutators.destroy(getAllocator());
}

Error ShaderProgramResource2::load(const ResourceFilename& filename, Bool async)
{
	// Load the binary from the cache. It should have been compiled there
	StringAuto baseFilename(getTempAllocator());
	getFilepathFilename(filename, baseFilename);
	StringAuto binaryFilename(getTempAllocator());
	binaryFilename.sprintf("%s/%s.bin", getManager().getCacheDirectory().cstr(), binaryFilename.cstr());
	ANKI_CHECK(m_binary.deserializeFromFile(binaryFilename));
	const ShaderProgramBinary& binary = m_binary.getBinary();

	// Create the mutators
	if(binary.m_mutators.getSize() > 0)
	{
		m_mutators.create(getAllocator(), binary.m_mutators.getSize());

		for(U32 i = 0; i < binary.m_mutators.getSize(); ++i)
		{
			m_mutators[i].m_name = binary.m_mutators[i].m_name.getBegin();
			ANKI_ASSERT(m_mutators[i].m_name.getLength() > 0);
			m_mutators[i].m_values = binary.m_mutators[i].m_values;
		}
	}

	// Create the inputs
	{
		U32 descriptorSet = MAX_U32;
		U32 maxDescriptorSet = 0;
		for(const ShaderProgramBinaryBlock& block : binary.m_uniformBlocks)
		{
			maxDescriptorSet = max(maxDescriptorSet, block.m_set);

			if(block.m_name.getBegin() != CString("b_ankiMaterial"))
			{
				continue;
			}

			m_materialUboIdx = U8(&block - &binary.m_uniformBlocks[0]);
			descriptorSet = block.m_set;

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

				ShaderProgramResourceInputVariable2& in = *m_inputVars.emplaceBack(getAllocator());
				in.m_name.create(getAllocator(), name);
				in.m_index = m_inputVars.getSize() - 1;
				in.m_constant = false;
				in.m_dataType = var.m_type;
				in.m_instanced = instanced;
			}
		}

		// Continue with the opaque if it's a material shader program
		if(descriptorSet != MAX_U32)
		{
			for(const ShaderProgramBinaryOpaque& o : binary.m_opaques)
			{
				maxDescriptorSet = max(maxDescriptorSet, o.m_set);

				if(o.m_set != descriptorSet)
				{
					continue;
				}

				ShaderProgramResourceInputVariable2& in = *m_inputVars.emplaceBack(getAllocator());
				in.m_name.create(getAllocator(), o.m_name.getBegin());
				in.m_index = m_inputVars.getSize() - 1;
				in.m_constant = false;
				in.m_dataType = o.m_type;
				in.m_instanced = false;
			}
		}

		if(descriptorSet != MAX_U32 && descriptorSet != maxDescriptorSet)
		{
			ANKI_RESOURCE_LOGE("All bindings of a material shader should be in the highest descriptor set");
			return Error::USER_DATA;
		}

		m_descriptorSet = U8(descriptorSet);

		for(const ShaderProgramBinaryConstant& c : binary.m_constants)
		{
			U32 componentIdx;
			U32 componentCount;
			CString name;
			ANKI_CHECK(parseConst(c.m_name.getBegin(), componentIdx, componentCount, name));

			if(componentIdx > 0)
			{
				continue;
			}

			ShaderProgramResourceInputVariable2& in = *m_inputVars.emplaceBack(getAllocator());
			in.m_name.create(getAllocator(), name);
			in.m_index = m_inputVars.getSize() - 1;
			in.m_constant = true;

			if(componentCount == 1)
			{
				in.m_dataType = c.m_type;
			}
			else if(componentCount == 2)
			{
				if(c.m_type == ShaderVariableDataType::INT)
				{
					in.m_dataType = ShaderVariableDataType::IVEC2;
				}
				else
				{
					ANKI_ASSERT(c.m_type == ShaderVariableDataType::FLOAT);
					in.m_dataType = ShaderVariableDataType::VEC2;
				}
			}
			else if(componentCount == 3)
			{
				if(c.m_type == ShaderVariableDataType::INT)
				{
					in.m_dataType = ShaderVariableDataType::IVEC3;
				}
				else
				{
					ANKI_ASSERT(c.m_type == ShaderVariableDataType::FLOAT);
					in.m_dataType = ShaderVariableDataType::VEC3;
				}
			}
			else if(componentCount == 4)
			{
				if(c.m_type == ShaderVariableDataType::INT)
				{
					in.m_dataType = ShaderVariableDataType::IVEC4;
				}
				else
				{
					ANKI_ASSERT(c.m_type == ShaderVariableDataType::FLOAT);
					in.m_dataType = ShaderVariableDataType::VEC4;
				}
			}
			else
			{
				ANKI_ASSERT(0);
			}
		}
	} // End creating the inputs

	m_shaderStages = binary.m_presentShaderTypes;

	return Error::NONE;
}

Error ShaderProgramResource2::parseVariable(CString fullVarName, Bool& instanced, U32& idx, CString& name)
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

Error ShaderProgramResource2::parseConst(CString constName, U32& componentIdx, U32& componentCount, CString& name)
{
	const CString prefixName = "_anki_const_";
	const PtrSize prefix = constName.find(prefixName);
	if(prefix != 0)
	{
		// Simple name
		componentIdx = 0;
		componentCount = 1;
		name = constName;
		return Error::NONE;
	}

	Array<char, 2> number;
	number[0] = constName[prefixName.getLength() + 1];
	number[1] = '\0';
	ANKI_CHECK(CString(number.getBegin()).toNumber(componentIdx));

	number[0] = constName[prefixName.getLength() + 3];
	ANKI_CHECK(CString(number.getBegin()).toNumber(componentCount));

	name = constName.getBegin() + prefixName.getLength() + 4;

	return Error::NONE;
}

void ShaderProgramResource2::getOrCreateVariant(
	const ShaderProgramResourceVariantInitInfo2& info, const ShaderProgramResourceVariant2*& variant) const
{
	// Sanity checks
	ANKI_ASSERT(info.m_setMutators.getEnabledBitCount() == m_mutators.getSize());

	// Compute variant hash
	U64 hash = 0;
	if(info.m_mutationCount)
	{
		hash = computeHash(info.m_mutation.getBegin(), info.m_mutationCount * sizeof(info.m_mutation[0]));
	}

	if(info.m_constantValueCount)
	{
		hash = appendHash(
			info.m_constantValues.getBegin(), info.m_constantValueCount * sizeof(info.m_constantValues[0]), hash);
	}

	// Check if the variant is in the cache
	{
		RLockGuard<RWMutex> lock(m_mtx);

		auto it = m_variants.find(hash);
		variant = (it != m_variants.getEnd()) ? *it : nullptr;
	}

	if(variant != nullptr)
	{
		// Done
		return;
	}

	// Create the variant
	WLockGuard<RWMutex> lock(m_mtx);

	ShaderProgramResourceVariant2* v = getAllocator().newInstance<ShaderProgramResourceVariant2>();
	initVariant(info, *v);
	m_variants.emplace(getAllocator(), hash, v);
	variant = v;
}

void ShaderProgramResource2::initVariant(
	const ShaderProgramResourceVariantInitInfo2& info, ShaderProgramResourceVariant2& variant) const
{
	const ShaderProgramBinary& binary = m_binary.getBinary();

	// Get the binary program variant
	const ShaderProgramBinaryVariant* binaryVariant = nullptr;
	if(m_mutators.getSize())
	{
		// Create the mutation hash
		const U64 hash = computeHash(info.m_mutation.getBegin(), info.m_mutationCount * sizeof(info.m_mutation[0]));

		// Search for the mutation in the binary
		// TODO optimize the search
		for(const ShaderProgramBinaryMutation& mutation : binary.m_mutations)
		{
			if(mutation.m_hash == hash)
			{
				binaryVariant = &binary.m_variants[mutation.m_variantIndex];
				break;
			}
		}
	}
	else
	{
		ANKI_ASSERT(binary.m_variants.getSize() == 1);
		binaryVariant = &binary.m_variants[0];
	}
	ANKI_ASSERT(binaryVariant);

	// Init the uniform vars
	if(m_materialUboIdx != MAX_U8)
	{
		variant.m_blockInfos.create(getAllocator(), m_inputVars.getSize());

		for(const ShaderProgramBinaryVariableInstance& instance :
			binaryVariant->m_uniformBlocks[m_materialUboIdx].m_variables)
		{
			ANKI_ASSERT(instance.m_index == m_materialUboIdx);
			const U32 inputIdx = m_binaryMapping.m_uniformVars[instance.m_index].m_inputVarIdx;
			const U32 arrayIdx = m_binaryMapping.m_uniformVars[instance.m_index].m_arrayIdx;
			ShaderVariableBlockInfo& blockInfo = variant.m_blockInfos[inputIdx];

			if(arrayIdx == 0)
			{
				blockInfo = instance.m_blockInfo;
			}
			else if(arrayIdx == 1)
			{
				ANKI_ASSERT(blockInfo.m_offset >= 0);
				blockInfo.m_arraySize = max(blockInfo.m_arraySize, I16(arrayIdx + 1));
				ANKI_ASSERT(instance.m_blockInfo.m_offset > blockInfo.m_offset);
				blockInfo.m_arrayStride = instance.m_blockInfo.m_offset - blockInfo.m_offset;
			}
			else
			{
				ANKI_ASSERT(blockInfo.m_offset >= 0);
				blockInfo.m_arraySize = max(blockInfo.m_arraySize, I16(arrayIdx + 1));
			}

			variant.m_activeInputVars.set(inputIdx);
		}
	}

	// Init the opaques
	variant.m_opaqueBindings.create(getAllocator(), m_inputVars.getSize(), -1);
	for(const ShaderProgramBinaryOpaqueInstance& instance : binaryVariant->m_opaques)
	{
		const U32 opaqueIdx = instance.m_index;
		const ShaderProgramBinaryOpaque& opaque = binary.m_opaques[opaqueIdx];
		if(opaque.m_set != m_descriptorSet)
		{
			continue;
		}

		const U32 inputIdx = m_binaryMapping.m_opaques[opaqueIdx];
		const Input& in = m_inputVars[inputIdx];
		ANKI_ASSERT(in.isSampler() || in.isTexture());

		variant.m_opaqueBindings[inputIdx] = I16(opaque.m_binding);
		variant.m_activeInputVars.set(inputIdx);
	}

	// Set the constannt values
	Array2d<ShaderSpecializationConstValue, U(ShaderType::COUNT), 64> constValues;
	Array<U32, U(ShaderType::COUNT)> constValueCounts;
	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		if(!(shaderTypeToBit(shaderType) & m_shaderStages))
		{
			continue;
		}

		for(const ShaderProgramBinaryConstantInstance& instance : binaryVariant->m_constants)
		{
			const ShaderProgramBinaryConstant& c = binary.m_constants[instance.m_index];
			if(!(c.m_shaderStages & shaderTypeToBit(shaderType)))
			{
				continue;
			}

			const U32 inputIdx = m_binaryMapping.m_constants[instance.m_index].m_inputVarIdx;
			const U32 component = m_binaryMapping.m_constants[instance.m_index].m_component;

			// Get value
			const ShaderProgramResourceConstantValue2* value = nullptr;
			for(U32 i = 0; i < info.m_constantValueCount; ++i)
			{
				if(info.m_constantValues[i].m_inputVariableIndex == inputIdx)
				{
					value = &info.m_constantValues[i];
					break;
				}
			}
			ANKI_ASSERT(value && "Forgot to set the value of a constant");

			U32& count = constValueCounts[shaderType];
			constValues[shaderType][count].m_constantId = c.m_constantId;
			constValues[shaderType][count].m_dataType = c.m_type;
			constValues[shaderType][count].m_int = value->m_ivec4[component];
			++count;
		}
	}

	// Create the program name
	StringAuto progName(getTempAllocator());
	getFilepathFilename(getFilename(), progName);
	char* cprogName = const_cast<char*>(progName.cstr());
	if(progName.getLength() > MAX_GR_OBJECT_NAME_LENGTH)
	{
		cprogName[MAX_GR_OBJECT_NAME_LENGTH] = '\0';
	}

	// Time to init the shaders
	ShaderProgramInitInfo progInf(cprogName);
	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		if(!(shaderTypeToBit(shaderType) & m_shaderStages))
		{
			continue;
		}

		ShaderInitInfo inf(cprogName);
		inf.m_shaderType = shaderType;
		inf.m_binary = binary.m_codeBlocks[binaryVariant->m_codeBlockIndices[shaderType]].m_binary;
		inf.m_constValues.setArray(&constValues[shaderType][0], constValueCounts[shaderType]);

		progInf.m_shaders[shaderType] = getManager().getGrManager().newShader(inf);
	}

	// Create the program
	variant.m_prog = getManager().getGrManager().newShaderProgram(progInf);
}

} // end namespace anki
