// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/ShaderCompiler/MaliOfflineCompiler.h>

namespace anki {

ShaderProgramResourceVariant::ShaderProgramResourceVariant()
{
}

ShaderProgramResourceVariant::~ShaderProgramResourceVariant()
{
}

ShaderProgramResource::ShaderProgramResource(ResourceManager* manager)
	: ResourceObject(manager)
	, m_binary(&getMemoryPool())
{
}

ShaderProgramResource::~ShaderProgramResource()
{
	m_mutators.destroy(getMemoryPool());

	for(ShaderProgramResourceConstant& c : m_consts)
	{
		c.m_name.destroy(getMemoryPool());
	}
	m_consts.destroy(getMemoryPool());
	m_constBinaryMapping.destroy(getMemoryPool());

	for(auto it : m_variants)
	{
		ShaderProgramResourceVariant* variant = &(*it);
		deleteInstance(getMemoryPool(), variant);
	}
	m_variants.destroy(getMemoryPool());
}

Error ShaderProgramResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	// Load the binary
	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));
	ANKI_CHECK(m_binary.deserializeFromAnyFile(*file));
	const ShaderProgramBinary& binary = m_binary.getBinary();

	// Create the mutators
	if(binary.m_mutators.getSize() > 0)
	{
		m_mutators.create(getMemoryPool(), binary.m_mutators.getSize());

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
			const ShaderProgramResourceConstant* other = tryFindConstant(name);
			ANKI_ASSERT(other);
			mapping.m_constsIdx = U32(other - m_consts.getBegin());
		}
		else
		{
			mapping.m_constsIdx = m_consts.getSize();
		}

		m_constBinaryMapping.emplaceBack(getMemoryPool(), mapping);

		// Skip if const is there
		if(componentIdx > 0)
		{
			continue;
		}

		// Create new one
		ShaderProgramResourceConstant& in = *m_consts.emplaceBack(getMemoryPool());
		in.m_name.create(getMemoryPool(), name);
		in.m_index = m_consts.getSize() - 1;

		if(componentCount == 1)
		{
			in.m_dataType = c.m_type;
		}
		else if(componentCount == 2)
		{
			if(c.m_type == ShaderVariableDataType::kU32)
			{
				in.m_dataType = ShaderVariableDataType::kUVec2;
			}
			else if(c.m_type == ShaderVariableDataType::kI32)
			{
				in.m_dataType = ShaderVariableDataType::kIVec2;
			}
			else
			{
				ANKI_ASSERT(c.m_type == ShaderVariableDataType::kF32);
				in.m_dataType = ShaderVariableDataType::kVec2;
			}
		}
		else if(componentCount == 3)
		{
			if(c.m_type == ShaderVariableDataType::kU32)
			{
				in.m_dataType = ShaderVariableDataType::kUVec3;
			}
			else if(c.m_type == ShaderVariableDataType::kI32)
			{
				in.m_dataType = ShaderVariableDataType::kIVec3;
			}
			else
			{
				ANKI_ASSERT(c.m_type == ShaderVariableDataType::kF32);
				in.m_dataType = ShaderVariableDataType::kVec3;
			}
		}
		else if(componentCount == 4)
		{
			if(c.m_type == ShaderVariableDataType::kU32)
			{
				in.m_dataType = ShaderVariableDataType::kUVec4;
			}
			else if(c.m_type == ShaderVariableDataType::kI32)
			{
				in.m_dataType = ShaderVariableDataType::kIVec4;
			}
			else
			{
				ANKI_ASSERT(c.m_type == ShaderVariableDataType::kF32);
				in.m_dataType = ShaderVariableDataType::kVec4;
			}
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}

	m_shaderStages = binary.m_presentShaderTypes;

	// Do some RT checks
	if(!!(m_shaderStages & ShaderTypeBit::kAllRayTracing))
	{
		if(m_shaderStages != (ShaderTypeBit::kAnyHit | ShaderTypeBit::kClosestHit)
		   && m_shaderStages != ShaderTypeBit::kMiss && m_shaderStages != ShaderTypeBit::kRayGen)
		{
			ANKI_RESOURCE_LOGE("Any and closest hit shaders shouldn't coexist with other stages. Miss can't coexist "
							   "with other stages. Raygen can't coexist with other stages as well");
			return Error::kUserData;
		}
	}

	return Error::kNone;
}

