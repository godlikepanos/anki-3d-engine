// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/Glslang.h>
#include <anki/util/StringList.h>
#include <anki/util/File.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wundef"
#	pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/StandAlone/DirStackFileIncluder.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki
{

class GlslangCtx
{
public:
	GlslangCtx()
	{
		glslang::InitializeProcess();
	}

	~GlslangCtx()
	{
		glslang::FinalizeProcess();
	}
};

GlslangCtx g_glslangCtx;

static TBuiltInResource setGlslangLimits()
{
	TBuiltInResource c = {};

	c.maxLights = 32;
	c.maxClipPlanes = 6;
	c.maxTextureUnits = 32;
	c.maxTextureCoords = 32;
	c.maxVertexAttribs = 64;
	c.maxVertexUniformComponents = 4096;
	c.maxVaryingFloats = 64;
	c.maxVertexTextureImageUnits = 32;
	c.maxCombinedTextureImageUnits = 80;
	c.maxTextureImageUnits = 32;
	c.maxFragmentUniformComponents = 4096;
	c.maxDrawBuffers = 32;
	c.maxVertexUniformVectors = 128;
	c.maxVaryingVectors = 8;
	c.maxFragmentUniformVectors = 16;
	c.maxVertexOutputVectors = 16;
	c.maxFragmentInputVectors = 15;
	c.minProgramTexelOffset = -8;
	c.maxProgramTexelOffset = 7;
	c.maxClipDistances = 8;
	c.maxComputeWorkGroupCountX = 65535;
	c.maxComputeWorkGroupCountY = 65535;
	c.maxComputeWorkGroupCountZ = 65535;
	c.maxComputeWorkGroupSizeX = 1024;
	c.maxComputeWorkGroupSizeY = 1024;
	c.maxComputeWorkGroupSizeZ = 64;
	c.maxComputeUniformComponents = 1024;
	c.maxComputeTextureImageUnits = 16;
	c.maxComputeImageUniforms = 8;
	c.maxComputeAtomicCounters = 8;
	c.maxComputeAtomicCounterBuffers = 1;
	c.maxVaryingComponents = 60;
	c.maxVertexOutputComponents = 64;
	c.maxGeometryInputComponents = 64;
	c.maxGeometryOutputComponents = 128;
	c.maxFragmentInputComponents = 128;
	c.maxImageUnits = 8;
	c.maxCombinedImageUnitsAndFragmentOutputs = 8;
	c.maxCombinedShaderOutputResources = 8;
	c.maxImageSamples = 0;
	c.maxVertexImageUniforms = 0;
	c.maxTessControlImageUniforms = 0;
	c.maxTessEvaluationImageUniforms = 0;
	c.maxGeometryImageUniforms = 0;
	c.maxFragmentImageUniforms = 8;
	c.maxCombinedImageUniforms = 8;
	c.maxGeometryTextureImageUnits = 16;
	c.maxGeometryOutputVertices = 256;
	c.maxGeometryTotalOutputComponents = 1024;
	c.maxGeometryUniformComponents = 1024;
	c.maxGeometryVaryingComponents = 64;
	c.maxTessControlInputComponents = 128;
	c.maxTessControlOutputComponents = 128;
	c.maxTessControlTextureImageUnits = 16;
	c.maxTessControlUniformComponents = 1024;
	c.maxTessControlTotalOutputComponents = 4096;
	c.maxTessEvaluationInputComponents = 128;
	c.maxTessEvaluationOutputComponents = 128;
	c.maxTessEvaluationTextureImageUnits = 16;
	c.maxTessEvaluationUniformComponents = 1024;
	c.maxTessPatchComponents = 120;
	c.maxPatchVertices = 32;
	c.maxTessGenLevel = 64;
	c.maxViewports = 16;
	c.maxVertexAtomicCounters = 0;
	c.maxTessControlAtomicCounters = 0;
	c.maxTessEvaluationAtomicCounters = 0;
	c.maxGeometryAtomicCounters = 0;
	c.maxFragmentAtomicCounters = 8;
	c.maxCombinedAtomicCounters = 8;
	c.maxAtomicCounterBindings = 1;
	c.maxVertexAtomicCounterBuffers = 0;
	c.maxTessControlAtomicCounterBuffers = 0;
	c.maxTessEvaluationAtomicCounterBuffers = 0;
	c.maxGeometryAtomicCounterBuffers = 0;
	c.maxFragmentAtomicCounterBuffers = 1;
	c.maxCombinedAtomicCounterBuffers = 1;
	c.maxAtomicCounterBufferSize = 16384;
	c.maxTransformFeedbackBuffers = 4;
	c.maxTransformFeedbackInterleavedComponents = 64;
	c.maxCullDistances = 8;
	c.maxCombinedClipAndCullDistances = 8;
	c.maxSamples = 4;

	c.limits.nonInductiveForLoops = 1;
	c.limits.whileLoops = 1;
	c.limits.doWhileLoops = 1;
	c.limits.generalUniformIndexing = 1;
	c.limits.generalAttributeMatrixVectorIndexing = 1;
	c.limits.generalVaryingIndexing = 1;
	c.limits.generalSamplerIndexing = 1;
	c.limits.generalVariableIndexing = 1;
	c.limits.generalConstantMatrixVectorIndexing = 1;

	return c;
}

static TBuiltInResource GLSLANG_LIMITS = setGlslangLimits();

static EShLanguage ankiToGlslangShaderType(ShaderType shaderType)
{
	EShLanguage gslangShader;
	switch(shaderType)
	{
	case ShaderType::VERTEX:
		gslangShader = EShLangVertex;
		break;
	case ShaderType::FRAGMENT:
		gslangShader = EShLangFragment;
		break;
	case ShaderType::TESSELLATION_EVALUATION:
		gslangShader = EShLangTessEvaluation;
		break;
	case ShaderType::TESSELLATION_CONTROL:
		gslangShader = EShLangTessControl;
		break;
	case ShaderType::GEOMETRY:
		gslangShader = EShLangGeometry;
		break;
	case ShaderType::COMPUTE:
		gslangShader = EShLangCompute;
		break;
	case ShaderType::RAY_GEN:
		gslangShader = EShLangRayGen;
		break;
	case ShaderType::ANY_HIT:
		gslangShader = EShLangAnyHit;
		break;
	case ShaderType::CLOSEST_HIT:
		gslangShader = EShLangClosestHit;
		break;
	case ShaderType::MISS:
		gslangShader = EShLangMiss;
		break;
	case ShaderType::INTERSECTION:
		gslangShader = EShLangIntersect;
		break;
	case ShaderType::CALLABLE:
		gslangShader = EShLangCallable;
		break;
	default:
		ANKI_ASSERT(0);
		gslangShader = EShLangCount;
	};

	return gslangShader;
}

static void logShaderErrorCode(CString error, CString source, GenericMemoryPoolAllocator<U8> alloc)
{
	StringAuto prettySrc(alloc);
	StringListAuto lines(alloc);

	static const char* padding = "==============================================================================";

	lines.splitString(source, '\n', true);

	I lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd(); ++it)
	{
		++lineno;
		StringAuto tmp(alloc);

		if(!it->isEmpty())
		{
			tmp.sprintf("%8d: %s\n", lineno, &(*it)[0]);
		}
		else
		{
			tmp.sprintf("%8d:\n", lineno);
		}

		prettySrc.append(tmp);
	}

	ANKI_SHADER_COMPILER_LOGE("Shader compilation failed:\n%s\n%s\n%s\n%s\n%s\n%s", padding, &error[0], padding,
							  &prettySrc[0], padding, &error[0]);
}

