// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderCompiler.h>
#include <anki/gr/GrManager.h>
#include <anki/core/Trace.h>
#include <anki/util/StringList.h>
#include <anki/util/Filesystem.h>
#include <anki/core/Trace.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <SPIRV-Cross/spirv_glsl.hpp>

namespace anki
{

void ShaderCompilerOptions::setFromGrManager(const GrManager& gr)
{
#if ANKI_GR_BACKEND == ANKI_GR_BACKEND_VULKAN
	m_outLanguage = ShaderLanguage::SPIRV;
#else
	m_outLanguage = ShaderLanguage::GLSL;
#endif
	m_gpuCapabilities = gr.getDeviceCapabilities();
}

static const Array<const char*, U(ShaderType::COUNT)> SHADER_NAME = {
	{"VERTEX", "TESSELATION_CONTROL", "TESSELATION_EVALUATION", "GEOMETRY", "FRAGMENT", "COMPUTE"}};

static const char* SHADER_HEADER = R"(#version 450 core
#define ANKI_BACKEND_%s 1
#define ANKI_VENDOR_%s 1
#define ANKI_%s_SHADER 1

#if defined(ANKI_BACKEND_GL) 
#	define ANKI_UBO_BINDING(set_, binding_) binding = set_ * %u + binding_
#	define ANKI_SS_BINDING(set_, binding_) binding = set_ * %u + binding_
#	define ANKI_TEX_BINDING(set_, binding_) binding = set_ * %u + binding_
#	define ANKI_IMAGE_BINDING(set_, binding_) binding = set_ * %u + binding_
#	define ANKI_SPEC_CONST(binding_, type_, name_) const type_ name_ = _anki_spec_const_ ## binding_
#	define ANKI_PUSH_CONSTANTS(struct_, name_) layout(location = %u, row_major) uniform struct_ name_
#else
#	define gl_VertexID gl_VertexIndex
#	define gl_InstanceID gl_InstanceIndex
#	define ANKI_TEX_BINDING(set_, binding_) set = set_, binding = %u + binding_
#	define ANKI_UBO_BINDING(set_, binding_) set = set_, binding = %u + binding_
#	define ANKI_SS_BINDING(set_, binding_) set = set_, binding = %u + binding_
#	define ANKI_IMAGE_BINDING(set_, binding_) set = set_, binding = %u + binding_
#	define ANKI_SPEC_CONST(binding_, type_, name_) layout(constant_id = binding_) const type_ name_ = type_(0)
#	define ANKI_PUSH_CONSTANTS(struct_, name_) layout(push_constant, row_major, std140) \
		uniform pushConst_ {struct_ name_;}
#endif

#if %u
#	extension GL_ARB_shader_ballot : require
#	define ANKI_ARB_SHADER_BALLOT 1
#endif

%s)";

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
	default:
		ANKI_ASSERT(0);
		gslangShader = EShLangCount;
	};

	return gslangShader;
}

static TBuiltInResource setLimits()
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

static const TBuiltInResource GLSLANG_LIMITS = setLimits();

class ShaderCompiler::BuildContext
{
public:
	CString m_originalSrc;
	CString m_src;
	ShaderCompilerOptions m_options;
	GenericMemoryPoolAllocator<U8> m_alloc;
};

static ANKI_USE_RESULT Error genSpirv(const ShaderCompiler::BuildContext& ctx, std::vector<unsigned int>& spirv)
{
	const EShLanguage stage = ankiToGlslangShaderType(ctx.m_options.m_shaderType);

	EShMessages messages = EShMsgSpvRules;
	if(ctx.m_options.m_outLanguage == ShaderLanguage::SPIRV)
	{
		messages = static_cast<EShMessages>(messages | EShMsgVulkanRules);
	}

	// Setup the shader
	glslang::TShader shader(stage);
	Array<const char*, 1> csrc = {{&ctx.m_src[0]}};
	shader.setStrings(&csrc[0], 1);
	if(!shader.parse(&GLSLANG_LIMITS, 100, false, messages))
	{
		ShaderCompiler::logShaderErrorCode(shader.getInfoLog(), ctx.m_src, ctx.m_alloc);
		return Error::USER_DATA;
	}

	// Setup the program
	glslang::TProgram program;
	program.addShader(&shader);

	if(!program.link(messages))
	{
		ANKI_GR_LOGE("glslang failed to link a shader");
		return Error::USER_DATA;
	}

	// Gen SPIRV
	glslang::SpvOptions spvOptions;
	spvOptions.optimizeSize = true;
	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &spvOptions);

	return Error::NONE;
}

I32 ShaderCompiler::m_refcount = {0};
Mutex ShaderCompiler::m_refcountMtx;

