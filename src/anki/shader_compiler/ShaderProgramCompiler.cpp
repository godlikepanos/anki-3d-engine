// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

		for(ShaderProgramBinaryVariant& variant : m_binary->m_variants)
		{
			m_alloc.getMemoryPool().free(variant.m_reflection.m_uniformBlocks.getBegin());
			m_alloc.getMemoryPool().free(variant.m_reflection.m_storageBlocks.getBegin());
			m_alloc.getMemoryPool().free(variant.m_reflection.m_pushConstantBlock);
			m_alloc.getMemoryPool().free(variant.m_reflection.m_specializationConstants.getBegin());
			m_alloc.getMemoryPool().free(variant.m_reflection.m_opaques.getBegin());
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
	GenericMemoryPoolAllocator<U8> tmpAlloc,
	GenericMemoryPoolAllocator<U8> binaryAlloc)
{
	variant = {};

	// Generate the source and the rest for the variant
	ShaderProgramParserVariant parserVariant;
	ANKI_CHECK(parser.generateVariant(mutation, parserVariant));

	// Compile stages
	Array<ConstWeakArray<U8, PtrSize>, U32(ShaderType::COUNT)> spirvBinaries;
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
			block.m_binary.setArray(code, spirv.getSizeInBytes());
			codeBlocks.emplaceBack(block);

			codeBlockHashes.emplaceBack(newHash);

			variant.m_codeBlockIndices[shaderType] = codeBlocks.getSize() - 1;
		}

		spirvBinaries[shaderType] = codeBlocks[variant.m_codeBlockIndices[shaderType]].m_binary;
	}

	// Do reflection
	ANKI_CHECK(performSpirvReflection(variant.m_reflection, spirvBinaries, tmpAlloc, binaryAlloc));

	return Error::NONE;
}

Error compileShaderProgram(CString fname,
	ShaderProgramFilesystemInterface& fsystem,
	GenericMemoryPoolAllocator<U8> tempAllocator,
	U32 pushConstantsSize,
	U32 backendMinor,
	U32 backendMajor,
	GpuVendor gpuVendor,
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
	ShaderProgramParser parser(
		fname, &fsystem, tempAllocator, pushConstantsSize, backendMinor, backendMajor, gpuVendor);
	ANKI_CHECK(parser.parse());

	// Mutators
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

	// Misc
	binary.m_presentShaderTypes = parser.getShaderTypes();

	return Error::NONE;
}

#define ANKI_TAB "    "

static void disassembleBlock(const ShaderProgramBinaryBlock& block, StringListAuto& lines)
{
	lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s set %4u binding %4u size %4u\n",
		block.m_name.getBegin(),
		block.m_set,
		block.m_binding,
		block.m_size);

	for(const ShaderProgramBinaryVariable& var : block.m_variables)
	{
		lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB ANKI_TAB
			"%-28s type: %8s active: %4s blockInfo: %d,%d,%d,%d\n",
			var.m_name.getBegin(),
			shaderVariableDataTypeToString(var.m_type).cstr(),
			var.m_active ? "true" : "false",
			var.m_blockInfo.m_offset,
			var.m_blockInfo.m_arraySize,
			var.m_blockInfo.m_arrayStride,
			var.m_blockInfo.m_matrixStride);
	}
}

void disassembleShaderProgramBinary(const ShaderProgramBinary& binary, StringAuto& humanReadable)
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
		if(variant.m_reflection.m_uniformBlocks.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Uniform blocks\n");
			for(const ShaderProgramBinaryBlock& block : variant.m_reflection.m_uniformBlocks)
			{
				disassembleBlock(block, lines);
			}
		}

		// Storage blocks
		if(variant.m_reflection.m_storageBlocks.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Storage blocks\n");
			for(const ShaderProgramBinaryBlock& block : variant.m_reflection.m_storageBlocks)
			{
				disassembleBlock(block, lines);
			}
		}

		// Opaque
		if(variant.m_reflection.m_opaques.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Opaque\n");
			for(const ShaderProgramBinaryOpaque& o : variant.m_reflection.m_opaques)
			{
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s set %4u binding %4u type %8s arraySize %4u\n",
					o.m_name.getBegin(),
					o.m_set,
					o.m_binding,
					shaderVariableDataTypeToString(o.m_type).cstr(),
					o.m_arraySize);
			}
		}

		// Push constants
		if(variant.m_reflection.m_pushConstantBlock)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Push constants\n");
			disassembleBlock(*variant.m_reflection.m_pushConstantBlock, lines);
		}

		// Constants
		if(variant.m_reflection.m_specializationConstants.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Specialization constants\n");
			for(const ShaderProgramBinaryConstant& c : variant.m_reflection.m_specializationConstants)
			{
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