Error ShaderProgramResource::parseConst(CString constName, U32& componentIdx, U32& componentCount, CString& name)
{
	const CString prefixName = "_anki_const_";
	const PtrSize prefix = constName.find(prefixName);
	if(prefix != 0)
	{
		// Simple name
		componentIdx = 0;
		componentCount = 1;
		name = constName;
		return Error::kNone;
	}

	Array<char, 2> number;
	number[0] = constName[prefixName.getLength()];
	number[1] = '\0';
	ANKI_CHECK(CString(number.getBegin()).toNumber(componentIdx));

	number[0] = constName[prefixName.getLength() + 2];
	ANKI_CHECK(CString(number.getBegin()).toNumber(componentCount));

	name = constName.getBegin() + prefixName.getLength() + 4;

	return Error::kNone;
}

void ShaderProgramResource::getOrCreateVariant(const ShaderProgramResourceVariantInitInfo& info,
											   const ShaderProgramResourceVariant*& variant) const
{
	// Sanity checks
	ANKI_ASSERT(info.m_setMutators.getEnabledBitCount() == m_mutators.getSize());
	ANKI_ASSERT(info.m_setConstants.getEnabledBitCount() == m_consts.getSize());

	// Compute variant hash
	U64 hash = 0;
	if(m_mutators.getSize())
	{
		hash = computeHash(info.m_mutation.getBegin(), m_mutators.getSize() * sizeof(info.m_mutation[0]));
	}

	if(m_consts.getSize())
	{
		hash =
			appendHash(info.m_constantValues.getBegin(), m_consts.getSize() * sizeof(info.m_constantValues[0]), hash);
	}

	// Check if the variant is in the cache
	{
		RLockGuard<RWMutex> lock(m_mtx);

		auto it = m_variants.find(hash);
		if(it != m_variants.getEnd())
		{
			// Done
			variant = *it;
			return;
		}
	}

	// Create the variant
	WLockGuard<RWMutex> lock(m_mtx);

	// Check again
	auto it = m_variants.find(hash);
	if(it != m_variants.getEnd())
	{
		// Done
		variant = *it;
		return;
	}

	// Create
	ShaderProgramResourceVariant* v = createNewVariant(info);
	if(v)
	{
		m_variants.emplace(getMemoryPool(), hash, v);
	}
	variant = v;
}

