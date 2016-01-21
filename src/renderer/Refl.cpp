// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Refl.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

struct ReflUniforms
{
	Vec4 m_projectionParams;
	Mat4 m_projectionMat;
};

//==============================================================================
// Refl                                                                        =
//==============================================================================

//==============================================================================
Refl::Refl(Renderer* r)
	: RenderingPass(r)
{
}

//==============================================================================
Refl::~Refl()
{
}

//==============================================================================
Error Refl::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init reflections");
	}

	return err;
}

//==============================================================================
Error Refl::initInternal(const ConfigSet& config)
{
	m_irEnabled = config.getNumber("ir.enabled");
	m_sslrEnabled = config.getNumber("sslr.enabled");

	m_enabled = m_irEnabled || m_sslrEnabled;
	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	// Size
	m_width = m_r->getWidth() / 2;
	m_height = m_r->getHeight() / 2;

	// IR
	if(m_irEnabled)
	{
		m_ir.reset(getAllocator().newInstance<Ir>(m_r));
		ANKI_CHECK(m_ir->init(config));
	}

	// Continue
	ANKI_CHECK(init1stPass(config));
	ANKI_CHECK(init2ndPass());

	return ErrorCode::NONE;
}

//==============================================================================
Error Refl::init1stPass(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	// Create shader
	StringAuto pps(getFrameAllocator());
	const PixelFormat& pixFormat = Pps::RT_PIXEL_FORMAT;

	pps.sprintf("#define WIDTH %u\n"
				"#define HEIGHT %u\n"
				"#define TILE_COUNT_X %u\n"
				"#define TILE_COUNT_Y %u\n"
				"#define SSLR_ENABLED %u\n"
				"#define IR_ENABLED %u\n"
				"#define IR_MIPMAP_COUNT %u\n"
				"#define TILE_SIZE %u\n"
				"#define SSLR_START_ROUGHNESS %f\n",
		m_width,
		m_height,
		(m_irEnabled) ? m_ir->getClusterer().getClusterCountX() : 0,
		(m_irEnabled) ? m_ir->getClusterer().getClusterCountY() : 0,
		U(m_sslrEnabled),
		U(m_irEnabled),
		(m_irEnabled) ? m_ir->getCubemapArrayMipmapCount() : 0,
		Renderer::TILE_SIZE,
		config.getNumber("sslr.startRoughnes"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_frag, "shaders/Refl.frag.glsl", pps.toCString(), "r_refl_"));

	// Create ppline
	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = pixFormat;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorState, m_ppline);

	// Create uniform buffer
	m_uniforms = getGrManager().newInstance<Buffer>(sizeof(ReflUniforms),
		BufferUsageBit::UNIFORM,
		BufferAccessBit::CLIENT_WRITE);

	// Create RC group
	ResourceGroupInitializer rcInit;
	SamplerInitializer sinit;

	sinit.m_minLod = 1.0;
	sinit.m_minMagFilter = SamplingFilter::NEAREST;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	rcInit.m_textures[0].m_texture = m_r->getMs().getDepthRt();
	rcInit.m_textures[0].m_sampler = gr.newInstance<Sampler>(sinit);

	rcInit.m_textures[1].m_texture = m_r->getMs().getRt2();
	rcInit.m_textures[1].m_sampler = gr.newInstance<Sampler>(sinit);

	if(m_sslrEnabled)
	{
		rcInit.m_textures[3].m_texture = m_r->getIs().getRt();
	}

	if(m_irEnabled)
	{
		rcInit.m_textures[4].m_texture = m_ir->getEnvironmentCubemapArray();
	}

	rcInit.m_uniformBuffers[0].m_buffer = m_uniforms;

	if(m_irEnabled)
	{
		rcInit.m_storageBuffers[0].m_dynamic = true;
		rcInit.m_storageBuffers[1].m_dynamic = true;
		rcInit.m_storageBuffers[2].m_dynamic = true;
	}

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Create RT
	m_r->createRenderTarget(
		m_width, m_height, pixFormat, 1, SamplingFilter::NEAREST, 1, m_rt);

	// Create FB
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource(
		"engine_data/SplitSumIntegration.ankitex", m_integrationLut));

	sinit = SamplerInitializer();
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_repeat = false;
	m_integrationLutSampler = getGrManager().newInstance<Sampler>(sinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Refl::init2ndPass()
{
	GrManager& gr = getGrManager();

	// Create RC group
	ResourceGroupInitializer rcInit;

	rcInit.m_textures[0].m_texture = m_r->getMs().getDepthRt();

	SamplerInitializer sinit;
	sinit.m_repeat = false;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	rcInit.m_textures[1].m_texture = m_rt;
	rcInit.m_textures[1].m_sampler = gr.newInstance<Sampler>(sinit);

	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	rcInit.m_textures[2].m_texture = m_rt;
	rcInit.m_textures[2].m_sampler = gr.newInstance<Sampler>(sinit);

	rcInit.m_uniformBuffers[0].m_dynamic = true;

	m_blitRcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Shader
	StringAuto pps(getFrameAllocator());
	pps.sprintf("#define TEXTURE_WIDTH %uu\n"
				"#define TEXTURE_HEIGHT %uu\n",
		m_width,
		m_height);

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_blitFrag,
		"shaders/NearDepthUpscale.frag.glsl",
		pps.toCString(),
		"r_refl_"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_blitVert,
		"shaders/NearDepthUpscale.vert.glsl",
		pps.toCString(),
		"r_refl_"));

	// Ppline
	PipelineInitializer ppinit;

	ppinit.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;

	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	ppinit.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	ppinit.m_color.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;

	ppinit.m_shaders[U(ShaderType::VERTEX)] = m_blitVert->getGrShader();
	ppinit.m_shaders[U(ShaderType::FRAGMENT)] = m_blitFrag->getGrShader();
	m_blitPpline = gr.newInstance<Pipeline>(ppinit);

	// Create FB
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::LOAD;
	m_isFb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Refl::run1(CommandBufferPtr cmdb)
{
	ANKI_ASSERT(m_enabled);
	return m_ir->run(cmdb);
}