Error preprocessGlsl(CString in, StringAuto& out)
{
	glslang::TShader shader(EShLangVertex);
	Array<const char*, 1> csrc = {&in[0]};
	shader.setStrings(&csrc[0], 1);

	DirStackFileIncluder includer;
	EShMessages messages = EShMsgDefault;
	std::string glslangOut;
	if(!shader.preprocess(&GLSLANG_LIMITS, 450, ENoProfile, false, false, messages, &glslangOut, includer))
	{
		ANKI_SHADER_COMPILER_LOGE("Preprocessing failed:\n%s", shader.getInfoLog());
		return Error::USER_DATA;
	}

	out.append(glslangOut.c_str());

	return Error::NONE;
}

Error compilerGlslToSpirv(CString src, ShaderType shaderType, GenericMemoryPoolAllocator<U8> tmpAlloc,
						  DynamicArrayAuto<U8>& spirv)
{
	const EShLanguage stage = ankiToGlslangShaderType(shaderType);
	const EShMessages messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);
	const glslang::EShTargetLanguageVersion langVersion = glslang::EShTargetSpv_1_3;

	glslang::TShader shader(stage);
	Array<const char*, 1> csrc = {&src[0]};
	shader.setStrings(&csrc[0], 1);
	shader.setEnvTarget(glslang::EShTargetSpv, langVersion);
	if(!shader.parse(&GLSLANG_LIMITS, 100, false, messages))
	{
		logShaderErrorCode(shader.getInfoLog(), src, tmpAlloc);
		return Error::USER_DATA;
	}

	// Setup the program
	glslang::TProgram program;
	program.addShader(&shader);

	if(!program.link(messages))
	{
		ANKI_SHADER_COMPILER_LOGE("glslang failed to link a shader");
		return Error::USER_DATA;
	}

	// Gen SPIRV
	glslang::SpvOptions spvOptions;
	spvOptions.optimizeSize = true;
	spvOptions.disableOptimizer = false;
	std::vector<unsigned int> glslangSpirv;
	glslang::GlslangToSpv(*program.getIntermediate(stage), glslangSpirv, &spvOptions);

	// Store
	spirv.resize(U32(glslangSpirv.size() * sizeof(unsigned int)));
	memcpy(&spirv[0], &glslangSpirv[0], spirv.getSizeInBytes());

#if 0
	// Dump it
	{
		static U32 count = 0;
		if(count == 0)
		{
			ANKI_SHADER_COMPILER_LOGW("SPIR-V dumping is enabled");
		}

		File file;
		StringAuto fname(tmpAlloc);
		fname.sprintf("/tmp/%u.spv", count++);
		ANKI_CHECK(file.open(fname, FileOpenFlag::WRITE | FileOpenFlag::BINARY));
		ANKI_CHECK(file.write(spirv.getBegin(), spirv.getSizeInBytes()));
	}
#endif

	return Error::NONE;
}

} // end namespace anki
