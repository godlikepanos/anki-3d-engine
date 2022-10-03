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

#define ANKI_GLSLANG_DUMP 0

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
	case ShaderType::kVertex:
		gslangShader = EShLangVertex;
		break;
	case ShaderType::kFragment:
		gslangShader = EShLangFragment;
		break;
	case ShaderType::kTessellationEvaluation:
		gslangShader = EShLangTessEvaluation;
		break;
	case ShaderType::kTessellationControl:
		gslangShader = EShLangTessControl;
		break;
	case ShaderType::kGeometry:
		gslangShader = EShLangGeometry;
		break;
	case ShaderType::kCompute:
		gslangShader = EShLangCompute;
		break;
	case ShaderType::kRayGen:
		gslangShader = EShLangRayGen;
		break;
	case ShaderType::kAnyHit:
		gslangShader = EShLangAnyHit;
		break;
	case ShaderType::kClosestHit:
		gslangShader = EShLangClosestHit;
		break;
	case ShaderType::kMiss:
		gslangShader = EShLangMiss;
		break;
	case ShaderType::kIntersection:
		gslangShader = EShLangIntersect;
		break;
	case ShaderType::kCallable:
		gslangShader = EShLangCallable;
		break;
	default:
		ANKI_ASSERT(0);
		gslangShader = EShLangCount;
	};

	return gslangShader;
}

/// Parse Glslang's error message for the line of the error.
static Error parseErrorLine(CString error, BaseMemoryPool& pool, U32& lineNumber)
{
	lineNumber = kMaxU32;

	StringListRaii lines(&pool);
	lines.splitString(error, '\n');
	for(String& line : lines)
	{
		if(line.find("ERROR: ") == 0)
		{
			StringListRaii tokens(&pool);
			tokens.splitString(line, ':');

			if(tokens.getSize() < 3 || (tokens.getBegin() + 2)->toNumber(lineNumber) != Error::kNone)
			{

				ANKI_SHADER_COMPILER_LOGE("Failed to parse the GLSlang error message: %s", error.cstr());
				return Error::kFunctionFailed;
			}
			else
			{
				break;
			}
		}
	}

	return Error::kNone;
}

static void createErrorLog(CString glslangError, CString source, BaseMemoryPool& pool, StringRaii& outError)
{
	U32 errorLineNumberu = 0;
	const Error err = parseErrorLine(glslangError, pool, errorLineNumberu);

	const I32 errorLineNumber = (!err) ? I32(errorLineNumberu) : -1;

	constexpr I32 lineCountAroundError = 4;

	StringRaii prettySrc(&pool);
	StringListRaii lines(&pool);

	lines.splitString(source, '\n', true);

	I32 lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd(); ++it)
	{
		++lineno;

		if(lineno >= errorLineNumber - lineCountAroundError && lineno <= errorLineNumber + lineCountAroundError)
		{
			prettySrc.append(StringRaii(&pool).sprintf("%s%s\n", (lineno == errorLineNumber) ? ">>  " : "    ",
													   (it->isEmpty()) ? " " : (*it).cstr()));
		}
	}

	outError.sprintf("%sIn:\n%s\n", glslangError.cstr(), prettySrc.cstr());
}

Error preprocessGlsl(CString in, StringRaii& out)
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
		return Error::kUserData;
	}

	out.append(glslangOut.c_str());

	return Error::kNone;
}

Error compilerGlslToSpirv(CString src, ShaderType shaderType, BaseMemoryPool& tmpPool, DynamicArrayRaii<U8>& spirv,
						  StringRaii& errorMessage)
{
#if ANKI_GLSLANG_DUMP
	// Dump it
	const U32 dumpFileCount = g_dumpFileCount.fetchAdd(1);
	{
		File file;

		StringRaii tmpDir(&tmpPool);
		ANKI_CHECK(getTempDirectory(tmpDir));

		StringRaii fname(&tmpPool);
		fname.sprintf("%s/%u.glsl", tmpDir.cstr(), dumpFileCount);
		ANKI_SHADER_COMPILER_LOGW("GLSL dumping is enabled: %s", fname.cstr());
		ANKI_CHECK(file.open(fname, FileOpenFlag::kWrite));
		ANKI_CHECK(file.writeTextf("%s", src.cstr()));
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
		createErrorLog(shader.getInfoLog(), src, tmpPool, errorMessage);
		return Error::kUserData;
	}

	// Setup the program
	glslang::TProgram program;
	program.addShader(&shader);

	if(!program.link(messages))
	{
		errorMessage.create("glslang failed to link a shader");
		return Error::kUserData;
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

		StringRaii tmpDir(&tmpPool);
		ANKI_CHECK(getTempDirectory(tmpDir));

		StringRaii fname(&tmpPool);
		fname.sprintf("%s/%u.spv", tmpDir.cstr(), dumpFileCount);
		ANKI_SHADER_COMPILER_LOGW("GLSL dumping is enabled: %s", fname.cstr());
		ANKI_CHECK(file.open(fname, FileOpenFlag::kWrite | FileOpenFlag::kBinary));
		ANKI_CHECK(file.write(spirv.getBegin(), spirv.getSizeInBytes()));
	}
#endif

	return Error::kNone;
}

} // end namespace anki
