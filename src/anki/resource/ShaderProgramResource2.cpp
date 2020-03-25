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
	m_consts.destroy(getAllocator());
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

	// Create the constants
	for(const ShaderProgramBinaryConstant& c : binary.m_constants)
	{
		U32 componentIdx;
		U32 componentCount;
		CString name;
		ANKI_CHECK(parseConst(c.m_name.getBegin(), componentIdx, componentCount, name));

		// Do the mapping
		ConstMapping mapping;
		mapping.m_component = componentIdx;
		if(componentIdx > 0)
		{
			const ShaderProgramResourceConstant2* other = tryFindConstant(name);
			ANKI_ASSERT(other);
			mapping.m_constsIdx = U32(other - m_consts.getBegin());
		}
		else
		{
			mapping.m_constsIdx = m_consts.getSize();
		}

		m_constBinaryMapping.emplaceBack(getAllocator(), mapping);

		// Skip if const is there
		if(componentIdx > 0)
		{
			continue;
		}

		// Create new one
		ShaderProgramResourceConstant2& in = *m_consts.emplaceBack(getAllocator());
		in.m_name.create(getAllocator(), name);
		in.m_index = m_consts.getSize() - 1;

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

	m_shaderStages = binary.m_presentShaderTypes;

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

			const U32 inputIdx = m_constBinaryMapping[instance.m_index].m_constsIdx;
			const U32 component = m_constBinaryMapping[instance.m_index].m_component;

			// Get value
			const ShaderProgramResourceConstantValue2* value = nullptr;
			for(U32 i = 0; i < info.m_constantValueCount; ++i)
			{
				if(info.m_constantValues[i].m_constantIndex == inputIdx)
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
