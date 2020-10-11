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

namespace anki
{

static const char* SHADER_BINARY_MAGIC = "ANKISDR4"; ///< @warning If changed change SHADER_BINARY_VERSION
const U32 SHADER_BINARY_VERSION = 4;

Error ShaderProgramBinaryWrapper::serializeToFile(CString fname) const
{
	ANKI_ASSERT(m_binary);

	File file;
	ANKI_CHECK(file.open(fname, FileOpenFlag::WRITE | FileOpenFlag::BINARY));

	BinarySerializer serializer;
	HeapAllocator<U8> tmpAlloc(m_alloc.getMemoryPool().getAllocationCallback(),
							   m_alloc.getMemoryPool().getAllocationCallbackUserData());
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

	if(memcmp(SHADER_BINARY_MAGIC, &m_binary->m_magic[0], strlen(SHADER_BINARY_MAGIC)) != 0)
	{
		ANKI_SHADER_COMPILER_LOGE("Corrupted or wrong version of shader binary: %s. Clean the shader cache",
								  fname.cstr());
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

static Error compileSpirv(ConstWeakArray<MutatorValue> mutation, const ShaderProgramParser& parser,
						  GenericMemoryPoolAllocator<U8>& tmpAlloc,
						  Array<DynamicArrayAuto<U8>, U32(ShaderType::COUNT)>& spirv)
{
	// Generate the source and the rest for the variant
	ShaderProgramParserVariant parserVariant;
	ANKI_CHECK(parser.generateVariant(mutation, parserVariant));

	// Compile stages
	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		if(!(ShaderTypeBit(1 << shaderType) & parser.getShaderTypes()))
		{
			continue;
		}

		// Compile
		ANKI_CHECK(compilerGlslToSpirv(parserVariant.getSource(shaderType), shaderType, tmpAlloc, spirv[shaderType]));
		ANKI_ASSERT(spirv[shaderType].getSize() > 0);
	}

	return Error::NONE;
}

static void compileVariantAsync(ConstWeakArray<MutatorValue> mutation, const ShaderProgramParser& parser,
								ShaderProgramBinaryVariant& variant,
								DynamicArrayAuto<ShaderProgramBinaryCodeBlock>& codeBlocks,
								DynamicArrayAuto<U64>& codeBlockHashes, GenericMemoryPoolAllocator<U8>& tmpAlloc,
								GenericMemoryPoolAllocator<U8>& binaryAlloc,
								ShaderProgramAsyncTaskInterface& taskManager, Mutex& mtx, Atomic<I32>& error)
{
	variant = {};

	class Ctx
	{
	public:
		GenericMemoryPoolAllocator<U8> m_tmpAlloc;
		GenericMemoryPoolAllocator<U8> m_binaryAlloc;
		DynamicArrayAuto<MutatorValue> m_mutation = {m_tmpAlloc};
		const ShaderProgramParser* m_parser;
		ShaderProgramBinaryVariant* m_variant;
		DynamicArrayAuto<ShaderProgramBinaryCodeBlock>* m_codeBlocks;
		DynamicArrayAuto<U64>* m_codeBlockHashes;
		Mutex* m_mtx;
		Atomic<I32>* m_err;

		Ctx(GenericMemoryPoolAllocator<U8> tmpAlloc)
			: m_tmpAlloc(tmpAlloc)
		{
		}
	};

	Ctx* ctx = tmpAlloc.newInstance<Ctx>(tmpAlloc);
	ctx->m_binaryAlloc = binaryAlloc;
	ctx->m_mutation.create(mutation.getSize());
	memcpy(ctx->m_mutation.getBegin(), mutation.getBegin(), mutation.getSizeInBytes());
	ctx->m_parser = &parser;
	ctx->m_variant = &variant;
	ctx->m_codeBlocks = &codeBlocks;
	ctx->m_codeBlockHashes = &codeBlockHashes;
	ctx->m_mtx = &mtx;
	ctx->m_err = &error;

	auto callback = [](void* userData) {
		Ctx& ctx = *static_cast<Ctx*>(userData);
		GenericMemoryPoolAllocator<U8>& tmpAlloc = ctx.m_tmpAlloc;

		if(ctx.m_err->load() != 0)
		{
			// Cleanup and return
			tmpAlloc.deleteInstance(&ctx);
			return;
		}

		// All good, compile the variant
		Array<DynamicArrayAuto<U8>, U32(ShaderType::COUNT)> spirvs = {{{tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc},
																	   {tmpAlloc}}};
		const Error err = compileSpirv(ctx.m_mutation, *ctx.m_parser, tmpAlloc, spirvs);

		if(!err)
		{
			// No error, check if the spirvs are common with some other variant and store it

			LockGuard<Mutex> lock(*ctx.m_mtx);

			for(ShaderType shaderType : EnumIterable<ShaderType>())
			{
				DynamicArrayAuto<U8>& spirv = spirvs[shaderType];

				if(spirv.isEmpty())
				{
					ctx.m_variant->m_codeBlockIndices[shaderType] = MAX_U32;
					continue;
				}

				// Check if the spirv is already generated
				const U64 newHash = computeHash(&spirv[0], spirv.getSize());
				Bool found = false;
				for(U32 i = 0; i < ctx.m_codeBlockHashes->getSize(); ++i)
				{
					if((*ctx.m_codeBlockHashes)[i] == newHash)
					{
						// Found it
						ctx.m_variant->m_codeBlockIndices[shaderType] = i;
						found = true;
						break;
					}
				}

				// Create it if not found
				if(!found)
				{
					U8* code = ctx.m_binaryAlloc.allocate(spirv.getSizeInBytes());
					memcpy(code, &spirv[0], spirv.getSizeInBytes());

					ShaderProgramBinaryCodeBlock block;
					block.m_binary.setArray(code, U32(spirv.getSizeInBytes()));
					block.m_hash = newHash;

					ctx.m_codeBlocks->emplaceBack(block);
					ctx.m_codeBlockHashes->emplaceBack(newHash);

					ctx.m_variant->m_codeBlockIndices[shaderType] = ctx.m_codeBlocks->getSize() - 1;
				}
			}
		}
		else
		{
			ctx.m_err->store(err._getCode());
		}

		// Cleanup
		tmpAlloc.deleteInstance(&ctx);
	};

	taskManager.enqueueTask(callback, ctx);
}

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

	Array<U32, 3> m_workgroupSizes = {MAX_U32, MAX_U32, MAX_U32};
	Array<U32, 3> m_workgroupSizesConstants = {MAX_U32, MAX_U32, MAX_U32};
	/// @}

	Refl(const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	Error setWorkgroupSizes(U32 x, U32 y, U32 z, U32 specConstMask) final
	{
		m_workgroupSizesConstants = {MAX_U32, MAX_U32, MAX_U32};
		m_workgroupSizes = {MAX_U32, MAX_U32, MAX_U32};
		const Array<U32, 3> input = {x, y, z};

		for(U32 i = 0; i < 3; ++i)
		{
			if(specConstMask & (1 << i))
			{
				for(const ShaderProgramBinaryConstantInstance& c : m_constInstances)
				{
					if(m_consts[c.m_index].m_constantId == input[i])
					{
						m_workgroupSizesConstants[i] = c.m_index;
						break;
					}
				}

				if(m_workgroupSizesConstants[i] == MAX_U32)
				{
					ANKI_SHADER_COMPILER_LOGE("Reflection identified workgroup size dimension %u as spec constant but "
											  "not such spec constant was found",
											  i);
					return Error::USER_DATA;
				}
			}
			else
			{
				m_workgroupSizes[i] = input[i];
			}
		}

		return Error::NONE;
	}

	Error setCounts(U32 uniformBlockCount, U32 storageBlockCount, U32 opaqueCount, Bool pushConstantBlock,
					U32 constCount) final
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

	Error visitUniformVariable(U32 blockIdx, U32 idx, CString name, ShaderVariableDataType type,
							   const ShaderVariableBlockInfo& blockInfo) final
	{
		return visitAnyVariable(blockIdx, idx, name, type, blockInfo, 0);
	}

	Error visitStorageBlock(U32 idx, CString name, U32 set, U32 binding, U32 size, U32 varCount) final
	{
		return visitAnyBlock(idx, name, set, binding, size, varCount, 1);
	}

	Error visitStorageVariable(U32 blockIdx, U32 idx, CString name, ShaderVariableDataType type,
							   const ShaderVariableBlockInfo& blockInfo) final
	{
		return visitAnyVariable(blockIdx, idx, name, type, blockInfo, 1);
	}

	Error visitPushConstantsBlock(CString name, U32 size, U32 varCount) final
	{
		return visitAnyBlock(0, name, 0, 0, size, varCount, 2);
	}

	Error visitPushConstant(U32 idx, CString name, ShaderVariableDataType type,
							const ShaderVariableBlockInfo& blockInfo) final
	{
		return visitAnyVariable(0, idx, name, type, blockInfo, 2);
	}

	Error visitOpaque(U32 instanceIdx, CString name, ShaderVariableDataType type, U32 set, U32 binding,
					  U32 arraySize) final
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

	static ANKI_USE_RESULT Error findBlock(CString name, U32 set, U32 binding,
										   ConstWeakArray<ShaderProgramBinaryBlock> arr, U32& idx)
	{
		idx = MAX_U32;

		for(U32 i = 0; i < arr.getSize(); ++i)
		{
			const ShaderProgramBinaryBlock& block = arr[i];
			if(block.m_name.getBegin() == name)
			{
				if(set != block.m_set || binding != block.m_binding)
				{
					ANKI_SHADER_COMPILER_LOGE("The set and binding can't difer between shader variants for block: %s",
											  name.cstr());
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

	Error visitAnyVariable(U32 blockInstanceIdx, U32 varInstanceIdx, CString name, ShaderVariableDataType type,
						   const ShaderVariableBlockInfo& blockInfo, U32 blockType)
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
					ANKI_SHADER_COMPILER_LOGE("The type should not differ between variants for variable: %s",
											  name.cstr());
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

static Error doReflection(ShaderProgramBinary& binary, GenericMemoryPoolAllocator<U8>& tmpAlloc,
						  GenericMemoryPoolAllocator<U8>& binaryAlloc)
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

		variant.m_workgroupSizes = refl.m_workgroupSizes;
		variant.m_workgroupSizesConstants = refl.m_workgroupSizesConstants;
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

Error compileShaderProgramInternal(CString fname, ShaderProgramFilesystemInterface& fsystem,
								   ShaderProgramPostParseInterface* postParseCallback,
								   ShaderProgramAsyncTaskInterface* taskManager_,
								   GenericMemoryPoolAllocator<U8> tempAllocator,
								   const GpuDeviceCapabilities& gpuCapabilities, const BindlessLimits& bindlessLimits,
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
	Mutex mtx;
	Atomic<I32> errorAtomic(0);
	class SyncronousShaderProgramAsyncTaskInterface : public ShaderProgramAsyncTaskInterface
	{
	public:
		void enqueueTask(void (*callback)(void* userData), void* userData) final
		{
			callback(userData);
		}

		Error joinTasks() final
		{
			// Nothing
			return Error::NONE;
		}
	} syncTaskManager;
	ShaderProgramAsyncTaskInterface& taskManager = (taskManager_) ? *taskManager_ : syncTaskManager;

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

		// Grow the storage of the variants array. Can't have it resize, threads will work on stale data
		variants.resizeStorage(mutationCount);
		const ShaderProgramBinaryVariant* baseVariant = nullptr;

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
			memcpy(mutation.m_values.getBegin(), originalMutationValues.getBegin(),
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
				baseVariant = (baseVariant == nullptr) ? variants.getBegin() : baseVariant;

				compileVariantAsync(originalMutationValues, parser, variant, codeBlocks, codeBlockHashes, tempAllocator,
									binaryAllocator, taskManager, mtx, errorAtomic);

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
					baseVariant = (baseVariant == nullptr) ? variants.getBegin() : baseVariant;

					compileVariantAsync(originalMutationValues, parser, *variant, codeBlocks, codeBlockHashes,
										tempAllocator, binaryAllocator, taskManager, mtx, errorAtomic);

					ShaderProgramBinaryMutation& otherMutation = mutations[mutationCount++];
					otherMutation.m_values.setArray(
						binaryAllocator.newArray<MutatorValue>(rewrittenMutationValues.getSize()),
						rewrittenMutationValues.getSize());
					memcpy(otherMutation.m_values.getBegin(), rewrittenMutationValues.getBegin(),
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
		ANKI_ASSERT(baseVariant == variants.getBegin() && "Can't have the variants array grow");

		// Done, wait the threads
		ANKI_CHECK(taskManager.joinTasks());
		ANKI_CHECK(Error(errorAtomic.getNonAtomically()));

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

		compileVariantAsync(mutation, parser, binary.m_variants[0], codeBlocks, codeBlockHashes, tempAllocator,
							binaryAllocator, taskManager, mtx, errorAtomic);

		ANKI_CHECK(taskManager.joinTasks());
		ANKI_CHECK(Error(errorAtomic.getNonAtomically()));

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
	std::sort(
		binary.m_mutations.getBegin(), binary.m_mutations.getEnd(),
		[](const ShaderProgramBinaryMutation& a, const ShaderProgramBinaryMutation& b) { return a.m_hash < b.m_hash; });

	// Lib name
	if(parser.getLibraryName().getLength() > 0)
	{
		if(parser.getLibraryName().getLength() >= sizeof(binary.m_libraryName))
		{
			ANKI_SHADER_COMPILER_LOGE("Library name too long: %s", parser.getLibraryName().cstr());
			return Error::USER_DATA;
		}

		memcpy(&binary.m_libraryName[0], &parser.getLibraryName()[0], parser.getLibraryName().getLength());
	}

	binary.m_rayType = parser.getRayType();

	// Misc
	binary.m_presentShaderTypes = parser.getShaderTypes();

	// Reflection
	ANKI_CHECK(doReflection(binary, tempAllocator, binaryAllocator));

	return Error::NONE;
}

Error compileShaderProgram(CString fname, ShaderProgramFilesystemInterface& fsystem,
						   ShaderProgramPostParseInterface* postParseCallback,
						   ShaderProgramAsyncTaskInterface* taskManager, GenericMemoryPoolAllocator<U8> tempAllocator,
						   const GpuDeviceCapabilities& gpuCapabilities, const BindlessLimits& bindlessLimits,
						   ShaderProgramBinaryWrapper& binaryW)
{
	const Error err = compileShaderProgramInternal(fname, fsystem, postParseCallback, taskManager, tempAllocator,
												   gpuCapabilities, bindlessLimits, binaryW);
	if(err)
	{
		ANKI_SHADER_COMPILER_LOGE("Failed to compile: %s", fname.cstr());
	}

	return err;
}

} // end namespace anki
