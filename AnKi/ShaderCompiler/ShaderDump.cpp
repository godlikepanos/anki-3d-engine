// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderDump.h>
#include <AnKi/ShaderCompiler/MaliOfflineCompiler.h>
#include <AnKi/ShaderCompiler/Dxc.h>
#include <AnKi/ShaderCompiler/RadeonGpuAnalyzer.h>
#include <AnKi/Util/Serializer.h>
#include <AnKi/Util/StringList.h>
#include <SpirvCross/spirv_glsl.hpp>
#include <ThirdParty/SpirvTools/include/spirv-tools/libspirv.h>

namespace anki {

#define ANKI_TAB "    "

class MaliOfflineCompilerStats
{
public:
	F64 m_fma = 0.0;
	F64 m_cvt = 0.0;
	F64 m_sfu = 0.0;
	F64 m_loadStore = 0.0;
	F64 m_varying = 0.0;
	F64 m_texture = 0.0;

	F64 m_workRegisters = 0.0;
	F64 m_spilling = 0.0;
	F64 m_fp16ArithmeticPercentage = 0.0;

	MaliOfflineCompilerStats() = default;

	MaliOfflineCompilerStats(const MaliOfflineCompilerOut& in)
	{
		*this = in;
	}

	MaliOfflineCompilerStats& operator=(const MaliOfflineCompilerOut& in)
	{
		m_fma = F64(in.m_fma);
		m_cvt = F64(in.m_cvt);
		m_sfu = F64(in.m_sfu);
		m_loadStore = F64(in.m_loadStore);
		m_varying = F64(in.m_varying);
		m_texture = F64(in.m_texture);
		m_workRegisters = F64(in.m_workRegisters);
		m_spilling = F64(in.m_spilling);
		m_fp16ArithmeticPercentage = F64(in.m_fp16ArithmeticPercentage);
		return *this;
	}

	MaliOfflineCompilerStats operator+(const MaliOfflineCompilerStats& b) const
	{
		MaliOfflineCompilerStats out;
		out.m_fma = m_fma + b.m_fma;
		out.m_cvt = m_cvt + b.m_cvt;
		out.m_sfu = m_sfu + b.m_sfu;
		out.m_loadStore = m_loadStore + b.m_loadStore;
		out.m_varying = m_varying + b.m_varying;
		out.m_texture = m_texture + b.m_texture;
		out.m_workRegisters = m_workRegisters + b.m_workRegisters;
		out.m_spilling = m_spilling + b.m_spilling;
		out.m_fp16ArithmeticPercentage = m_fp16ArithmeticPercentage + b.m_fp16ArithmeticPercentage;
		return out;
	}

	MaliOfflineCompilerStats operator*(F64 val) const
	{
		MaliOfflineCompilerStats out;
		out.m_fma = m_fma * val;
		out.m_cvt = m_cvt * val;
		out.m_sfu = m_sfu * val;
		out.m_loadStore = m_loadStore * val;
		out.m_varying = m_varying * val;
		out.m_texture = m_texture * val;
		out.m_workRegisters = m_workRegisters * val;
		out.m_spilling = m_spilling * val;
		out.m_fp16ArithmeticPercentage = m_fp16ArithmeticPercentage * val;
		return out;
	}

	MaliOfflineCompilerStats max(const MaliOfflineCompilerStats& b) const
	{
		MaliOfflineCompilerStats out;
		out.m_fma = anki::max(m_fma, b.m_fma);
		out.m_cvt = anki::max(m_cvt, b.m_cvt);
		out.m_sfu = anki::max(m_sfu, b.m_sfu);
		out.m_loadStore = anki::max(m_loadStore, b.m_loadStore);
		out.m_varying = anki::max(m_varying, b.m_varying);
		out.m_texture = anki::max(m_texture, b.m_texture);
		out.m_workRegisters = anki::max(m_workRegisters, b.m_workRegisters);
		out.m_spilling = anki::max(m_spilling, b.m_spilling);
		out.m_fp16ArithmeticPercentage = anki::max(m_fp16ArithmeticPercentage, b.m_fp16ArithmeticPercentage);
		return out;
	}

