// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Glslang.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wundef"
#	pragma GCC diagnostic ignored "-Wconversion"
#endif
#define ENABLE_OPT 0
#include <Glslang/glslang/Public/ShaderLang.h>
#include <Glslang/SPIRV/GlslangToSpv.h>
#include <Glslang/StandAlone/DirStackFileIncluder.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

#define ANKI_GLSLANG_DUMP 1

namespace anki {

#if ANKI_GLSLANG_DUMP
static Atomic<U32> g_dumpFileCount;
#endif

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

/// Parse Glslang's error message for the line of the error.
static ANKI_USE_RESULT Error parseErrorLine(CString error, GenericMemoryPoolAllocator<U8> alloc, U32& lineNumber)
{
	lineNumber = MAX_U32;

	StringListAuto lines(alloc);
	lines.splitString(error, '\n');
	for(String& line : lines)
	{
		if(line.find("ERROR: ") == 0)
		{
			StringListAuto tokens(alloc);
			tokens.splitString(line, ':');

			if(tokens.getSize() < 3 || (tokens.getBegin() + 2)->toNumber(lineNumber) != Error::NONE)
			{

				ANKI_SHADER_COMPILER_LOGE("Failed to parse the error message: %s", error.cstr());
				return Error::FUNCTION_FAILED;
			}
			else
			{
				break;
			}
		}
	}

	return Error::NONE;
}

static ANKI_USE_RESULT Error logShaderErrorCode(CString error, CString source, GenericMemoryPoolAllocator<U8> alloc)
{
	U32 errorLineNumber = 0;
	ANKI_CHECK(parseErrorLine(error, alloc, errorLineNumber));

	StringAuto prettySrc(alloc);
	StringListAuto lines(alloc);
	StringAuto errorLineTxt(alloc);

	static const char* padding = "==============================================================================";

	lines.splitString(source, '\n', true);

	U32 lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd(); ++it)
	{
		++lineno;
		StringAuto tmp(alloc);

		if(!it->isEmpty() && lineno == errorLineNumber)
		{
			tmp.sprintf(">>%8u: %s\n", lineno, &(*it)[0]);
			errorLineTxt.sprintf("%s", &(*it)[0]);
		}
		else if(!it->isEmpty())
		{
			tmp.sprintf("  %8u: %s\n", lineno, &(*it)[0]);
		}
		else
		{
			tmp.sprintf("  %8u:\n", lineno);
		}

		prettySrc.append(tmp);
	}

	ANKI_SHADER_COMPILER_LOGE("Shader compilation failed:\n%s\n%s\nIn: %s\n%s\n%s\n%s\n%s\nIn: %s\n", padding,
							  &error[0], errorLineTxt.cstr(), padding, &prettySrc[0], padding, &error[0],
							  errorLineTxt.cstr());

	return Error::NONE;
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
#if ANKI_GLSLANG_DUMP
	// Dump it
	const U32 dumpFileCount = g_dumpFileCount.fetchAdd(1);
	{
		File file;

		StringAuto tmpDir(tmpAlloc);
		ANKI_CHECK(getTempDirectory(tmpDir));

		StringAuto fname(tmpAlloc);
		fname.sprintf("%s/%u.glsl", tmpDir.cstr(), dumpFileCount);
		ANKI_SHADER_COMPILER_LOGW("GLSL dumping is enabled: %s", fname.cstr());
		ANKI_CHECK(file.open(fname, FileOpenFlag::WRITE));
		ANKI_CHECK(file.writeText("%s", src.cstr()));
	}
#endif

	const EShLanguage stage = ankiToGlslangShaderType(shaderType);
	const EShMessages messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);

	glslang::TShader shader(stage);
	Array<const char*, 1> csrc = {&src[0]};
	shader.setStrings(&csrc[0], 1);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
	if(!shader.parse(&GLSLANG_LIMITS, 100, false, messages))
	{
		ANKI_CHECK(logShaderErrorCode(shader.getInfoLog(), src, tmpAlloc));
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

#if ANKI_GLSLANG_DUMP
	// Dump it
	{
		File file;

		StringAuto tmpDir(tmpAlloc);
		ANKI_CHECK(getTempDirectory(tmpDir));

		StringAuto fname(tmpAlloc);
		fname.sprintf("%s/%u.spv", tmpDir.cstr(), dumpFileCount);
		ANKI_SHADER_COMPILER_LOGW("GLSL dumping is enabled: %s", fname.cstr());
		ANKI_CHECK(file.open(fname, FileOpenFlag::WRITE | FileOpenFlag::BINARY));
		ANKI_CHECK(file.write(spirv.getBegin(), spirv.getSizeInBytes()));
	}
#endif

	return Error::NONE;
}

} // end namespace anki
