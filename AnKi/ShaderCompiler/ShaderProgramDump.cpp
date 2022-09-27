// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderProgramDump.h>
#include <AnKi/Util/Serializer.h>
#include <AnKi/Util/StringList.h>
#include <SpirvCross/spirv_glsl.hpp>

namespace anki {

#define ANKI_TAB "    "

static void disassembleBlockInstance(const ShaderProgramBinaryBlockInstance& instance,
									 const ShaderProgramBinaryBlock& block, StringListRaii& lines)
{
	lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s set %4u binding %4u size %4u\n", block.m_name.getBegin(),
						  block.m_set, block.m_binding, instance.m_size);

	for(U32 i = 0; i < instance.m_variableInstances.getSize(); ++i)
	{
		const ShaderProgramBinaryVariableInstance& varInstance = instance.m_variableInstances[i];
		const ShaderProgramBinaryVariable& var = block.m_variables[varInstance.m_index];

		lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB ANKI_TAB "%-48s type %8s blockInfo %d,%d,%d,%d\n",
							  var.m_name.getBegin(), getShaderVariableDataTypeInfo(var.m_type).m_name,
							  varInstance.m_blockInfo.m_offset, varInstance.m_blockInfo.m_arraySize,
							  varInstance.m_blockInfo.m_arrayStride, varInstance.m_blockInfo.m_matrixStride);
	}
}

static void disassembleBlock(const ShaderProgramBinaryBlock& block, StringListRaii& lines)
{
	lines.pushBackSprintf(ANKI_TAB "%-32s set %4u binding %4u\n", block.m_name.getBegin(), block.m_set,
						  block.m_binding);

	for(const ShaderProgramBinaryVariable& var : block.m_variables)
	{
		lines.pushBackSprintf(ANKI_TAB ANKI_TAB "%-48s type %8s\n", var.m_name.getBegin(),
							  getShaderVariableDataTypeInfo(var.m_type).m_name);
	}
}