	String toString() const
	{
		String str;
		str.sprintf("Regs %f Spilling %f FMA %f CVT %f SFU %f LS %f VAR %f TEX %f FP16 %f%%", m_workRegisters, m_spilling, m_fma, m_cvt, m_sfu,
					m_loadStore, m_varying, m_texture, m_fp16ArithmeticPercentage);
		return str;
	}
};

class RgaStats
{
public:
	F64 m_vgprCount = 0.0;
	F64 m_sgprCount = 0.0;
	F64 m_isaSize = 0.0;

	RgaStats() = default;

	RgaStats(const RgaOutput& b)
	{
		*this = b;
	}

	RgaStats& operator=(const RgaOutput& b)
	{
		m_vgprCount = F64(b.m_vgprCount);
		m_sgprCount = F64(b.m_sgprCount);
		m_isaSize = F64(b.m_isaSize);
		return *this;
	}

	RgaStats operator+(const RgaStats& b) const
	{
		RgaStats out;
		out.m_vgprCount = m_vgprCount + b.m_vgprCount;
		out.m_sgprCount = m_sgprCount + b.m_sgprCount;
		out.m_isaSize = m_isaSize + b.m_isaSize;
		return out;
	}

	RgaStats operator*(F64 val) const
	{
		RgaStats out;
		out.m_vgprCount = m_vgprCount * val;
		out.m_sgprCount = m_sgprCount * val;
		out.m_isaSize = m_isaSize * val;
		return out;
	}

	RgaStats max(const RgaStats& b) const
	{
		RgaStats out;
		out.m_vgprCount = anki::max(m_vgprCount, b.m_vgprCount);
		out.m_sgprCount = anki::max(m_sgprCount, b.m_sgprCount);
		out.m_isaSize = anki::max(m_isaSize, b.m_isaSize);
		return out;
	}

	String toString() const
	{
		return String().sprintf("VGPRs %f SGPRs %f ISA size %f", m_vgprCount, m_sgprCount, m_isaSize);
	}
};

class PerStageDumpStats
{
public:
	class
	{
	public:
		MaliOfflineCompilerStats m_mali;
		RgaStats m_amd;
	} m_average;

