// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderDump.h>
#include <AnKi/Util/Serializer.h>
#include <AnKi/Util/StringList.h>
#include <SpirvCross/spirv_glsl.hpp>
#include <ThirdParty/SpirvTools/include/spirv-tools/libspirv.h>

namespace anki {

#define ANKI_TAB "    "

void dumpShaderBinary(const ShaderDumpOptions& options, const ShaderBinary& binary, ShaderCompilerString& humanReadable)
{
	ShaderCompilerStringList lines;

	lines.pushBackSprintf("\n**BINARIES (%u)**\n", binary.m_codeBlocks.getSize());
	U32 count = 0;
	for(const ShaderBinaryCodeBlock& code : binary.m_codeBlocks)
	{
		lines.pushBackSprintf(ANKI_TAB "bin%05u \n", count++);

		if(options.m_writeGlsl)
		{
			spirv_cross::CompilerGLSL::Options options;
			options.vulkan_semantics = true;
			options.version = 460;
			options.force_temporary = true;

			const unsigned int* spvb = reinterpret_cast<const unsigned int*>(code.m_binary.getBegin());
			ANKI_ASSERT((code.m_binary.getSize() % (sizeof(unsigned int))) == 0);
			std::vector<unsigned int> spv(spvb, spvb + code.m_binary.getSize() / sizeof(unsigned int));
			spirv_cross::CompilerGLSL compiler(spv);
			compiler.set_common_options(options);

			std::string glsl = compiler.compile();
			StringList sourceLines;
			sourceLines.splitString(glsl.c_str(), '\n');
			String newGlsl;
			sourceLines.join("\n" ANKI_TAB ANKI_TAB, newGlsl);

			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "%s\n", newGlsl.cstr());
		}

		if(options.m_writeSpirv)
		{
			spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_5);

			const U32 disOptions = SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_NO_HEADER;
			spv_text text = nullptr;

			const spv_result_t error = spvBinaryToText(context, reinterpret_cast<const U32*>(code.m_binary.getBegin()),
													   code.m_binary.getSizeInBytes() / 4, disOptions, &text, nullptr);

			spvContextDestroy(context);

			if(!error)
			{
				StringList spvlines;
				spvlines.splitString(text->str, '\n');

				String final;
				spvlines.join("\n" ANKI_TAB ANKI_TAB, final);

				lines.pushBackSprintf(ANKI_TAB ANKI_TAB "%s\n", final.cstr());
			}
			else
			{
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB "*error in spiv-dis*\n");
			}

			spvTextDestroy(text);
		}
	}

	// Mutators
	lines.pushBackSprintf("\n**MUTATORS (%u)**\n", binary.m_mutators.getSize());
	if(binary.m_mutators.getSize() > 0)
	{
		for(const ShaderBinaryMutator& mutator : binary.m_mutators)
		{
			lines.pushBackSprintf(ANKI_TAB "%-32s values (", &mutator.m_name[0]);
			for(U32 i = 0; i < mutator.m_values.getSize(); ++i)
			{
				lines.pushBackSprintf((i < mutator.m_values.getSize() - 1) ? "%d," : "%d)", mutator.m_values[i]);
			}
			lines.pushBack("\n");
		}
	}

	// Techniques
	lines.pushBackSprintf("\n**TECHNIQUES (%u)**\n", binary.m_techniques.getSize());
	count = 0;
	for(const ShaderBinaryTechnique& t : binary.m_techniques)
	{
		lines.pushBackSprintf(ANKI_TAB "%-32s shaderTypes 0x%02x\n", t.m_name.getBegin(), U32(t.m_shaderTypes));
	}

	// Mutations
	U32 skippedMutations = 0;
	for(const ShaderBinaryMutation& mutation : binary.m_mutations)
	{
		skippedMutations += mutation.m_variantIndex == kMaxU32;
	}

	lines.pushBackSprintf("\n**MUTATIONS (%u skipped %u)**\n", binary.m_mutations.getSize(), skippedMutations);
	count = 0;
	for(const ShaderBinaryMutation& mutation : binary.m_mutations)
	{
		if(mutation.m_variantIndex != kMaxU32)
		{
			lines.pushBackSprintf(ANKI_TAB "mut%05u variantIndex var%05u hash 0x%016" PRIX64 " values (", count++, mutation.m_variantIndex,
								  mutation.m_hash);
		}
		else
		{
			lines.pushBackSprintf(ANKI_TAB "mut%05u variantIndex N/A      hash 0x%016" PRIX64 " values (", count++, mutation.m_hash);
		}

		if(mutation.m_values.getSize() > 0)
		{
			for(U32 i = 0; i < mutation.m_values.getSize(); ++i)
			{
				lines.pushBackSprintf((i < mutation.m_values.getSize() - 1) ? "%s %4d, " : "%s %4d", binary.m_mutators[i].m_name.getBegin(),
									  I32(mutation.m_values[i]));
			}

			lines.pushBack(")");
		}
		else
		{
			lines.pushBack("N/A)");
		}

		lines.pushBack("\n");
	}

	// Variants
	lines.pushBackSprintf("\n**SHADER VARIANTS (%u)**\n", binary.m_variants.getSize());
	count = 0;
	for(const ShaderBinaryVariant& variant : binary.m_variants)
	{
		lines.pushBackSprintf(ANKI_TAB "var%05u\n", count++);

		// Binary indices
		for(U32 t = 0; t < binary.m_techniques.getSize(); ++t)
		{
			lines.pushBackSprintf(ANKI_TAB ANKI_TAB "%-32s binaries (", binary.m_techniques[t].m_name.getBegin());

			for(ShaderType s : EnumIterable<ShaderType>())
			{
				if(variant.m_techniqueCodeBlocks[t].m_codeBlockIndices[s] < kMaxU32)
				{
					lines.pushBackSprintf("bin%05u", variant.m_techniqueCodeBlocks[t].m_codeBlockIndices[s]);
				}
				else
				{
					lines.pushBack("--------");
				}

				lines.pushBack((s == ShaderType::kCount - 1) ? ")\n" : ", ");
			}
		}
	}

	// Structs
	lines.pushBackSprintf("\n**STRUCTS (%u)**\n", binary.m_structs.getSize());
	if(binary.m_structs.getSize() > 0)
	{
		for(const ShaderBinaryStruct& s : binary.m_structs)
		{
			lines.pushBackSprintf(ANKI_TAB "%-32s size %4u\n", s.m_name.getBegin(), s.m_size);

			for(const ShaderBinaryStructMember& member : s.m_members)
			{
				const CString typeStr = getShaderVariableDataTypeInfo(member.m_type).m_name;
				lines.pushBackSprintf(ANKI_TAB ANKI_TAB "%-32s type %5s offset %4d\n", member.m_name.getBegin(), typeStr.cstr(), member.m_offset);
			}
		}
	}

	lines.join("", humanReadable);
}

#undef ANKI_TAB

} // end namespace anki