ShaderProgramResourceVariant*
ShaderProgramResource::createNewVariant(const ShaderProgramResourceVariantInitInfo& info) const
{
	const ShaderProgramBinary& binary = m_binary.getBinary();

	// Get the binary program variant
	const ShaderProgramBinaryVariant* binaryVariant = nullptr;
	U64 mutationHash = 0;
	if(m_mutators.getSize())
	{
		// Create the mutation hash
		mutationHash = computeHash(info.m_mutation.getBegin(), m_mutators.getSize() * sizeof(info.m_mutation[0]));

		// Search for the mutation in the binary
		// TODO optimize the search
		for(const ShaderProgramBinaryMutation& mutation : binary.m_mutations)
		{
			if(mutation.m_hash == mutationHash)
			{
				if(mutation.m_variantIndex == kMaxU32)
				{
					// Skipped mutation, nothing to create
					return nullptr;
				}

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
	ShaderProgramResourceVariant* variant = newInstance<ShaderProgramResourceVariant>(getMemoryPool());
	variant->m_binaryVariant = binaryVariant;

	// Set the constant values
	Array<ShaderSpecializationConstValue, 64> constValues;
	U32 constValueCount = 0;
	for(const ShaderProgramBinaryConstantInstance& instance : binaryVariant->m_constants)
	{
		const ShaderProgramBinaryConstant& c = binary.m_constants[instance.m_index];
		const U32 inputIdx = m_constBinaryMapping[instance.m_index].m_constsIdx;
		const U32 component = m_constBinaryMapping[instance.m_index].m_component;

		// Get value
		const ShaderProgramResourceConstantValue* value = nullptr;
		for(U32 i = 0; i < m_consts.getSize(); ++i)
		{
			if(info.m_constantValues[i].m_constantIndex == inputIdx)
			{
				value = &info.m_constantValues[i];
				break;
			}
		}
		ANKI_ASSERT(value && "Forgot to set the value of a constant");

		constValues[constValueCount].m_constantId = c.m_constantId;
		constValues[constValueCount].m_dataType = c.m_type;
		constValues[constValueCount].m_int = value->m_ivec4[component];
		++constValueCount;
	}

	// Get the workgroup sizes
	if(!!(m_shaderStages & ShaderTypeBit::kCompute))
	{
		for(U32 i = 0; i < 3; ++i)
		{
			if(binaryVariant->m_workgroupSizes[i] != kMaxU32)
			{
				// Size didn't come from specialization const
				variant->m_workgroupSizes[i] = binaryVariant->m_workgroupSizes[i];
			}
			else
			{
				// Size is specialization const

				ANKI_ASSERT(binaryVariant->m_workgroupSizesConstants[i] != kMaxU32);

				const U32 binaryConstIdx = binaryVariant->m_workgroupSizesConstants[i];
				const U32 constIdx = m_constBinaryMapping[binaryConstIdx].m_constsIdx;
				const U32 component = m_constBinaryMapping[binaryConstIdx].m_component;
				[[maybe_unused]] const Const& c = m_consts[constIdx];
				ANKI_ASSERT(c.m_dataType == ShaderVariableDataType::kU32
							|| c.m_dataType == ShaderVariableDataType::kUVec2
							|| c.m_dataType == ShaderVariableDataType::kUVec3
							|| c.m_dataType == ShaderVariableDataType::kUVec4);

				// Find the value
				for(U32 i = 0; i < m_consts.getSize(); ++i)
				{
					if(info.m_constantValues[i].m_constantIndex == constIdx)
					{
						const I32 value = info.m_constantValues[i].m_ivec4[component];
						ANKI_ASSERT(value > 0);

						variant->m_workgroupSizes[i] = U32(value);
						break;
					}
				}
			}

			ANKI_ASSERT(variant->m_workgroupSizes[i] != kMaxU32);
		}
	}

	// Time to init the shaders
	if(!!(m_shaderStages & (ShaderTypeBit::kAllGraphics | ShaderTypeBit::kCompute)))
	{
		// Create the program name
		StringRaii progName(&getTempMemoryPool());
		getFilepathFilename(getFilename(), progName);
		char* cprogName = const_cast<char*>(progName.cstr());
		if(progName.getLength() > kMaxGrObjectNameLength)
		{
			cprogName[kMaxGrObjectNameLength] = '\0';
		}

		ShaderProgramInitInfo progInf(cprogName);
		for(ShaderType shaderType : EnumIterable<ShaderType>())
		{
			if(!(ShaderTypeBit(1 << shaderType) & m_shaderStages))
			{
				continue;
			}

			ShaderInitInfo inf(cprogName);
			inf.m_shaderType = shaderType;
			inf.m_binary = binary.m_codeBlocks[binaryVariant->m_codeBlockIndices[shaderType]].m_binary;
			inf.m_constValues.setArray((constValueCount) ? constValues.getBegin() : nullptr, constValueCount);
			ShaderPtr shader = getManager().getGrManager().newShader(inf);

			const ShaderTypeBit shaderBit = ShaderTypeBit(1 << shaderType);
			if(!!(shaderBit & ShaderTypeBit::kAllGraphics))
			{
				progInf.m_graphicsShaders[shaderType] = shader;
			}
			else if(shaderType == ShaderType::kCompute)
			{
				progInf.m_computeShader = std::move(shader);
			}
			else
			{
				ANKI_ASSERT(0);
			}
		}

		// Create the program
		variant->m_prog = getManager().getGrManager().newShaderProgram(progInf);
	}
	else
	{
		ANKI_ASSERT(!!(m_shaderStages & ShaderTypeBit::kAllRayTracing));

		// Find the library
		CString libName = &binary.m_libraryName[0];
		ANKI_ASSERT(libName.getLength() > 0);

		const ShaderProgramResourceSystem& progSystem = getManager().getShaderProgramResourceSystem();
		const ShaderProgramRaytracingLibrary* foundLib = nullptr;
		for(const ShaderProgramRaytracingLibrary& lib : progSystem.getRayTracingLibraries())
		{
			if(lib.getLibraryName() == libName)
			{
				foundLib = &lib;
				break;
			}
		}
		ANKI_ASSERT(foundLib);

		variant->m_prog = foundLib->getShaderProgram();

		// Set the group handle index
		variant->m_shaderGroupHandleIndex = foundLib->getShaderGroupHandleIndex(getFilename(), mutationHash);
	}

	return variant;
}

} // end namespace anki