	class
	{
	public:
		MaliOfflineCompilerStats m_mali;
		RgaStats m_amd;
	} m_max;
};

class DumpStats
{
public:
	Array<PerStageDumpStats, U32(ShaderType::kCount)> m_stages;
};

void dumpShaderBinary(const ShaderDumpOptions& options, const ShaderBinary& binary, ShaderCompilerString& humanReadable)
{
	ShaderCompilerStringList lines;

	lines.pushBackSprintf("# BINARIES (%u)\n\n", binary.m_codeBlocks.getSize());
	Array<MaliOfflineCompilerStats, U32(ShaderType::kCount)> maliAverages;
	Array<MaliOfflineCompilerStats, U32(ShaderType::kCount)> maliMaxes;
	Array<RgaStats, U32(ShaderType::kCount)> rgaAverages;
	Array<RgaStats, U32(ShaderType::kCount)> rgaMaxes;
	Array<U32, U32(ShaderType::kCount)> shadersPerStage = {};
	U32 count = 0;
	ShaderTypeBit stagesInUse = ShaderTypeBit::kNone;
	for(const ShaderBinaryCodeBlock& code : binary.m_codeBlocks)
	{
		// Rewrite spir-v because of the decorations we ask DXC to put
		Bool bRequiresMeshShaders = false;
		DynamicArray<U8> newSpirv;
		newSpirv.resize(code.m_binary.getSize());
		memcpy(newSpirv.getBegin(), code.m_binary.getBegin(), code.m_binary.getSizeInBytes());
		ShaderType shaderType = ShaderType::kCount;
		Error visitErr = Error::kNone;
		visitSpirv(WeakArray<U32>(reinterpret_cast<U32*>(newSpirv.getBegin()), U32(newSpirv.getSizeInBytes() / sizeof(U32))),
				   [&](U32 cmd, WeakArray<U32> instructions) {
					   if(cmd == spv::OpDecorate && instructions[1] == spv::DecorationDescriptorSet && instructions[2] == kDxcVkBindlessRegisterSpace)
					   {
						   // Bindless set, rewrite its set
						   instructions[2] = kMaxRegisterSpaces;
					   }
					   else if(cmd == spv::OpCapability && instructions[0] == spv::CapabilityMeshShadingEXT)
					   {
						   bRequiresMeshShaders = true;
					   }
					   else if(cmd == spv::OpEntryPoint)
					   {
						   switch(instructions[0])
						   {
						   case spv::ExecutionModelVertex:
							   shaderType = ShaderType::kVertex;
							   break;
						   case spv::ExecutionModelTessellationControl:
							   shaderType = ShaderType::kHull;
							   break;
						   case spv::ExecutionModelTessellationEvaluation:
							   shaderType = ShaderType::kDomain;
							   break;
						   case spv::ExecutionModelGeometry:
							   shaderType = ShaderType::kGeometry;
							   break;
						   case spv::ExecutionModelTaskEXT:
						   case spv::ExecutionModelTaskNV:
							   shaderType = ShaderType::kAmplification;
							   break;
						   case spv::ExecutionModelMeshEXT:
						   case spv::ExecutionModelMeshNV:
							   shaderType = ShaderType::kMesh;
							   break;
						   case spv::ExecutionModelFragment:
							   shaderType = ShaderType::kPixel;
							   break;
						   case spv::ExecutionModelGLCompute:
							   shaderType = ShaderType::kCompute;
							   break;
						   case spv::ExecutionModelRayGenerationKHR:
							   shaderType = ShaderType::kRayGen;
							   break;
						   case spv::ExecutionModelAnyHitKHR:
							   shaderType = ShaderType::kAnyHit;
							   break;
						   case spv::ExecutionModelClosestHitKHR:
							   shaderType = ShaderType::kClosestHit;
							   break;
						   case spv::ExecutionModelMissKHR:
							   shaderType = ShaderType::kMiss;
							   break;
						   case spv::ExecutionModelIntersectionKHR:
							   shaderType = ShaderType::kIntersection;
							   break;
						   case spv::ExecutionModelCallableKHR:
							   shaderType = ShaderType::kCallable;
							   break;
						   default:
							   ANKI_SHADER_COMPILER_LOGE("Unrecognized SPIRV execution model: %u", instructions[0]);
							   visitErr = Error::kFunctionFailed;
						   }
					   }
				   });

		stagesInUse |= ShaderTypeBit(1 << shaderType);
		++shadersPerStage[shaderType];

		lines.pushBackSprintf("## bin%05u (%s)\n", count, g_shaderTypeNames[shaderType].cstr());

		if(options.m_maliStats || options.m_amdStats)
		{
			lines.pushBack("### Stats\n");
		}

		if(options.m_maliStats && !visitErr)
		{
			lines.pushBack("```\n");
		}

		if(options.m_maliStats && !visitErr)
		{
			if((shaderType == ShaderType::kVertex || shaderType == ShaderType::kPixel || shaderType == ShaderType::kCompute) && !bRequiresMeshShaders)
			{
				MaliOfflineCompilerOut maliocOut;
				const Error err = runMaliOfflineCompiler(newSpirv, shaderType, maliocOut);
				if(err)
				{
					ANKI_LOGE("Mali offline compiler failed");
					lines.pushBackSprintf("Mali: *malioc failed*  \n");
				}
				else
				{
					lines.pushBackSprintf("Mali: %s  \n", maliocOut.toString().cstr());
					maliAverages[shaderType] = maliAverages[shaderType] + maliocOut;
					maliMaxes[shaderType] = maliMaxes[shaderType].max(maliocOut);
				}
			}
		}

		if(options.m_amdStats && !visitErr)
		{
			if((shaderType == ShaderType::kVertex || shaderType == ShaderType::kPixel || shaderType == ShaderType::kCompute) && !bRequiresMeshShaders)
			{
				RgaOutput rgaOut = {};
				const Error err = runRadeonGpuAnalyzer(newSpirv, shaderType, rgaOut);
				if(err)
				{
					ANKI_LOGE("RGA failed");
					lines.pushBackSprintf("AMD: *RGA failed*  \n");
				}
				else
				{
					lines.pushBackSprintf("AMD: %s  \n", rgaOut.toString().cstr());
					rgaAverages[shaderType] = rgaAverages[shaderType] + rgaOut;
					rgaMaxes[shaderType] = rgaMaxes[shaderType].max(rgaOut);
				}
			}
		}

		if(options.m_maliStats && !visitErr)
		{
			lines.pushBack("```\n");
		}

		String reflectionStr;
		code.m_reflection.toString().join("\n", reflectionStr);
		lines.pushBack("### Reflection\n");
		lines.pushBackSprintf("```\n%s\n```\n", reflectionStr.cstr());

		if(options.m_writeGlsl)
		{
			lines.pushBack("### GLSL\n");

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
			sourceLines.join("\n", newGlsl);

			lines.pushBackSprintf("```GLSL\n%s\n```\n", newGlsl.cstr());
		}

		if(options.m_writeSpirv)
		{
			lines.pushBack("### SPIR-V\n");

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
				spvlines.join("\n", final);

				lines.pushBackSprintf("```\n%s\n```\n", final.cstr());
			}
			else
			{
				lines.pushBackSprintf("*error in spiv-dis*\n");
			}

			spvTextDestroy(text);
		}

		++count;
	}