ShaderCompiler::ShaderCompiler(GenericMemoryPoolAllocator<U8> alloc)
	: m_alloc(alloc)
{
	LockGuard<Mutex> lock(m_refcountMtx);

	const I32 refcount = m_refcount++;
	ANKI_ASSERT(refcount >= 0);

	if(refcount == 0)
	{
		glslang::InitializeProcess();
	}
}

ShaderCompiler::~ShaderCompiler()
{
	LockGuard<Mutex> lock(m_refcountMtx);

	const I32 refcount = m_refcount--;
	ANKI_ASSERT(refcount >= 0);

	if(refcount == 1)
	{
		glslang::FinalizeProcess();
	}
}

Error ShaderCompiler::compile(CString source, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& bin) const
{
	ANKI_ASSERT(!source.isEmpty() && source.getLength() > 0);
	Error err = Error::NONE;

	ANKI_TRACE_SCOPED_EVENT(GR_SHADER_COMPILE);

	// Create the context
	BuildContext ctx;
	ctx.m_originalSrc = source;
	ctx.m_options = options;
	ctx.m_alloc = m_alloc;

	// Gen the new source
	StringAuto fullSrc(m_alloc);

	fullSrc.sprintf(SHADER_HEADER,
		(options.m_outLanguage == ShaderLanguage::GLSL) ? "GL" : "VULKAN",
		&GPU_VENDOR_STR[options.m_gpuCapabilities.m_gpuVendor][0],
		SHADER_NAME[options.m_shaderType],
		// GL bindings
		MAX_UNIFORM_BUFFER_BINDINGS,
		MAX_STORAGE_BUFFER_BINDINGS,
		MAX_TEXTURE_BINDINGS,
		MAX_IMAGE_BINDINGS,
		MAX_TEXTURE_BINDINGS * MAX_DESCRIPTOR_SETS, // Push constant location
		// VK bindings
		0,
		MAX_TEXTURE_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS,
		// Ballot
		!!(options.m_gpuCapabilities.m_shaderSubgroups) ? 1u : 0u,
		&source[0]);

	ctx.m_src = fullSrc.toCString();

	// Compile
	if(options.m_outLanguage == ShaderLanguage::GLSL)
	{
#if 0
		std::vector<unsigned int> spv;
		err = genSpirv(ctx, spv);
		if(!err)
		{
			spirv_cross::CompilerGLSL cross(spv);
			std::string newSrc = cross.compile();

			bin.resize(newSrc.length() + 1);
			memcpy(&bin[0], &newSrc[0], bin.getSize());
		}
#else
		bin.resize(fullSrc.getLength() + 1);
		memcpy(&bin[0], &fullSrc[0], bin.getSize());
#endif
	}
	else
	{
		std::vector<unsigned int> spv;
		err = genSpirv(ctx, spv);
		if(!err)
		{
			bin.resize(spv.size() * sizeof(unsigned int));
			memcpy(&bin[0], &spv[0], bin.getSize());
		}
	}

	return err;
}

void ShaderCompiler::logShaderErrorCode(CString error, CString source, GenericMemoryPoolAllocator<U8> alloc)
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

	ANKI_GR_LOGE("Shader compilation failed:\n%s\n%s\n%s\n%s\n%s\n%s",
		padding,
		&error[0],
		padding,
		&prettySrc[0],
		padding,
		&error[0]);
}

Error ShaderCompilerCache::compile(
	CString source, U64* hash, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& bin) const
{
	Error err = compileInternal(source, hash, options, bin);
	if(err)
	{
		ANKI_GR_LOGE("Failed to compile or retrieve shader from the cache");
	}

	return err;
}

Error ShaderCompilerCache::compileInternal(
	CString source, U64* hash, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& bin) const
{
	ANKI_ASSERT(!source.isEmpty() && source.getLength() > 0);

	// Compute hash
	U64 fhash;
	if(hash)
	{
		fhash = *hash;
		ANKI_ASSERT(fhash != 0);
	}
	else
	{
		fhash = computeHash(&source[0], source.getLength());
	}

	fhash = appendHash(&options, sizeof(options), fhash);

	// Search the cache
	StringAuto fname(m_alloc);
	fname.sprintf("%s/%llu.shdrbin", m_cacheDir.cstr(), fhash);
	if(fileExists(fname.toCString()))
	{
		File file;
		ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::READ | FileOpenFlag::BINARY));

		PtrSize size = file.getSize();
		bin.resize(size);
		ANKI_CHECK(file.read(&bin[0], bin.getSize()));
	}
	else
	{
		ANKI_CHECK(m_compiler.compile(source, options, bin));

		File file;
		ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE | FileOpenFlag::BINARY));

		ANKI_CHECK(file.write(&bin[0], bin.getSize()));
	}

	return Error::NONE;
}

} // end namespace anki