//==============================================================================
void Refl::run2(CommandBufferPtr cmdb)
{
	ANKI_ASSERT(m_enabled);

	// Compute the reflection
	//
	writeUniforms(cmdb);

	cmdb->bindFramebuffer(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->bindPipeline(m_ppline);

	DynamicBufferInfo dyn;
	DynamicBufferInfo* pdyn = nullptr;
	if(m_irEnabled)
	{
		dyn.m_storageBuffers[0] = m_ir->getProbesToken();
		dyn.m_storageBuffers[1] = m_ir->getProbeIndicesToken();
		dyn.m_storageBuffers[2] = m_ir->getClustersToken();
		pdyn = &dyn;
	};
	cmdb->bindResourceGroup(m_rcGroup, 0, pdyn);

	m_r->drawQuad(cmdb);

	// Write the reflection back to IS RT
	//
	DynamicBufferToken token;
	Vec4* linearDepth =
		static_cast<Vec4*>(getGrManager().allocateFrameHostVisibleMemory(
			sizeof(Vec4), BufferUsage::UNIFORM, token));
	const Frustum& fr = m_r->getActiveFrustumComponent().getFrustum();
	computeLinearizeDepthOptimal(
		fr.getNear(), fr.getFar(), linearDepth->x(), linearDepth->y());

	DynamicBufferInfo dyn1;
	dyn1.m_uniformBuffers[0] = token;

	cmdb->bindFramebuffer(m_isFb);
	cmdb->bindPipeline(m_blitPpline);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindResourceGroup(m_blitRcGroup, 0, &dyn1);

	m_r->drawQuad(cmdb);
}

//==============================================================================
void Refl::writeUniforms(CommandBufferPtr cmdb)
{
	const FrustumComponent& frc = m_r->getActiveFrustumComponent();

	if(m_uniformsUpdateTimestamp < m_r->getProjectionParametersUpdateTimestamp()
		|| m_uniformsUpdateTimestamp < frc.getTimestamp()
		|| m_uniformsUpdateTimestamp == 0)
	{
		DynamicBufferToken token;
		ReflUniforms* blk = static_cast<ReflUniforms*>(
			getGrManager().allocateFrameHostVisibleMemory(
				sizeof(ReflUniforms), BufferUsage::TRANSFER, token));

		blk->m_projectionParams = m_r->getProjectionParameters();
		blk->m_projectionMat = frc.getProjectionMatrix();

		cmdb->writeBuffer(m_uniforms, 0, token);

		m_uniformsUpdateTimestamp = getGlobalTimestamp();
	}
}

} // end namespace anki