	// Mutators
	lines.pushBackSprintf("\n# MUTATORS (%u)\n\n", binary.m_mutators.getSize());
	if(binary.m_mutators.getSize() > 0)
	{
		lines.pushBackSprintf("| %-32s | %-18s |\n", "Name", "Values");
		lines.pushBackSprintf("| -------------------------------- | ------------------ |\n");

		for(const ShaderBinaryMutator& mutator : binary.m_mutators)
		{
			ShaderCompilerStringList valuesStrl;
			for(U32 i = 0; i < mutator.m_values.getSize(); ++i)
			{
				valuesStrl.pushBackSprintf("%d", mutator.m_values[i]);
			}
			ShaderCompilerString valuesStr;
			valuesStrl.join(", ", valuesStr);

			lines.pushBackSprintf("| %-32s | %-18s |\n", &mutator.m_name[0], valuesStr.cstr());
		}
	}

	// Techniques
	lines.pushBackSprintf("\n# TECHNIQUES (%u)\n\n", binary.m_techniques.getSize());
	lines.pushBackSprintf("| %-32s | %-12s |\n", "Name", "Shader Types");
	lines.pushBackSprintf("| -------------------------------- | ------------ |\n");
	count = 0;
	for(const ShaderBinaryTechnique& t : binary.m_techniques)
	{
		lines.pushBackSprintf("| %-32s | 0x%010x |\n", t.m_name.getBegin(), U32(t.m_shaderTypes));
	}

	// Mutations
	U32 skippedMutations = 0;
	for(const ShaderBinaryMutation& mutation : binary.m_mutations)
	{
		skippedMutations += mutation.m_variantIndex == kMaxU32;
	}

	lines.pushBackSprintf("\n# MUTATIONS (total %u, skipped %u)\n\n", binary.m_mutations.getSize(), skippedMutations);
	lines.pushBackSprintf("| %-8s | %-8s | %-18s |", "Mutation", "Variant", "Hash");

	if(binary.m_mutators.getSize() > 0)
	{
		for(U32 i = 0; i < binary.m_mutators.getSize(); ++i)
		{
			lines.pushBackSprintf(" %-32s |", binary.m_mutators[i].m_name.getBegin());
		}
	}
	lines.pushBack("\n");

	lines.pushBackSprintf("| -------- | -------- | ------------------ |");
	if(binary.m_mutators.getSize() > 0)
	{
		for(U32 i = 0; i < binary.m_mutators.getSize(); ++i)
		{
			lines.pushBackSprintf(" -------------------------------- |");
		}
	}
	lines.pushBack("\n");

	count = 0;
	for(const ShaderBinaryMutation& mutation : binary.m_mutations)
	{
		if(mutation.m_variantIndex != kMaxU32)
		{
			lines.pushBackSprintf("| mut%05u | var%05u | 0x%016" PRIX64 " | ", count++, mutation.m_variantIndex, mutation.m_hash);
		}
		else
		{
			lines.pushBackSprintf("| mut%05u | -        | 0x%016" PRIX64 " | ", count++, mutation.m_hash);
		}

		if(mutation.m_values.getSize() > 0)
		{
			for(U32 i = 0; i < mutation.m_values.getSize(); ++i)
			{
				lines.pushBackSprintf("%-32d | ", I32(mutation.m_values[i]));
			}
		}

		lines.pushBack("\n");
	}

