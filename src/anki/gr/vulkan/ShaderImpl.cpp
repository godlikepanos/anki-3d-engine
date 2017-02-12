// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ShaderImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/common/Misc.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <SPIRV-Cross/spirv_cross.hpp>

#define ANKI_DUMP_SHADERS ANKI_EXTRA_CHECKS

#if ANKI_DUMP_SHADERS
#include <anki/util/File.h>
#include <anki/gr/GrManager.h>
#endif

namespace anki
{

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
	TBuiltInResource c;

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

static const char* SHADER_HEADER = R"(#version 450 core
#define ANKI_VK 1
#define ANKI_VENDOR_%s
#define %s
#define gl_VertexID gl_VertexIndex
#define gl_InstanceID gl_InstanceIndex
#define ANKI_TEX_BINDING(set_, binding_) set = set_, binding = %u + binding_
#define ANKI_UBO_BINDING(set_, binding_) set = set_, binding = %u + binding_
#define ANKI_SS_BINDING(set_, binding_) set = set_, binding = %u + binding_
#define ANKI_IMAGE_BINDING(set_, binding_) set = set_, binding = %u + binding_

#if defined(FRAGMENT_SHADER)
#define ANKI_USING_FRAG_COORD(h_) vec4 anki_fragCoord = \
	vec4(gl_FragCoord.x, h_ - gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
#endif

#if defined(VERTEX_SHADER)
#define ANKI_WRITE_POSITION(x_) gl_Position = x_; gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5
#endif

%s)";

ShaderImpl::~ShaderImpl()
{
	for(auto& x : m_bindings)
	{
		x.destroy(getAllocator());
	}

	if(m_handle)
	{
		vkDestroyShaderModule(getDevice(), m_handle, nullptr);
	}
}

Error ShaderImpl::genSpirv(const CString& source, std::vector<unsigned int>& spirv)
{
	const EShLanguage stage = ankiToGlslangShaderType(m_shaderType);
	const EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

	// Setup the shader
	glslang::TShader shader(stage);
	Array<const char*, 1> csrc = {{&source[0]}};
	shader.setStrings(&csrc[0], 1);
	if(!shader.parse(&GLSLANG_LIMITS, 100, false, messages))
	{
		logShaderErrorCode(shader.getInfoLog(), source, getAllocator());
		return ErrorCode::USER_DATA;
	}

	// Setup the program
	glslang::TProgram program;
	program.addShader(&shader);

	if(!program.link(messages))
	{
		ANKI_VK_LOGE("glslang failed to link a shader");
		return ErrorCode::USER_DATA;
	}

	// Gen SPIRV
	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

	return ErrorCode::NONE;
}

Error ShaderImpl::init(ShaderType shaderType, const CString& source)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(m_handle == VK_NULL_HANDLE);
	m_shaderType = shaderType;

	// Setup the shader
	auto alloc = getAllocator();
	StringAuto fullSrc(alloc);

	static const Array<const char*, 6> shaderName = {{"VERTEX_SHADER",
		"TESSELATION_CONTROL_SHADER",
		"TESSELATION_EVALUATION_SHADER",
		"GEOMETRY_SHADER",
		"FRAGMENT_SHADER",
		"COMPUTE_SHADER"}};

	fullSrc.sprintf(SHADER_HEADER,
		&GPU_VENDOR_STR[getGrManagerImpl().getGpuVendor()][0],
		shaderName[shaderType],
		0,
		MAX_TEXTURE_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS,
		&source[0]);

