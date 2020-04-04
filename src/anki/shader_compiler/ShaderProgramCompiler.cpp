// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramCompiler.h>
#include <anki/shader_compiler/ShaderProgramParser.h>
#include <anki/shader_compiler/Glslang.h>
#include <anki/shader_compiler/ShaderProgramReflection.h>
#include <anki/util/Serializer.h>
#include <anki/util/HashMap.h>
#include <SPIRV-Cross/spirv_glsl.hpp>

namespace anki
{

static const char* SHADER_BINARY_MAGIC = "ANKISDR1";

Error ShaderProgramBinaryWrapper::serializeToFile(CString fname) const
{
	ANKI_ASSERT(m_binary);

	File file;
	ANKI_CHECK(file.open(fname, FileOpenFlag::WRITE | FileOpenFlag::BINARY));

	BinarySerializer serializer;
	StackAllocator<U8> tmpAlloc(
		m_alloc.getMemoryPool().getAllocationCallback(), m_alloc.getMemoryPool().getAllocationCallbackUserData(), 4_KB);
	ANKI_CHECK(serializer.serialize(*m_binary, tmpAlloc, file));

	return Error::NONE;
}

Error ShaderProgramBinaryWrapper::deserializeFromFile(CString fname)
{
	cleanup();

	File file;
	ANKI_CHECK(file.open(fname, FileOpenFlag::READ | FileOpenFlag::BINARY));

	BinaryDeserializer deserializer;
	ANKI_CHECK(deserializer.deserialize(m_binary, m_alloc, file));

	m_singleAllocation = true;

	if(memcmp(SHADER_BINARY_MAGIC, &m_binary->m_magic[0], 0) != 0)
	{
		ANKI_SHADER_COMPILER_LOGE("Corrupted or wrong version of shader binary: %s", fname.cstr());
		return Error::USER_DATA;
	}

	return Error::NONE;
}

void ShaderProgramBinaryWrapper::cleanup()
{
	if(m_binary == nullptr)
	{
		return;
	}

	if(!m_singleAllocation)
	{
		for(ShaderProgramBinaryMutator& mutator : m_binary->m_mutators)
		{
			m_alloc.getMemoryPool().free(mutator.m_values.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_mutators.getBegin());

		for(ShaderProgramBinaryCodeBlock& code : m_binary->m_codeBlocks)
		{
			m_alloc.getMemoryPool().free(code.m_binary.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_codeBlocks.getBegin());

		for(ShaderProgramBinaryMutation& m : m_binary->m_mutations)
		{
			m_alloc.getMemoryPool().free(m.m_values.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_mutations.getBegin());

		for(ShaderProgramBinaryBlock& block : m_binary->m_uniformBlocks)
		{
			m_alloc.getMemoryPool().free(block.m_variables.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_uniformBlocks.getBegin());

		for(ShaderProgramBinaryBlock& block : m_binary->m_storageBlocks)
		{
			m_alloc.getMemoryPool().free(block.m_variables.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_storageBlocks.getBegin());

		if(m_binary->m_pushConstantBlock)
		{
			m_alloc.getMemoryPool().free(m_binary->m_pushConstantBlock->m_variables.getBegin());
			m_alloc.getMemoryPool().free(m_binary->m_pushConstantBlock);
		}

		m_alloc.getMemoryPool().free(m_binary->m_opaques.getBegin());
		m_alloc.getMemoryPool().free(m_binary->m_constants.getBegin());

		for(ShaderProgramBinaryVariant& variant : m_binary->m_variants)
		{
			for(ShaderProgramBinaryBlockInstance& block : variant.m_uniformBlocks)
			{
				m_alloc.getMemoryPool().free(block.m_variables.getBegin());
			}

			for(ShaderProgramBinaryBlockInstance& block : variant.m_storageBlocks)
			{
				m_alloc.getMemoryPool().free(block.m_variables.getBegin());
			}

			if(variant.m_pushConstantBlock)
			{
				m_alloc.getMemoryPool().free(variant.m_pushConstantBlock->m_variables.getBegin());
			}

			m_alloc.getMemoryPool().free(variant.m_uniformBlocks.getBegin());
			m_alloc.getMemoryPool().free(variant.m_storageBlocks.getBegin());
			m_alloc.getMemoryPool().free(variant.m_pushConstantBlock);
			m_alloc.getMemoryPool().free(variant.m_constants.getBegin());
			m_alloc.getMemoryPool().free(variant.m_opaques.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_variants.getBegin());
	}

	m_alloc.getMemoryPool().free(m_binary);
	m_binary = nullptr;
	m_singleAllocation = false;
}

/// Spin the dials. Used to compute all mutator combinations.
static Bool spinDials(DynamicArrayAuto<U32>& dials, ConstWeakArray<ShaderProgramParserMutator> mutators)
{
	ANKI_ASSERT(dials.getSize() == mutators.getSize() && dials.getSize() > 0);
	Bool done = true;

	U32 crntDial = dials.getSize() - 1;
	while(true)
	{
		// Turn dial
		++dials[crntDial];

		if(dials[crntDial] >= mutators[crntDial].getValues().getSize())
		{
			if(crntDial == 0)
			{
				// Reached the 1st dial, stop spinning
				done = true;
				break;
			}
			else
			{
				dials[crntDial] = 0;
				--crntDial;
			}
		}
		else
		{
			done = false;
			break;
		}
	}

	return done;
}

static Error compileVariant(ConstWeakArray<MutatorValue> mutation,
	const ShaderProgramParser& parser,
	ShaderProgramBinaryVariant& variant,
	DynamicArrayAuto<ShaderProgramBinaryCodeBlock>& codeBlocks,
	DynamicArrayAuto<U64>& codeBlockHashes,
	GenericMemoryPoolAllocator<U8>& tmpAlloc,
	GenericMemoryPoolAllocator<U8>& binaryAlloc)
{
	variant = {};

	// Generate the source and the rest for the variant
	ShaderProgramParserVariant parserVariant;
	ANKI_CHECK(parser.generateVariant(mutation, parserVariant));

	// Compile stages
	Array<ConstWeakArray<U8>, U32(ShaderType::COUNT)> spirvBinaries;
	for(ShaderType shaderType = ShaderType::FIRST; shaderType < ShaderType::COUNT; ++shaderType)
	{
		if(!(shaderTypeToBit(shaderType) & parser.getShaderTypes()))
		{
			variant.m_codeBlockIndices[shaderType] = MAX_U32;
			continue;
		}

		// Compile
		DynamicArrayAuto<U8> spirv(tmpAlloc);
		ANKI_CHECK(compilerGlslToSpirv(parserVariant.getSource(shaderType), shaderType, tmpAlloc, spirv));
		ANKI_ASSERT(spirv.getSize() > 0);

		// Check if the spirv is already generated
		const U64 newHash = computeHash(&spirv[0], spirv.getSize());
		Bool found = false;
		for(U32 i = 0; i < codeBlockHashes.getSize(); ++i)
		{
			if(codeBlockHashes[i] == newHash)
			{
				// Found it
				variant.m_codeBlockIndices[shaderType] = i;
				found = true;
				break;
			}
		}

		// Create it if not found
		if(!found)
		{
			U8* code = binaryAlloc.allocate(spirv.getSizeInBytes());
			memcpy(code, &spirv[0], spirv.getSizeInBytes());

			ShaderProgramBinaryCodeBlock block;
			block.m_binary.setArray(code, U32(spirv.getSizeInBytes()));
			codeBlocks.emplaceBack(block);

			codeBlockHashes.emplaceBack(newHash);

			variant.m_codeBlockIndices[shaderType] = codeBlocks.getSize() - 1;
		}

		spirvBinaries[shaderType] = codeBlocks[variant.m_codeBlockIndices[shaderType]].m_binary;
	}

	return Error::NONE;
}

/// Will be called once per instance.
class Refl final : public ShaderReflectionVisitorInterface
{
public:
	GenericMemoryPoolAllocator<U8> m_alloc;

	/// Will be stored in the binary
	/// @{

	/// [blockType][blockIdx]
	Array<DynamicArrayAuto<ShaderProgramBinaryBlock>, 3> m_blocks = {{m_alloc, m_alloc, m_alloc}};

	/// [blockType][blockIdx][varIdx]
	Array<DynamicArrayAuto<DynamicArrayAuto<ShaderProgramBinaryVariable>>, 3> m_vars = {
		{{m_alloc}, {m_alloc}, {m_alloc}}};

	DynamicArrayAuto<ShaderProgramBinaryOpaque> m_opaque = {m_alloc};
	DynamicArrayAuto<ShaderProgramBinaryConstant> m_consts = {m_alloc};
	/// @}

	/// Will be stored in a variant
	/// @{

	/// [blockType][blockInstanceIdx]
	Array<DynamicArrayAuto<ShaderProgramBinaryBlockInstance>, 3> m_blockInstances = {{m_alloc, m_alloc, m_alloc}};

	DynamicArrayAuto<ShaderProgramBinaryOpaqueInstance> m_opaqueInstances = {m_alloc};
	DynamicArrayAuto<ShaderProgramBinaryConstantInstance> m_constInstances = {m_alloc};
	/// @}

	Refl(const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	Error setCounts(
		U32 uniformBlockCount, U32 storageBlockCount, U32 opaqueCount, Bool pushConstantBlock, U32 constCount) final
	{
		m_blockInstances[0].create(uniformBlockCount);
		m_blockInstances[1].create(storageBlockCount);
		if(pushConstantBlock)
		{
			m_blockInstances[2].create(1);
		}
		m_opaqueInstances.create(opaqueCount);
		m_constInstances.create(constCount);
		return Error::NONE;
	}

	Error visitUniformBlock(U32 idx, CString name, U32 set, U32 binding, U32 size, U32 varCount) final
	{
		return visitAnyBlock(idx, name, set, binding, size, varCount, 0);
	}

	Error visitUniformVariable(U32 blockIdx,
		U32 idx,
		CString name,
		ShaderVariableDataType type,
		const ShaderVariableBlockInfo& blockInfo) final
	{
		return visitAnyVariable(blockIdx, idx, name, type, blockInfo, 0);
	}

	Error visitStorageBlock(U32 idx, CString name, U32 set, U32 binding, U32 size, U32 varCount) final
	{
		return visitAnyBlock(idx, name, set, binding, size, varCount, 1);
	}

	Error visitStorageVariable(U32 blockIdx,
		U32 idx,
		CString name,
		ShaderVariableDataType type,
		const ShaderVariableBlockInfo& blockInfo) final
	{
		return visitAnyVariable(blockIdx, idx, name, type, blockInfo, 1);
	}

	Error visitPushConstantsBlock(CString name, U32 size, U32 varCount) final
	{
		return visitAnyBlock(0, name, 0, 0, size, varCount, 2);
	}

	Error visitPushConstant(
		U32 idx, CString name, ShaderVariableDataType type, const ShaderVariableBlockInfo& blockInfo) final
	{
		return visitAnyVariable(0, idx, name, type, blockInfo, 2);
	}

	Error visitOpaque(
		U32 instanceIdx, CString name, ShaderVariableDataType type, U32 set, U32 binding, U32 arraySize) final
	{
		// Find the opaque
		U32 opaqueIdx = MAX_U32;
		for(U32 i = 0; i < m_opaque.getSize(); ++i)
		{
			if(name == m_opaque[i].m_name.getBegin())
			{
				if(type != m_opaque[i].m_type || set != m_opaque[i].m_set || binding != m_opaque[i].m_binding)
				{
					ANKI_SHADER_COMPILER_LOGE(
						"The set, binding and type can't difer between shader variants for opaque: %s", name.cstr());
					return Error::USER_DATA;
				}

				opaqueIdx = i;
				break;
			}
		}

		// Create the opaque
		if(opaqueIdx == MAX_U32)
		{
			ShaderProgramBinaryOpaque& o = *m_opaque.emplaceBack();
			ANKI_CHECK(setName(name, o.m_name));
			o.m_type = type;
			o.m_binding = binding;
			o.m_set = set;

			opaqueIdx = m_opaque.getSize() - 1;
		}

		// Create the instance
		ShaderProgramBinaryOpaqueInstance& instance = m_opaqueInstances[instanceIdx];
		instance.m_index = opaqueIdx;
		instance.m_arraySize = arraySize;

		return Error::NONE;
	}

	Error visitConstant(U32 instanceIdx, CString name, ShaderVariableDataType type, U32 constantId) final
	{
		// Find const
		U32 constIdx = MAX_U32;
		for(U32 i = 0; i < m_consts.getSize(); ++i)
		{
			if(name == m_consts[i].m_name.getBegin())
			{
				if(type != m_consts[i].m_type || constantId != m_consts[i].m_constantId)
				{
					ANKI_SHADER_COMPILER_LOGE(
						"The type, constantId and stages can't difer between shader variants for const: %s",
						name.cstr());
					return Error::USER_DATA;
				}

				constIdx = i;
				break;
			}
		}

		// Create the const
		if(constIdx == MAX_U32)
		{
			ShaderProgramBinaryConstant& c = *m_consts.emplaceBack();
			ANKI_CHECK(setName(name, c.m_name));
			c.m_type = type;
			c.m_constantId = constantId;

			constIdx = m_consts.getSize() - 1;
		}

		// Create the instance
		ShaderProgramBinaryConstantInstance& instance = m_constInstances[instanceIdx];
		instance.m_index = constIdx;

		return Error::NONE;
	}

	static ANKI_USE_RESULT Error setName(CString in, Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1>& out)
	{
		if(in.getLength() + 1 > MAX_SHADER_BINARY_NAME_LENGTH)
		{
			ANKI_SHADER_COMPILER_LOGE("Name too long: %s", in.cstr());
			return Error::USER_DATA;
		}
		else if(in.getLength() == 0)
		{
			ANKI_SHADER_COMPILER_LOGE("Found an empty string as name");
			return Error::USER_DATA;
		}
		else
		{
			memcpy(out.getBegin(), in.getBegin(), in.getLength() + 1);
		}
		return Error::NONE;
	}

	static ANKI_USE_RESULT Error findBlock(
		CString name, U32 set, U32 binding, ConstWeakArray<ShaderProgramBinaryBlock> arr, U32& idx)
	{
		idx = MAX_U32;

		for(U32 i = 0; i < arr.getSize(); ++i)
		{
			const ShaderProgramBinaryBlock& block = arr[i];
			if(block.m_name.getBegin() == name)
			{
				if(set != block.m_set || binding != block.m_binding)
				{
					ANKI_SHADER_COMPILER_LOGE(
						"The set and binding can't difer between shader variants for block: %s", name.cstr());
					return Error::USER_DATA;
				}

				idx = i;
				break;
			}

			return Error::NONE;
		}

		return Error::NONE;
	}

	Error visitAnyBlock(U32 blockInstanceIdx, CString name, U32 set, U32 binding, U32 size, U32 varSize, U32 blockType)
	{
		// Init the block
		U32 blockIdx;
		ANKI_CHECK(findBlock(name, set, binding, m_blocks[blockType], blockIdx));
		if(blockIdx == MAX_U32)
		{
			// Not found, create it
			ShaderProgramBinaryBlock& block = *m_blocks[blockType].emplaceBack();
			ANKI_CHECK(setName(name, block.m_name));
			block.m_set = set;
			block.m_binding = binding;
			blockIdx = m_blocks[blockType].getSize() - 1;

			// Create some storage for vars as well
			m_vars[blockType].emplaceBack(m_alloc);
			ANKI_ASSERT(m_vars[blockType].getSize() == m_blocks[blockType].getSize());
		}

		// Init the instance
		ShaderProgramBinaryBlockInstance& instance = m_blockInstances[blockType][blockInstanceIdx];
		instance.m_index = blockIdx;
		instance.m_size = size;
		instance.m_variables.setArray(m_alloc.newArray<ShaderProgramBinaryVariableInstance>(varSize), varSize);

		return Error::NONE;
	}

	Error visitAnyVariable(U32 blockInstanceIdx,
		U32 varInstanceIdx,
		CString name,
		ShaderVariableDataType type,
		const ShaderVariableBlockInfo& blockInfo,
		U32 blockType)
	{
		// Find the variable
		U32 varIdx = MAX_U32;
		const U32 blockIdx = m_blockInstances[blockType][blockInstanceIdx].m_index;
		for(U32 i = 0; i < m_vars[blockType][blockIdx].getSize(); ++i)
		{
			const ShaderProgramBinaryVariable& var = m_vars[blockType][blockIdx][i];
			if(var.m_name.getBegin() == name)
			{
				if(var.m_type != type)
				{
					ANKI_SHADER_COMPILER_LOGE(
						"The type should not differ between variants for variable: %s", name.cstr());
					return Error::USER_DATA;
				}

				varIdx = i;
				break;
			}
		}

		// Create the variable
		if(varIdx == MAX_U32)
		{
			ShaderProgramBinaryVariable& var = *m_vars[blockType][blockIdx].emplaceBack();
			ANKI_CHECK(setName(name, var.m_name));
			var.m_type = type;

			varIdx = m_vars[blockType][blockIdx].getSize() - 1;
		}

		// Init the instance
		ShaderProgramBinaryVariableInstance& instance =
			m_blockInstances[blockType][blockInstanceIdx].m_variables[varInstanceIdx];
		instance.m_blockInfo = blockInfo;
		instance.m_index = varIdx;

		return Error::NONE;
	}
};

static Error doReflection(
	ShaderProgramBinary& binary, GenericMemoryPoolAllocator<U8>& tmpAlloc, GenericMemoryPoolAllocator<U8>& binaryAlloc)
{
	ANKI_ASSERT(binary.m_variants.getSize() > 0);

	Refl refl(binaryAlloc);

	for(ShaderProgramBinaryVariant& variant : binary.m_variants)
	{
		Array<ConstWeakArray<U8>, U32(ShaderType::COUNT)> spirvs;
		for(ShaderType stage : EnumIterable<ShaderType>())
		{
			if(variant.m_codeBlockIndices[stage] != MAX_U32)
			{
				spirvs[stage] = binary.m_codeBlocks[variant.m_codeBlockIndices[stage]].m_binary;
			}
		}

		ANKI_CHECK(performSpirvReflection(spirvs, tmpAlloc, refl));

		// Store the instances
		if(refl.m_blockInstances[0].getSize())
		{
			ShaderProgramBinaryBlockInstance* instances;
			U32 size, storageSize;
			refl.m_blockInstances[0].moveAndReset(instances, size, storageSize);
			variant.m_uniformBlocks.setArray(instances, size);
		}

		if(refl.m_blockInstances[1].getSize())
		{
			ShaderProgramBinaryBlockInstance* instances;
			U32 size, storageSize;
			refl.m_blockInstances[1].moveAndReset(instances, size, storageSize);
			variant.m_storageBlocks.setArray(instances, size);
		}

		if(refl.m_blockInstances[2].getSize())
		{
			ShaderProgramBinaryBlockInstance* instances;
			U32 size, storageSize;
			refl.m_blockInstances[2].moveAndReset(instances, size, storageSize);
			ANKI_ASSERT(size == 1);
			variant.m_pushConstantBlock = instances;
		}

		if(refl.m_opaqueInstances.getSize())
		{
			ShaderProgramBinaryOpaqueInstance* instances;
			U32 size, storageSize;
			refl.m_opaqueInstances.moveAndReset(instances, size, storageSize);
			variant.m_opaques.setArray(instances, size);
		}

		if(refl.m_constInstances.getSize())
		{
			ShaderProgramBinaryConstantInstance* instances;
			U32 size, storageSize;
			refl.m_constInstances.moveAndReset(instances, size, storageSize);
			variant.m_constants.setArray(instances, size);
		}
	}

	if(refl.m_blocks[0].getSize())
	{
		ShaderProgramBinaryBlock* blocks;
		U32 size, storageSize;
		refl.m_blocks[0].moveAndReset(blocks, size, storageSize);
		binary.m_uniformBlocks.setArray(blocks, size);

		for(U32 i = 0; i < size; ++i)
		{
			ShaderProgramBinaryVariable* vars;
			U32 varSize, varStorageSize;
			refl.m_vars[0][i].moveAndReset(vars, varSize, varStorageSize);
			binary.m_uniformBlocks[i].m_variables.setArray(vars, varSize);
		}
	}

	if(refl.m_blocks[1].getSize())
	{
		ShaderProgramBinaryBlock* blocks;
		U32 size, storageSize;
		refl.m_blocks[1].moveAndReset(blocks, size, storageSize);
		binary.m_storageBlocks.setArray(blocks, size);

		for(U32 i = 0; i < size; ++i)
		{
			ShaderProgramBinaryVariable* vars;
			U32 varSize, varStorageSize;
			refl.m_vars[1][i].moveAndReset(vars, varSize, varStorageSize);
			binary.m_storageBlocks[i].m_variables.setArray(vars, varSize);
		}
	}

	if(refl.m_blocks[2].getSize())
	{
		ShaderProgramBinaryBlock* blocks;
		U32 size, storageSize;
		refl.m_blocks[2].moveAndReset(blocks, size, storageSize);
		ANKI_ASSERT(size == 1);
		binary.m_pushConstantBlock = blocks;

		ShaderProgramBinaryVariable* vars;
		U32 varSize, varStorageSize;
		refl.m_vars[2][0].moveAndReset(vars, varSize, varStorageSize);
		binary.m_pushConstantBlock->m_variables.setArray(vars, varSize);
	}

	if(refl.m_opaque.getSize())
	{
		ShaderProgramBinaryOpaque* opaques;
		U32 size, storageSize;
		refl.m_opaque.moveAndReset(opaques, size, storageSize);
		binary.m_opaques.setArray(opaques, size);
	}

	if(refl.m_consts.getSize())
	{
		ShaderProgramBinaryConstant* consts;
		U32 size, storageSize;
		refl.m_consts.moveAndReset(consts, size, storageSize);
		binary.m_constants.setArray(consts, size);
	}

	return Error::NONE;
}

Error compileShaderProgram(CString fname,
	ShaderProgramFilesystemInterface& fsystem,
	ShaderProgramPostParseInterface* postParseCallback,
	GenericMemoryPoolAllocator<U8> tempAllocator,
	const GpuDeviceCapabilities& gpuCapabilities,
	const BindlessLimits& bindlessLimits,
	ShaderProgramBinaryWrapper& binaryW)
{
	// Initialize the binary
	binaryW.cleanup();
	binaryW.m_singleAllocation = false;
	GenericMemoryPoolAllocator<U8> binaryAllocator = binaryW.m_alloc;
	binaryW.m_binary = binaryAllocator.newInstance<ShaderProgramBinary>();
	ShaderProgramBinary& binary = *binaryW.m_binary;
	binary = {};
	memcpy(&binary.m_magic[0], SHADER_BINARY_MAGIC, 8);

	// Parse source
	ShaderProgramParser parser(fname, &fsystem, tempAllocator, gpuCapabilities, bindlessLimits);
	ANKI_CHECK(parser.parse());

	if(postParseCallback && postParseCallback->skipCompilation(parser.getHash()))
	{
		return Error::NONE;
	}

	// Get mutators
	U32 mutationCount = 0;
	if(parser.getMutators().getSize() > 0)
	{
		binary.m_mutators.setArray(binaryAllocator.newArray<ShaderProgramBinaryMutator>(parser.getMutators().getSize()),
			parser.getMutators().getSize());

		for(U32 i = 0; i < binary.m_mutators.getSize(); ++i)
		{
			ShaderProgramBinaryMutator& out = binary.m_mutators[i];
			const ShaderProgramParserMutator& in = parser.getMutators()[i];

			ANKI_ASSERT(in.getName().getLength() < out.m_name.getSize());
			memcpy(&out.m_name[0], in.getName().cstr(), in.getName().getLength() + 1);

			out.m_values.setArray(binaryAllocator.newArray<I32>(in.getValues().getSize()), in.getValues().getSize());
			memcpy(out.m_values.getBegin(), in.getValues().getBegin(), in.getValues().getSizeInBytes());

			// Update the count
			mutationCount = (i == 0) ? out.m_values.getSize() : mutationCount * out.m_values.getSize();
		}
	}
	else
	{
		ANKI_ASSERT(binary.m_mutators.getSize() == 0);
	}

	// Create all variants
	if(parser.getMutators().getSize() > 0)
	{
		// Initialize
		DynamicArrayAuto<MutatorValue> originalMutationValues(tempAllocator, parser.getMutators().getSize());
		DynamicArrayAuto<MutatorValue> rewrittenMutationValues(tempAllocator, parser.getMutators().getSize());
		DynamicArrayAuto<U32> dials(tempAllocator, parser.getMutators().getSize(), 0);
		DynamicArrayAuto<ShaderProgramBinaryVariant> variants(binaryAllocator);
		DynamicArrayAuto<ShaderProgramBinaryCodeBlock> codeBlocks(binaryAllocator);
		DynamicArrayAuto<ShaderProgramBinaryMutation> mutations(binaryAllocator, mutationCount);
		DynamicArrayAuto<U64> codeBlockHashes(tempAllocator);
		HashMapAuto<U64, U32> mutationHashToIdx(tempAllocator);

		mutationCount = 0;

		// Spin for all possible combinations of mutators and
		// - Create the spirv
		// - Populate the binary variant
		do
		{
			// Create the mutation
			for(U32 i = 0; i < parser.getMutators().getSize(); ++i)
			{
				originalMutationValues[i] = parser.getMutators()[i].getValues()[dials[i]];
				rewrittenMutationValues[i] = originalMutationValues[i];
			}

			ShaderProgramBinaryMutation& mutation = mutations[mutationCount++];
			mutation.m_values.setArray(binaryAllocator.newArray<MutatorValue>(originalMutationValues.getSize()),
				originalMutationValues.getSize());
			memcpy(mutation.m_values.getBegin(),
				originalMutationValues.getBegin(),
				originalMutationValues.getSizeInBytes());

			mutation.m_hash = computeHash(originalMutationValues.getBegin(), originalMutationValues.getSizeInBytes());
			ANKI_ASSERT(mutation.m_hash > 0);

			const Bool rewritten = parser.rewriteMutation(
				WeakArray<MutatorValue>(rewrittenMutationValues.getBegin(), rewrittenMutationValues.getSize()));

			// Create the variant
			if(!rewritten)
			{
				// New and unique mutation and thus variant, add it

				ShaderProgramBinaryVariant& variant = *variants.emplaceBack();

				ANKI_CHECK(compileVariant(originalMutationValues,
					parser,
					variant,
					codeBlocks,
					codeBlockHashes,
					tempAllocator,
					binaryAllocator));

				mutation.m_variantIndex = variants.getSize() - 1;

				ANKI_ASSERT(mutationHashToIdx.find(mutation.m_hash) == mutationHashToIdx.getEnd());
				mutationHashToIdx.emplace(mutation.m_hash, mutationCount - 1);
			}
			else
			{
				// Check if the rewritten mutation exists
				const U64 otherMutationHash =
					computeHash(rewrittenMutationValues.getBegin(), rewrittenMutationValues.getSizeInBytes());
				auto it = mutationHashToIdx.find(otherMutationHash);

				ShaderProgramBinaryVariant* variant = nullptr;
				if(it == mutationHashToIdx.getEnd())
				{
					// Rewrite variant not found, create it

					variant = variants.emplaceBack();

					ANKI_CHECK(compileVariant(rewrittenMutationValues,
						parser,
						*variant,
						codeBlocks,
						codeBlockHashes,
						tempAllocator,
						binaryAllocator));

					ShaderProgramBinaryMutation& otherMutation = mutations[mutationCount++];
					otherMutation.m_values.setArray(
						binaryAllocator.newArray<MutatorValue>(rewrittenMutationValues.getSize()),
						rewrittenMutationValues.getSize());
					memcpy(otherMutation.m_values.getBegin(),
						rewrittenMutationValues.getBegin(),
						rewrittenMutationValues.getSizeInBytes());

					mutation.m_hash = otherMutationHash;
					mutation.m_variantIndex = variants.getSize() - 1;

					it = mutationHashToIdx.emplace(otherMutationHash, mutationCount - 1);
				}

				// Setup the new mutation
				mutation.m_variantIndex = mutations[*it].m_variantIndex;

				mutationHashToIdx.emplace(mutation.m_hash, U32(&mutation - mutations.getBegin()));
			}
		} while(!spinDials(dials, parser.getMutators()));

		ANKI_ASSERT(mutationCount == mutations.getSize());

		// Store temp containers to binary
		U32 size, storage;
		ShaderProgramBinaryVariant* firstVariant;
		variants.moveAndReset(firstVariant, size, storage);
		binary.m_variants.setArray(firstVariant, size);

		ShaderProgramBinaryCodeBlock* firstCodeBlock;
		codeBlocks.moveAndReset(firstCodeBlock, size, storage);
		binary.m_codeBlocks.setArray(firstCodeBlock, size);

		ShaderProgramBinaryMutation* firstMutation;
		mutations.moveAndReset(firstMutation, size, storage);
		binary.m_mutations.setArray(firstMutation, size);
	}
	else
	{
		DynamicArrayAuto<MutatorValue> mutation(tempAllocator);
		DynamicArrayAuto<ShaderProgramBinaryCodeBlock> codeBlocks(binaryAllocator);
		DynamicArrayAuto<U64> codeBlockHashes(tempAllocator);

		binary.m_variants.setArray(binaryAllocator.newInstance<ShaderProgramBinaryVariant>(), 1);

		ANKI_CHECK(compileVariant(
			mutation, parser, binary.m_variants[0], codeBlocks, codeBlockHashes, tempAllocator, binaryAllocator));
		ANKI_ASSERT(codeBlocks.getSize() == U32(__builtin_popcount(U32(parser.getShaderTypes()))));

		ShaderProgramBinaryCodeBlock* firstCodeBlock;
		U32 size, storage;
		codeBlocks.moveAndReset(firstCodeBlock, size, storage);
		binary.m_codeBlocks.setArray(firstCodeBlock, size);

		binary.m_mutations.setArray(binaryAllocator.newInstance<ShaderProgramBinaryMutation>(), 1);
		binary.m_mutations[0].m_hash = 1;
		binary.m_mutations[0].m_variantIndex = 0;
	}

	// Sort the mutations
	std::sort(binary.m_mutations.getBegin(),
		binary.m_mutations.getEnd(),
		[](const ShaderProgramBinaryMutation& a, const ShaderProgramBinaryMutation& b) { return a.m_hash < b.m_hash; });

	// Misc
	binary.m_presentShaderTypes = parser.getShaderTypes();

	// Reflection
	ANKI_CHECK(doReflection(binary, tempAllocator, binaryAllocator));

	return Error::NONE;
}

#define ANKI_TAB "    "

static void disassembleBlock(
	const ShaderProgramBinaryBlockInstance& instance, const ShaderProgramBinaryBlock& block, StringListAuto& lines)
{
	lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s set %4u binding %4u size %4u\n",
		block.m_name.getBegin(),
		block.m_set,
		block.m_binding,
		instance.m_size);

	for(U32 i = 0; i < instance.m_variables.getSize(); ++i)
	{
		const ShaderProgramBinaryVariableInstance& varInstance = instance.m_variables[i];
		const ShaderProgramBinaryVariable& var = block.m_variables[varInstance.m_index];

		lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB ANKI_TAB "%-48s type %8s blockInfo %d,%d,%d,%d\n",
			var.m_name.getBegin(),
			shaderVariableDataTypeToString(var.m_type).cstr(),
			varInstance.m_blockInfo.m_offset,
			varInstance.m_blockInfo.m_arraySize,
			varInstance.m_blockInfo.m_arrayStride,
			varInstance.m_blockInfo.m_matrixStride);
	}
}

void dumpShaderProgramBinary(const ShaderProgramBinary& binary, StringAuto& humanReadable)
{
	GenericMemoryPoolAllocator<U8> alloc = humanReadable.getAllocator();
	StringListAuto lines(alloc);

	lines.pushBack("**MUTATORS**\n");
	if(binary.m_mutators.getSize() > 0)
	{
		for(const ShaderProgramBinaryMutator& mutator : binary.m_mutators)
		{
			lines.pushBackSprintf(ANKI_TAB "%-32s ", &mutator.m_name[0]);
			for(U32 i = 0; i < mutator.m_values.getSize(); ++i)
			{
				lines.pushBackSprintf((i < mutator.m_values.getSize() - 1) ? "%d," : "%d", mutator.m_values[i]);
			}
			lines.pushBack("\n");
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**BINARIES**\n");
	U32 count = 0;
	for(const ShaderProgramBinaryCodeBlock& code : binary.m_codeBlocks)
	{
		spirv_cross::CompilerGLSL::Options options;
		options.vulkan_semantics = true;

		const unsigned int* spvb = reinterpret_cast<const unsigned int*>(code.m_binary.getBegin());
		ANKI_ASSERT((code.m_binary.getSize() % (sizeof(unsigned int))) == 0);
		std::vector<unsigned int> spv(spvb, spvb + code.m_binary.getSize() / sizeof(unsigned int));
		spirv_cross::CompilerGLSL compiler(spv);
		compiler.set_common_options(options);

		std::string glsl = compiler.compile();
		StringListAuto sourceLines(alloc);
		sourceLines.splitString(glsl.c_str(), '\n');
		StringAuto newGlsl(alloc);
		sourceLines.join("\n" ANKI_TAB ANKI_TAB, newGlsl);

		lines.pushBackSprintf(ANKI_TAB "#%u \n" ANKI_TAB ANKI_TAB "%s\n", count++, newGlsl.cstr());
	}

	lines.pushBack("\n**SHADER VARIANTS**\n");
	count = 0;
	for(const ShaderProgramBinaryVariant& variant : binary.m_variants)
	{
		lines.pushBackSprintf(ANKI_TAB "#%u\n", count++);

		// Uniform blocks
		if(variant.m_uniformBlocks.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Uniform blocks\n");
			for(const ShaderProgramBinaryBlockInstance& instance : variant.m_uniformBlocks)
			{
				disassembleBlock(instance, binary.m_uniformBlocks[instance.m_index], lines);
			}
		}

		// Storage blocks
		if(variant.m_storageBlocks.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Storage blocks\n");
			for(const ShaderProgramBinaryBlockInstance& instance : variant.m_storageBlocks)
			{
				disassembleBlock(instance, binary.m_storageBlocks[instance.m_index], lines);
			}
		}

		// Opaque
		if(variant.m_opaques.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Opaque\n");
			for(const ShaderProgramBinaryOpaqueInstance& instance : variant.m_opaques)
			{
				const ShaderProgramBinaryOpaque& o = binary.m_opaques[instance.m_index];
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s set %4u binding %4u type %12s arraySize %4u\n",
					o.m_name.getBegin(),
					o.m_set,
					o.m_binding,
					shaderVariableDataTypeToString(o.m_type).cstr(),
					instance.m_arraySize);
			}
		}

		// Push constants
		if(variant.m_pushConstantBlock)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Push constants\n");
			disassembleBlock(*variant.m_pushConstantBlock, *binary.m_pushConstantBlock, lines);
		}

		// Constants
		if(variant.m_constants.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Specialization constants\n");
			for(const ShaderProgramBinaryConstantInstance& instance : variant.m_constants)
			{
				const ShaderProgramBinaryConstant& c = binary.m_constants[instance.m_index];
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s type %8s id %4u\n",
					c.m_name.getBegin(),
					shaderVariableDataTypeToString(c.m_type).cstr(),
					c.m_constantId);
			}
		}

		// Binary indices
		lines.pushBack(ANKI_TAB ANKI_TAB "Binaries ");
		for(ShaderType shaderType : EnumIterable<ShaderType>())
		{
			if(variant.m_codeBlockIndices[shaderType] < MAX_U32)
			{
				lines.pushBackSprintf("%u", variant.m_codeBlockIndices[shaderType]);
			}
			else
			{
				lines.pushBack("-");
			}

			if(shaderType != ShaderType::LAST)
			{
				lines.pushBack(",");
			}
		}
		lines.pushBack("\n");
	}

	// Mutations
	lines.pushBack("\n**MUTATIONS**\n");
	count = 0;
	for(const ShaderProgramBinaryMutation& mutation : binary.m_mutations)
	{
		lines.pushBackSprintf(ANKI_TAB "#%-4u variantIndex %4u values (", count++, mutation.m_variantIndex);
		if(mutation.m_values.getSize() > 0)
		{
			for(U32 i = 0; i < mutation.m_values.getSize(); ++i)
			{
				lines.pushBackSprintf((i < mutation.m_values.getSize() - 1) ? "%s %4d, " : "%s %4d",
					binary.m_mutators[i].m_name.getBegin(),
					I32(mutation.m_values[i]));
			}

			lines.pushBack(")");
		}
		else
		{
			lines.pushBack("N/A");
		}

		lines.pushBack("\n");
	}

	lines.join("", humanReadable);
}

#undef ANKI_TAB

} // end namespace anki
