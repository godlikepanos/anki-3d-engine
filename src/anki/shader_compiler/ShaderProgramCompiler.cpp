// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramCompiler.h>
#include <anki/shader_compiler/ShaderProgramParser.h>
#include <anki/shader_compiler/Glslang.h>
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

	if(memcmp(SHADER_BINARY_MAGIC, &m_binary->m_magic[0], 0) != 0)
	{
		ANKI_SHADER_COMPILER_LOGE("Corrupted or wrong version of shader binary: %s", fname.cstr());
		return Error::USER_DATA;
	}

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
		m_alloc.getMemoryPool().free(m_binary->m_inputVariables.getBegin());

		for(ShaderProgramBinaryCode& code : m_binary->m_codeBlocks)
		{
			m_alloc.getMemoryPool().free(code.m_binary.getBegin());
		}
		m_alloc.getMemoryPool().free(m_binary->m_codeBlocks.getBegin());

		for(ShaderProgramBinaryVariant& variant : m_binary->m_variants)
		{
			m_alloc.getMemoryPool().free(variant.m_mutation.getBegin());
			m_alloc.getMemoryPool().free(variant.m_blockInfos.getBegin());
			m_alloc.getMemoryPool().free(variant.m_bindings.getBegin());
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
	DynamicArrayAuto<ShaderProgramBinaryCode>& codeBlocks,
	DynamicArrayAuto<U64>& codeBlockHashes,
	GenericMemoryPoolAllocator<U8> tmpAlloc,
	GenericMemoryPoolAllocator<U8> binaryAlloc)
{
	variant = {};

	// Generate the source and the rest for the variant
	ShaderProgramParserVariant parserVariant;
	ANKI_CHECK(parser.generateVariant(mutation, parserVariant));

	// Active vars
	{
		variant.m_activeVariables = ActiveProgramInputVariableMask(false);
		for(U32 i = 0; i < parser.getInputs().getSize(); ++i)
		{
			if(parserVariant.isInputActive(parser.getInputs()[i]))
			{
				variant.m_activeVariables.set(i, true);
			}
		}
	}

	// Compile stages
	for(ShaderType shaderType = ShaderType::FIRST; shaderType < ShaderType::COUNT; ++shaderType)
	{
		if(!(shaderTypeToBit(shaderType) & parser.getShaderTypes()))
		{
			variant.m_binaryIndices[shaderType] = MAX_U32;
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
				variant.m_binaryIndices[shaderType] = i;
				found = true;
				break;
			}
		}

		// Create it if not found
		if(!found)
		{
			U8* code = binaryAlloc.allocate(spirv.getSizeInBytes());
			memcpy(code, &spirv[0], spirv.getSizeInBytes());

			ShaderProgramBinaryCode block;
			block.m_binary.setArray(code, spirv.getSizeInBytes());
			codeBlocks.emplaceBack(block);

			codeBlockHashes.emplaceBack(newHash);

			variant.m_binaryIndices[shaderType] = codeBlocks.getSize() - 1;
		}
	}

	// Mutator values
	variant.m_mutation.setArray(
		binaryAlloc.newArray<MutatorValue>(parser.getMutators().getSize()), parser.getMutators().getSize());
	for(U32 i = 0; i < parser.getMutators().getSize(); ++i)
	{
		variant.m_mutation[i] = mutation[i];
	}

	// Input vars
	{
		ShaderVariableBlockInfo defaultInfo;
		defaultInfo.m_arraySize = -1;
		defaultInfo.m_arrayStride = -1;
		defaultInfo.m_matrixStride = -1;
		defaultInfo.m_offset = -1;
		variant.m_blockInfos.setArray(
			binaryAlloc.newArray<ShaderVariableBlockInfo>(parser.getInputs().getSize(), defaultInfo),
			parser.getInputs().getSize());

		variant.m_bindings.setArray(
			binaryAlloc.newArray<I16>(parser.getInputs().getSize(), -1), parser.getInputs().getSize());

		for(U32 i = 0; i < parser.getInputs().getSize(); ++i)
		{
			const ShaderProgramParserInput& parserInput = parser.getInputs()[i];
			if(!parserVariant.isInputActive(parserInput))
			{
				continue;
			}

			if(parserInput.inUbo())
			{
				variant.m_blockInfos[i] = parserVariant.getBlockInfo(parserInput);
			}

			if(parserInput.isSampler() || parserInput.isTexture())
			{
				variant.m_bindings[i] = I16(parserVariant.getBinding(parserInput));
			}
		}
	}

	// Misc
	variant.m_blockSize = parserVariant.getBlockSize();
	variant.m_usesPushConstants = parserVariant.usesPushConstants();

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

	// Inputs
	if(parser.getInputs().getSize() > 0)
	{
		binary.m_inputVariables.setArray(
			binaryAllocator.newArray<ShaderProgramBinaryInput>(parser.getInputs().getSize()),
			parser.getInputs().getSize());

		for(U32 i = 0; i < binary.m_inputVariables.getSize(); ++i)
		{
			ShaderProgramBinaryInput& out = binary.m_inputVariables[i];
			const ShaderProgramParserInput& in = parser.getInputs()[i];

			ANKI_ASSERT(in.getName().getLength() < out.m_name.getSize());
			memcpy(&out.m_name[0], in.getName().cstr(), in.getName().getLength() + 1);

			out.m_firstSpecializationConstantIndex = MAX_U32;
			in.isConstant(&out.m_firstSpecializationConstantIndex);

			out.m_instanced = in.isInstanced();
			out.m_dataType = in.getDataType();
		}
	}
	else
	{
		ANKI_ASSERT(binary.m_inputVariables.getSize() == 0);
	}

	// Mutators
	U32 variantCount = 0;
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

			out.m_instanceCount = in.isInstanceCount();

			// Update the count
			variantCount = (i == 0) ? out.m_values.getSize() : variantCount * out.m_values.getSize();
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
		DynamicArrayAuto<MutatorValue> mutation(tempAllocator, parser.getMutators().getSize());
		DynamicArrayAuto<MutatorValue> mutation2(tempAllocator, parser.getMutators().getSize());
		DynamicArrayAuto<U32> dials(tempAllocator, parser.getMutators().getSize(), 0);
		DynamicArrayAuto<ShaderProgramBinaryVariant> variants(binaryAllocator, variantCount);
		DynamicArrayAuto<ShaderProgramBinaryCode> codeBlocks(binaryAllocator);
		DynamicArrayAuto<U64> codeBlockHashes(tempAllocator);
		HashMapAuto<U64, U32> mutationToVariantIdx(tempAllocator);

		variantCount = 0;

		// Spin for all possible combinations of mutators and
		// - Create the spirv
		// - Populate the binary variant
		do
		{
			// Create the mutation
			for(U32 i = 0; i < parser.getMutators().getSize(); ++i)
			{
				mutation[i] = parser.getMutators()[i].getValues()[dials[i]];
				mutation2[i] = mutation[i];
			}
			const Bool rewritten =
				parser.rewriteMutation(WeakArray<MutatorValue>(mutation.getBegin(), mutation.getSize()));

			// Create the variant
			ShaderProgramBinaryVariant& variant = variants[variantCount++];
			if(!rewritten)
			{
				// New and unique variant, add it
				ANKI_CHECK(compileVariant(
					mutation, parser, variant, codeBlocks, codeBlockHashes, tempAllocator, binaryAllocator));

				mutationToVariantIdx.emplace(computeHash(&mutation[0], mutation.getSizeInBytes()), variantCount - 1);
			}
			else
			{
				// Check if the original variant exists
				auto it = mutationToVariantIdx.find(computeHash(&mutation[0], mutation.getSizeInBytes()));
				U32 originalVariantIdx = (it != mutationToVariantIdx.getEnd()) ? *it : MAX_U32;

				if(originalVariantIdx == MAX_U32)
				{
					// Original variant not found, create it

					ShaderProgramBinaryVariant& other = variants[variantCount++];
					originalVariantIdx = variantCount - 1;

					ANKI_CHECK(compileVariant(
						mutation, parser, other, codeBlocks, codeBlockHashes, tempAllocator, binaryAllocator));

					mutationToVariantIdx.emplace(
						computeHash(&mutation[0], mutation.getSizeInBytes()), originalVariantIdx);
				}

				// Copy the original variant to the current variant
				{
					ANKI_ASSERT(originalVariantIdx < variantCount);
					const ShaderProgramBinaryVariant& other = variants[originalVariantIdx];

					variant = other;

					variant.m_mutation.setArray(
						binaryAllocator.newArray<MutatorValue>(other.m_mutation.getSize()), other.m_mutation.getSize());
					memcpy(
						variant.m_mutation.getBegin(), other.m_mutation.getBegin(), other.m_mutation.getSizeInBytes());

					if(other.m_blockInfos.getSize())
					{
						variant.m_blockInfos.setArray(
							binaryAllocator.newArray<ShaderVariableBlockInfo>(other.m_blockInfos.getSize()),
							other.m_blockInfos.getSize());
						memcpy(variant.m_blockInfos.getBegin(),
							other.m_blockInfos.getBegin(),
							other.m_blockInfos.getSizeInBytes());

						variant.m_bindings.setArray(
							binaryAllocator.newArray<I16>(other.m_bindings.getSize()), other.m_bindings.getSize());
						memcpy(variant.m_bindings.getBegin(),
							other.m_bindings.getBegin(),
							other.m_bindings.getSizeInBytes());
					}

					mutationToVariantIdx.emplace(
						computeHash(&mutation2[0], mutation2.getSizeInBytes()), U32(&variant - &variants[0]));
				}
			}
		} while(!spinDials(dials, parser.getMutators()));

		ANKI_ASSERT(variantCount == variants.getSize());

		// Store to binary
		U32 size, storage;
		ShaderProgramBinaryVariant* firstVariant;
		variants.moveAndReset(firstVariant, size, storage);
		binary.m_variants.setArray(firstVariant, size);

		ShaderProgramBinaryCode* firstCodeBlock;
		codeBlocks.moveAndReset(firstCodeBlock, size, storage);
		binary.m_codeBlocks.setArray(firstCodeBlock, size);
	}
	else
	{
		DynamicArrayAuto<MutatorValue> mutation(tempAllocator);
		DynamicArrayAuto<ShaderProgramBinaryCode> codeBlocks(binaryAllocator);
		DynamicArrayAuto<U64> codeBlockHashes(tempAllocator);

		binary.m_variants.setArray(binaryAllocator.newInstance<ShaderProgramBinaryVariant>(), 1);

		ANKI_CHECK(compileVariant(
			mutation, parser, binary.m_variants[0], codeBlocks, codeBlockHashes, tempAllocator, binaryAllocator));
		ANKI_ASSERT(codeBlocks.getSize() == U32(__builtin_popcount(U32(parser.getShaderTypes()))));

		ShaderProgramBinaryCode* firstCodeBlock;
		U32 size, storage;
		codeBlocks.moveAndReset(firstCodeBlock, size, storage);
		binary.m_codeBlocks.setArray(firstCodeBlock, size);
	}

	// Misc
	binary.m_descriptorSet = parser.getDescritproSet();
	binary.m_presentShaderTypes = parser.getShaderTypes();

	return Error::NONE;
}

