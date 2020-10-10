// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramDump.h>
#include <anki/util/Serializer.h>
#include <anki/util/StringList.h>
#include <SPIRV-Cross/spirv_glsl.hpp>

namespace anki
{

#define ANKI_TAB "    "

static void disassembleBlock(const ShaderProgramBinaryBlockInstance& instance, const ShaderProgramBinaryBlock& block,
							 StringListAuto& lines)
{
	lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s set %4u binding %4u size %4u\n", block.m_name.getBegin(),
						  block.m_set, block.m_binding, instance.m_size);

	for(U32 i = 0; i < instance.m_variables.getSize(); ++i)
	{
		const ShaderProgramBinaryVariableInstance& varInstance = instance.m_variables[i];
		const ShaderProgramBinaryVariable& var = block.m_variables[varInstance.m_index];

		lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB ANKI_TAB "%-48s type %8s blockInfo %d,%d,%d,%d\n",
							  var.m_name.getBegin(), shaderVariableDataTypeToString(var.m_type).cstr(),
							  varInstance.m_blockInfo.m_offset, varInstance.m_blockInfo.m_arraySize,
							  varInstance.m_blockInfo.m_arrayStride, varInstance.m_blockInfo.m_matrixStride);
	}
}

void dumpShaderProgramBinary(const ShaderProgramBinary& binary, StringAuto& humanReadable)
{
	GenericMemoryPoolAllocator<U8> alloc = humanReadable.getAllocator();
	StringListAuto lines(alloc);

	if(binary.m_libraryName[0])
	{
		lines.pushBack("**LIBRARY**\n");
		lines.pushBackSprintf(ANKI_TAB "%s\n\n", &binary.m_libraryName[0]);
	}

	if(binary.m_rayType != MAX_U32)
	{
		lines.pushBack("**RAY TYPE**\n");
		lines.pushBackSprintf(ANKI_TAB "%u\n\n", binary.m_rayType);
	}

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
		options.version = 460;

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
									  o.m_name.getBegin(), o.m_set, o.m_binding,
									  shaderVariableDataTypeToString(o.m_type).cstr(), instance.m_arraySize);
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
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s type %8s id %4u\n", c.m_name.getBegin(),
									  shaderVariableDataTypeToString(c.m_type).cstr(), c.m_constantId);
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
		lines.pushBackSprintf(ANKI_TAB "#%-4u variantIndex %5u values (", count++, mutation.m_variantIndex);
		if(mutation.m_values.getSize() > 0)
		{
			for(U32 i = 0; i < mutation.m_values.getSize(); ++i)
			{
				lines.pushBackSprintf((i < mutation.m_values.getSize() - 1) ? "%s %4d, " : "%s %4d",
									  binary.m_mutators[i].m_name.getBegin(), I32(mutation.m_values[i]));
			}

			lines.pushBack(")");
		}
		else
		{
			lines.pushBack("N/A)");
		}

		lines.pushBack("\n");
	}

	lines.join("", humanReadable);
}

#undef ANKI_TAB

} // end namespace anki
