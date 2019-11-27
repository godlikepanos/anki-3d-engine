// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramCompiler.h>
#include <anki/shader_compiler/ShaderProgramParser.h>
#include <anki/shader_compiler/Glslang.h>
#include <anki/util/Serializer.h>

namespace anki
{

/// XXX
static Bool spinDials(DynamicArrayAuto<U32> dials, ConstWeakArray<ShaderProgramParserMutator> mutators)
{
	// XXX
	return false;
}

Error ShaderProgramBinaryWrapper::serializeToFile(CString fname) const
{
	ANKI_ASSERT(m_binary);

	File file;
	ANKI_CHECK(file.open(fname, FileOpenFlag::WRITE | FileOpenFlag::BINARY));

	BinarySerializer serializer;
	ANKI_CHECK(serializer.serialize(*m_binary, m_alloc, file));

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

Error ShaderProgramCompiler::compile(CString fname,
	ShaderProgramFilesystemInterface* fsystem,
	GenericMemoryPoolAllocator<U8> tempAllocator,
	U32 pushConstantsSize,
	U32 backendMinor,
	U32 backendMajor,
	GpuVendor gpuVendor,
	ShaderProgramBinaryWrapper& binary)
{
	ANKI_ASSERT(fsystem);

	binary.cleanup();
	GenericMemoryPoolAllocator<U8> binaryAllocator;

	// Parse source
	ShaderProgramParser parser(fname, fsystem, tempAllocator, pushConstantsSize, backendMinor, backendMajor, gpuVendor);
	ANKI_CHECK(parser.parse());

	// Create all variants
	if(parser.getMutators().getSize() > 0)
	{
		// Initialize
		DynamicArrayAuto<ShaderProgramParserMutatorState> states(tempAllocator, parser.getMutators().getSize());
		DynamicArrayAuto<U32> dials(tempAllocator, parser.getMutators().getSize());
		for(PtrSize i = 0; i < parser.getMutators().getSize(); ++i)
		{
			states[i].m_mutator = &parser.getMutators()[i];
			states[i].m_value = parser.getMutators()[i].getValues()[0];

			dials[i] = 0;
		}
		DynamicArrayAuto<ShaderProgramBinaryVariant> variants(binaryAllocator);
		DynamicArrayAuto<ShaderProgramBinaryCode> codeBlocks(binaryAllocator);
		DynamicArrayAuto<U64> codeBlockHashes(tempAllocator);

		// Spin for all possible combinations of mutators and
		// - Create the spirv
		// - Populate the binary variant
		ShaderProgramParserVariant parserVariant;
		do
		{
			ShaderProgramBinaryVariant& variant = *variants.emplaceBack();
			variant = {};

			// Generate the source and the rest for the variant
			ANKI_CHECK(parser.generateVariant(states, parserVariant));

			// Compile
			for(ShaderType shaderType = ShaderType::FIRST; shaderType < ShaderType::COUNT; ++shaderType)
			{
				if(!(shaderTypeToBit(shaderType) & parser.getShaderStages()))
				{
					continue;
				}

				// Compile
				DynamicArrayAuto<U8> spirv(tempAllocator);
				ANKI_CHECK(compilerGlslToSpirv(parserVariant.getSource(shaderType), shaderType, tempAllocator, spirv));
				ANKI_ASSERT(spirv.getSize() > 0);

				// Check if the spirv is already generated
				const U64 hash = computeHash(&spirv[0], spirv.getSize());
				Bool found = false;
				for(PtrSize i = 0; i < codeBlockHashes.getSize(); ++i)
				{
					if(codeBlockHashes[i] == hash)
					{
						// Found it
						variant.m_binaryIndices[shaderType] = U32(i);
						found = true;
						break;
					}
				}

				// Create it if not found
				if(!found)
				{
					U8* code = binaryAllocator.allocate(spirv.getSizeInBytes());
					memcpy(code, &spirv[0], spirv.getSizeInBytes());

					ShaderProgramBinaryCode block;
					block.m_binary = code;
					block.m_binarySize = spirv.getSizeInBytes();
					codeBlocks.emplaceBack(block);

					codeBlockHashes.emplaceBack(hash);

					variant.m_binaryIndices[shaderType] = U32(codeBlocks.getSize() - 1);
				}
			}

			// Active vars
			BitSet<MAX_SHADER_PROGRAM_INPUT_VARIABLES, U64> active(false);
			for(PtrSize i = 0; i < parser.getInputs().getSize(); ++i)
			{
				if(parserVariant.isInputActive(parser.getInputs()[i]))
				{
					active.set(i, true);
				}
			}
			variant.m_activeVariables = active.getData();

			// Mutator values
			variant.m_mutatorValues = binaryAllocator.newArray<I32>(parser.getMutators().getSize());
			for(PtrSize i = 0; i < parser.getMutators().getSize(); ++i)
			{
				variant.m_mutatorValues[i] = states[i].m_value;
			}

			// Input vars
			ShaderVariableBlockInfo defaultInfo;
			defaultInfo.m_arraySize = -1;
			defaultInfo.m_arrayStride = -1;
			defaultInfo.m_matrixStride = -1;
			defaultInfo.m_offset = -1;
			variant.m_blockInfos =
				binaryAllocator.newArray<ShaderVariableBlockInfo>(parser.getInputs().getSize(), defaultInfo);

			variant.m_bindings = binaryAllocator.newArray<I16>(parser.getInputs().getSize(), -1);

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
			}

		} while(spinDials(dials, parser.getMutators()));
	}
	else
	{
		// TODO
	}

	return Error::NONE;
}

} // end namespace anki