	// Variants
	lines.pushBackSprintf("\n# SHADER VARIANTS (%u)\n\n", binary.m_variants.getSize());
	lines.pushBackSprintf("| Variant  | %-32s | ", "Technique");
	for(ShaderType s : EnumBitsIterable<ShaderType, ShaderTypeBit>(stagesInUse))
	{
		lines.pushBackSprintf("%-13s | ", g_shaderTypeNames[s].cstr());
	}
	lines.pushBack("\n");
	lines.pushBackSprintf("| -------- | %s |", ShaderCompilerString('-', 32).cstr());
	for([[maybe_unused]] ShaderType s : EnumBitsIterable<ShaderType, ShaderTypeBit>(stagesInUse))
	{
		lines.pushBackSprintf(" %s |", ShaderCompilerString('-', 13).cstr());
	}
	lines.pushBack("\n");

	count = 0;
	for(const ShaderBinaryVariant& variant : binary.m_variants)
	{
		// Binary indices
		for(U32 t = 0; t < binary.m_techniques.getSize(); ++t)
		{
			if(t == 0)
			{
				lines.pushBackSprintf("| var%05u | ", count);
			}
			else
			{
				lines.pushBack("|          | ");
			}
			lines.pushBackSprintf("%-32s | ", binary.m_techniques[t].m_name.getBegin());

			for(ShaderType s : EnumBitsIterable<ShaderType, ShaderTypeBit>(stagesInUse))
			{
				if(variant.m_techniqueCodeBlocks[t].m_codeBlockIndices[s] < kMaxU32)
				{
					lines.pushBackSprintf("bin%05u      | ", variant.m_techniqueCodeBlocks[t].m_codeBlockIndices[s]);
				}
				else
				{
					lines.pushBackSprintf("-%s | ", ShaderCompilerString(' ', 12).cstr());
				}
			}

			lines.pushBack("\n");
		}

		++count;
	}

	// Structs
	lines.pushBackSprintf("\n# STRUCTS (%u)\n\n", binary.m_structs.getSize());
	if(binary.m_structs.getSize() > 0)
	{
		for(const ShaderBinaryStruct& s : binary.m_structs)
		{
			lines.pushBack("```C++\n");
			lines.pushBackSprintf("struct %s // size: %u\n", s.m_name.getBegin(), s.m_size);
			lines.pushBack("{\n");

			for(const ShaderBinaryStructMember& member : s.m_members)
			{
				const CString typeStr = getShaderVariableDataTypeInfo(member.m_type).m_name;
				lines.pushBackSprintf(ANKI_TAB "%5s %s; // offset: %u\n", typeStr.cstr(), member.m_name.getBegin(), member.m_offset);
			}
			lines.pushBack("};\n");
			lines.pushBack("```\n");
		}
	}

	// Stats
	if(options.m_maliStats || options.m_amdStats)
	{
		lines.pushBackSprintf("\n# COMPILER STATS\n\n");

		for(ShaderType s : EnumBitsIterable<ShaderType, ShaderTypeBit>(stagesInUse))
		{
			lines.pushBackSprintf("%s\n```\n", g_shaderTypeNames[s].cstr());

			if(options.m_maliStats)
			{
				maliAverages[s] = maliAverages[s] * (1.0 / F64(shadersPerStage[s]));
				lines.pushBackSprintf("Mali avg: %s  \n", maliAverages[s].toString().cstr());
				lines.pushBackSprintf("Mali max: %s  \n", maliMaxes[s].toString().cstr());
			}

			if(options.m_amdStats)
			{
				rgaAverages[s] = rgaAverages[s] * (1.0 / F64(shadersPerStage[s]));
				lines.pushBackSprintf("AMD avg: %s  \n", rgaAverages[s].toString().cstr());
				lines.pushBackSprintf("AMD max: %s  \n", rgaMaxes[s].toString().cstr());
			}

			lines.pushBack("```\n");
		}
	}

	lines.join("", humanReadable);
}

#undef ANKI_TAB

} // end namespace anki