	std::vector<unsigned int> spirv;
	ANKI_CHECK(genSpirv(fullSrc.toCString(), spirv));
	ANKI_ASSERT(!spirv.empty());

#if ANKI_DUMP_SHADERS
	{
		static U32 name = 0;
		SpinLock m_nameLock;
		const char* ext;

		U32 newName;
		{
			LockGuard<SpinLock> lock(m_nameLock);
			newName = name++;
		}

		switch(shaderType)
		{
		case ShaderType::VERTEX:
			ext = "vert";
			break;
		case ShaderType::TESSELLATION_CONTROL:
			ext = "tesc";
			break;
		case ShaderType::TESSELLATION_EVALUATION:
			ext = "tese";
			break;
		case ShaderType::GEOMETRY:
			ext = "geom";
			break;
		case ShaderType::FRAGMENT:
			ext = "frag";
			break;
		case ShaderType::COMPUTE:
			ext = "comp";
			break;
		default:
			ext = nullptr;
			ANKI_ASSERT(0);
		}

		StringAuto fname(alloc);
		CString cacheDir = getGrManager().getCacheDirectory();
		fname.sprintf("%s/%05u.%s", &cacheDir[0], newName, ext);

		File file;
		ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));
		ANKI_CHECK(file.writeText("%s", &fullSrc[0]));

		StringAuto fnameSpirv(alloc);
		fnameSpirv.sprintf("%s/%05u.%s.spv", &cacheDir[0], newName, ext);

		File fileSpirv;
		ANKI_CHECK(fileSpirv.open(fnameSpirv.toCString(), FileOpenFlag::BINARY | FileOpenFlag::WRITE));
		ANKI_CHECK(fileSpirv.write(&spirv[0], spirv.size() * sizeof(unsigned int)));
	}
#endif

	VkShaderModuleCreateInfo ci = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, spirv.size() * sizeof(unsigned int), &spirv[0]};

	ANKI_VK_CHECK(vkCreateShaderModule(getDevice(), &ci, nullptr, &m_handle));

	// Get reflection info
	doReflection(spirv);

	return ErrorCode::NONE;
}

void ShaderImpl::doReflection(const std::vector<unsigned int>& spirv)
{
	spirv_cross::Compiler spvc(spirv);
	spirv_cross::ShaderResources rsrc = spvc.get_shader_resources(spvc.get_active_interface_variables());

	Array<U, MAX_DESCRIPTOR_SETS> counts = {{
		0,
	}};
	Array2d<DescriptorBinding, MAX_DESCRIPTOR_SETS, MAX_BINDINGS_PER_DESCRIPTOR_SET> descriptors;

	auto func = [&](const std::vector<spirv_cross::Resource>& resources, DescriptorType type) -> void {
		for(const spirv_cross::Resource& r : resources)
		{
			const U32 id = r.id;
			const U32 set = spvc.get_decoration(id, spv::Decoration::DecorationDescriptorSet);
			const U32 binding = spvc.get_decoration(id, spv::Decoration::DecorationBinding);

			m_descriptorSetMask.set(set);
			m_activeBindingMask[set].set(set);

			// Check that there are no other descriptors with the same binding
			for(U i = 0; i < counts[set]; ++i)
			{
				ANKI_ASSERT(descriptors[set][i].m_binding != binding);
			}

			DescriptorBinding& descriptor = descriptors[set][counts[set]++];
			descriptor.m_binding = binding;
			descriptor.m_type = type;
			descriptor.m_stageMask = static_cast<ShaderTypeBit>(1 << m_shaderType);
		}
	};

	func(rsrc.uniform_buffers, DescriptorType::UNIFORM_BUFFER);
	func(rsrc.sampled_images, DescriptorType::TEXTURE);
	func(rsrc.storage_buffers, DescriptorType::STORAGE_BUFFER);
	func(rsrc.storage_images, DescriptorType::IMAGE);

	for(U set = 0; set < MAX_DESCRIPTOR_SETS; ++set)
	{
		if(counts[set])
		{
			m_bindings[set].create(getAllocator(), counts[set]);
			memcpy(&m_bindings[set][0], &descriptors[set][0], counts[set] * sizeof(DescriptorBinding));
		}
	}

	// Color attachments
	if(m_shaderType == ShaderType::FRAGMENT)
	{
		for(const spirv_cross::Resource& r : rsrc.stage_outputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			m_colorAttachmentWritemask.set(location);
		}
	}

	// Attribs
	if(m_shaderType == ShaderType::VERTEX)
	{
		for(const spirv_cross::Resource& r : rsrc.stage_inputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			m_attributeMask.set(location);
		}
	}
}

} // end namespace anki