void dumpShaderProgramBinary(const ShaderProgramBinary& binary, StringRaii& humanReadable)
{
	BaseMemoryPool& pool = humanReadable.getMemoryPool();
	StringListRaii lines(&pool);

	if(binary.m_libraryName[0])
	{
		lines.pushBack("**LIBRARY**\n");
		lines.pushBackSprintf(ANKI_TAB "%s\n", &binary.m_libraryName[0]);
	}

	if(binary.m_rayType != kMaxU32)
	{
		lines.pushBack("\n**RAY TYPE**\n");
		lines.pushBackSprintf(ANKI_TAB "%u\n", binary.m_rayType);
	}

	lines.pushBack("\n**MUTATORS**\n");
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

	lines.pushBack("\n**UNIFORM BLOCKS**\n");
	if(binary.m_uniformBlocks.getSize() > 0)
	{
		for(const ShaderProgramBinaryBlock& block : binary.m_uniformBlocks)
		{
			disassembleBlock(block, lines);
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**STORAGE BLOCKS**\n");
	if(binary.m_storageBlocks.getSize() > 0)
	{
		for(const ShaderProgramBinaryBlock& block : binary.m_storageBlocks)
		{
			disassembleBlock(block, lines);
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**PUSH CONSTANTS**\n");
	if(binary.m_pushConstantBlock)
	{
		disassembleBlock(*binary.m_pushConstantBlock, lines);
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**OPAQUE**\n");
	if(binary.m_opaques.getSize() > 0)
	{
		for(const ShaderProgramBinaryOpaque& o : binary.m_opaques)
		{
			lines.pushBackSprintf(ANKI_TAB "%-32s set %4u binding %4u type %12s\n", o.m_name.getBegin(), o.m_set,
								  o.m_binding, getShaderVariableDataTypeInfo(o.m_type).m_name);
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**CONSTANTS**\n");
	if(binary.m_constants.getSize() > 0)
	{
		for(const ShaderProgramBinaryConstant& c : binary.m_constants)
		{
			lines.pushBackSprintf(ANKI_TAB "%-32s type %8s id %4u\n", c.m_name.getBegin(),
								  getShaderVariableDataTypeInfo(c.m_type).m_name, c.m_constantId);
		}
	}
	else
	{
		lines.pushBack(ANKI_TAB "N/A\n");
	}

	lines.pushBack("\n**STRUCTS**\n");
	if(binary.m_structs.getSize() > 0)
	{
		for(const ShaderProgramBinaryStruct& s : binary.m_structs)
		{
			lines.pushBackSprintf(ANKI_TAB "%-32s\n", s.m_name.getBegin());

			for(const ShaderProgramBinaryStructMember& member : s.m_members)
			{
				const CString typeStr = (member.m_type == ShaderVariableDataType::kNone)
											? &binary.m_structs[member.m_structIndex].m_name[0]
											: getShaderVariableDataTypeInfo(member.m_type).m_name;
				const CString dependentMutator = (member.m_dependentMutator != kMaxU32)
													 ? binary.m_mutators[member.m_dependentMutator].m_name.getBegin()
													 : "None";
				lines.pushBackSprintf(
					ANKI_TAB ANKI_TAB "%-32s type %24s dependentMutator %-32s dependentMutatorValue %4d\n",
					member.m_name.getBegin(), typeStr.cstr(), dependentMutator.cstr(), member.m_dependentMutatorValue);
			}
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
		StringListRaii sourceLines(&pool);
		sourceLines.splitString(glsl.c_str(), '\n');
		StringRaii newGlsl(&pool);
		sourceLines.join("\n" ANKI_TAB ANKI_TAB, newGlsl);

		lines.pushBackSprintf(ANKI_TAB "#bin%05u \n" ANKI_TAB ANKI_TAB "%s\n", count++, newGlsl.cstr());
	}

	lines.pushBack("\n**SHADER VARIANTS**\n");
	count = 0;
	for(const ShaderProgramBinaryVariant& variant : binary.m_variants)
	{
		lines.pushBackSprintf(ANKI_TAB "#var%05u\n", count++);

		// Uniform blocks
		if(variant.m_uniformBlocks.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Uniform blocks\n");
			for(const ShaderProgramBinaryBlockInstance& instance : variant.m_uniformBlocks)
			{
				disassembleBlockInstance(instance, binary.m_uniformBlocks[instance.m_index], lines);
			}
		}

		// Storage blocks
		if(variant.m_storageBlocks.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Storage blocks\n");
			for(const ShaderProgramBinaryBlockInstance& instance : variant.m_storageBlocks)
			{
				disassembleBlockInstance(instance, binary.m_storageBlocks[instance.m_index], lines);
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
									  getShaderVariableDataTypeInfo(o.m_type).m_name, instance.m_arraySize);
			}
		}

		// Push constants
		if(variant.m_pushConstantBlock)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Push constants\n");
			disassembleBlockInstance(*variant.m_pushConstantBlock, *binary.m_pushConstantBlock, lines);
		}

		// Constants
		if(variant.m_constants.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Specialization constants\n");
			for(const ShaderProgramBinaryConstantInstance& instance : variant.m_constants)
			{
				const ShaderProgramBinaryConstant& c = binary.m_constants[instance.m_index];
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s type %8s id %4u\n", c.m_name.getBegin(),
									  getShaderVariableDataTypeInfo(c.m_type).m_name, c.m_constantId);
			}
		}

		// Structs
		if(variant.m_structs.getSize() > 0)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "Structs\n");
			for(const ShaderProgramBinaryStructInstance& instance : variant.m_structs)
			{
				const ShaderProgramBinaryStruct& s = binary.m_structs[instance.m_index];
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB "%-32s size %4u\n", s.m_name.getBegin(),
									  instance.m_size);

				for(const ShaderProgramBinaryStructMemberInstance& memberInstance : instance.m_memberInstances)
				{
					const ShaderProgramBinaryStructMember& member = s.m_members[memberInstance.m_index];
					lines.pushBackSprintf(ANKI_TAB ANKI_TAB ANKI_TAB ANKI_TAB "%-32s offset %4u arraySize %4u\n",
										  member.m_name.getBegin(), memberInstance.m_offset,
										  memberInstance.m_arraySize);
				}
			}
		}

		// Binary indices
		lines.pushBack(ANKI_TAB ANKI_TAB "Binaries ");
		for(ShaderType shaderType : EnumIterable<ShaderType>())
		{
			if(variant.m_codeBlockIndices[shaderType] < kMaxU32)
			{
				lines.pushBackSprintf("#bin%05u", variant.m_codeBlockIndices[shaderType]);
			}
			else
			{
				lines.pushBack("-");
			}

			if(shaderType != ShaderType::kLast)
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
		lines.pushBackSprintf(ANKI_TAB "#mut%-4u variantIndex #var%05u hash 0x%016" PRIX64 " values (", count++,
							  mutation.m_variantIndex, mutation.m_hash);
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