void disassembleShaderProgramBinary(const ShaderProgramBinary& binary, StringAuto& humanReadable)
{
#define ANKI_TAB "    "

	GenericMemoryPoolAllocator<U8> alloc = humanReadable.getAllocator();
	StringListAuto lines(alloc);

	lines.pushBack("**MUTATORS**\n");
	if(binary.m_mutators.getSize() > 0)
	{
		for(const ShaderProgramBinaryMutator& mutator : binary.m_mutators)
		{
			lines.pushBackSprintf(ANKI_TAB "\"%s\"", &mutator.m_name[0]);
			for(MutatorValue value : mutator.m_values)
			{
				lines.pushBackSprintf(" %d", value);
			}
			lines.pushBack("\n");
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**INPUT VARIABLES**\n");
	if(binary.m_inputVariables.getSize() > 0)
	{
		for(const ShaderProgramBinaryInput& input : binary.m_inputVariables)
		{
			lines.pushBackSprintf(ANKI_TAB "\"%s\" ", &input.m_name[0]);
			if(input.m_firstSpecializationConstantIndex < MAX_U32)
			{
				lines.pushBackSprintf(
					"firstSpecializationConstant %" PRIu32 " ", input.m_firstSpecializationConstantIndex);
			}
			lines.pushBackSprintf("instanced %" PRIu32 " ", U32(input.m_instanced));
			lines.pushBackSprintf("dataType %" PRIu8 "\n", U8(input.m_dataType));
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**BINARIES**\n");
	U32 count = 0;
	for(const ShaderProgramBinaryCode& code : binary.m_codeBlocks)
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

		lines.pushBackSprintf(ANKI_TAB "%" PRIuFAST32 " \n" ANKI_TAB ANKI_TAB "%s\n", count++, newGlsl.cstr());
	}

	lines.pushBack("\n**SHADER VARIANTS**\n");
	count = 0;
	for(const ShaderProgramBinaryVariant& variant : binary.m_variants)
	{
		lines.pushBackSprintf(ANKI_TAB "%" PRIuFAST32 "\n", count++);

		// Misc
		ANKI_ASSERT(variant.m_activeVariables.getData().getSize() == 2);
		lines.pushBackSprintf(ANKI_TAB ANKI_TAB "activeVariables 0b%" ANKI_PRIb64 " 0b%" ANKI_PRIb64 " ",
			ANKI_FORMAT_U64(variant.m_activeVariables.getData()[1]),
			ANKI_FORMAT_U64(variant.m_activeVariables.getData()[0]));
		lines.pushBackSprintf(
			"blockSize %" PRIu32 " usesPushConstants %" PRIu8 "\n", variant.m_blockSize, variant.m_usesPushConstants);

		// Mutator values
		lines.pushBack(ANKI_TAB ANKI_TAB "mutatorValues ");
		if(variant.m_mutation.getSize() > 0)
		{
			for(U32 j = 0; j < variant.m_mutation.getSize(); ++j)
			{
				lines.pushBackSprintf("\"%s\" %" PRId32 " ", &binary.m_mutators[j].m_name[0], variant.m_mutation[j]);
			}
		}
		else
		{
			lines.pushBack("N/A");
		}
		lines.pushBack("\n");

		// Block infos
		lines.pushBack(ANKI_TAB ANKI_TAB "blockInfos ");
		if(variant.m_blockInfos.getSize() > 0)
		{
			for(const ShaderVariableBlockInfo& inf : variant.m_blockInfos)
			{
				lines.pushBackSprintf("%" PRIi16 "|%" PRIi16 "|%" PRIi16 "|%" PRIi16 " ",
					inf.m_offset,
					inf.m_arraySize,
					inf.m_arrayStride,
					inf.m_matrixStride);
			}
		}
		else
		{
			lines.pushBack("N/A");
		}
		lines.pushBack("\n");

		// Bindings
		lines.pushBack(ANKI_TAB ANKI_TAB "bindings ");
		if(variant.m_bindings.getSize() > 0)
		{
			for(I32 binding : variant.m_bindings)
			{
				if(binding < 0)
				{
					lines.pushBack("N/A ");
				}
				else
				{
					lines.pushBackSprintf("%" PRIi32 " ", binding);
				}
			}
		}
		else
		{
			lines.pushBack("N/A");
		}
		lines.pushBack("\n");

		// Binary indices
		lines.pushBack(ANKI_TAB ANKI_TAB "binaries ");
		for(ShaderType shaderType = ShaderType::FIRST; shaderType < ShaderType::COUNT; ++shaderType)
		{
			if(variant.m_binaryIndices[shaderType] < MAX_U32)
			{
				lines.pushBackSprintf("%" PRIu32 " ", variant.m_binaryIndices[shaderType]);
			}
			else
			{
				lines.pushBack("N/A ");
			}
		}
		lines.pushBack("\n");
	}

	lines.join("", humanReadable);

#undef ANKI_TAB
}

} // end namespace anki
