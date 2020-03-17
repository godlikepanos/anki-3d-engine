// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramResource2.h>
#include <anki/resource/ResourceManager.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/util/Filesystem.h>

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
	for(const ShaderProgramBinaryBlock& block : binary.m_uniformBlocks)
	{
		if(block.m_name.getBegin() != CString("b_ankiMaterial"))
		{
			continue;
		}

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

	// XXX
	for(const Input& in : m_inputVars)
	{
		(void)in;
	}
}

} // end namespace anki
