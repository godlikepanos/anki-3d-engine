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
	m_sslrEnabled = config.getNumber("sslr.enabled") && m_r->getIrEnabled();

	// Size
	m_width = m_r->getWidth() / 2;
	m_height = m_r->getHeight() / 2;

	// Continue
	GrManager& gr = getGrManager();

	// Create shader
	StringAuto pps(getFrameAllocator());
	const PixelFormat& pixFormat = Is::RT_PIXEL_FORMAT;

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
		(m_r->getIrEnabled()) ? m_r->getIr().getClusterer().getClusterCountX()
							  : 0,
		(m_r->getIrEnabled()) ? m_r->getIr().getClusterer().getClusterCountY()
							  : 0,
		U(m_sslrEnabled),
		U(m_r->getIrEnabled()),
		(m_r->getIrEnabled()) ? m_r->getIr().getCubemapArrayMipmapCount() : 0,
		Renderer::TILE_SIZE,
		config.getNumber("sslr.startRoughnes"));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_frag, "shaders/Refl.frag.glsl", pps.toCString(), "r_refl_"));

	// Create vert shader
	pps.destroy();
	pps.sprintf("#define UV_OFFSET vec2(%f, %f)",
		(1.0 / m_width) / 2.0,
		(1.0 / m_height) / 2.0);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_vert, "shaders/Quad.vert.glsl", pps.toCString(), "r_refl_"));

	// Create ppline
	PipelineInitializer ppinit;
	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = pixFormat;
	ppinit.m_depthStencil.m_depthWriteEnabled = true;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;
	ppinit.m_depthStencil.m_format = Ms::DEPTH_RT_PIXEL_FORMAT;
	ppinit.m_shaders[ShaderType::VERTEX] = m_vert->getGrShader();
	ppinit.m_shaders[ShaderType::FRAGMENT] = m_frag->getGrShader();

	m_ppline = gr.newInstance<Pipeline>(ppinit);

	// Create RC group
	ResourceGroupInitializer rcInit;

	SamplerInitializer sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	rcInit.m_textures[0].m_texture = m_r->getMs().getDepthRt();
	rcInit.m_textures[0].m_sampler = gr.newInstance<Sampler>(sinit);

	rcInit.m_textures[1].m_texture = m_r->getMs().getRt0();
	rcInit.m_textures[1].m_sampler = rcInit.m_textures[0].m_sampler;

	rcInit.m_textures[2].m_texture = m_r->getMs().getRt1();
	rcInit.m_textures[2].m_sampler = rcInit.m_textures[0].m_sampler;

	rcInit.m_textures[3].m_texture = m_r->getMs().getRt2();
	rcInit.m_textures[3].m_sampler = rcInit.m_textures[0].m_sampler;

	if(m_sslrEnabled)
	{
		rcInit.m_textures[4].m_texture = m_r->getIs().getRt();
	}

	rcInit.m_uniformBuffers[0].m_dynamic = true;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Create RTs
	m_r->createRenderTarget(
		m_width, m_height, pixFormat, 1, SamplingFilter::NEAREST, 1, m_rt);

	// Create FB
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 2;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;

	fbInit.m_colorAttachments[1].m_texture = m_r->getMs().getRt2();
	fbInit.m_colorAttachments[1].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[1].m_mipmap = 1;

	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().getDepthRt();
	fbInit.m_depthStencilAttachment.m_format = Ms::DEPTH_RT_PIXEL_FORMAT;
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_mipmap = 1;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
void Refl::run(CommandBufferPtr cmdb)
{
	cmdb->bindFramebuffer(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->bindPipeline(m_ppline);

	// Bind first resource group
	DynamicBufferInfo dyn;

	const FrustumComponent& frc = m_r->getActiveFrustumComponent();
	ReflUniforms* blk = static_cast<ReflUniforms*>(
		getGrManager().allocateFrameHostVisibleMemory(sizeof(ReflUniforms),
			BufferUsage::UNIFORM,
			dyn.m_uniformBuffers[0]));
	blk->m_projectionParams = m_r->getProjectionParameters();
	blk->m_projectionMat = frc.getProjectionMatrix();

	cmdb->bindResourceGroup(m_rcGroup, 0, &dyn);

	// Bind second resource group
	if(m_r->getIrEnabled())
	{
		DynamicBufferInfo dyn;

		dyn.m_storageBuffers[0] = m_r->getIr().getProbesToken();
		dyn.m_storageBuffers[1] = m_r->getIr().getProbeIndicesToken();
		dyn.m_storageBuffers[2] = m_r->getIr().getClustersToken();

		cmdb->bindResourceGroup(m_r->getIr().getResourceGroup(), 1, &dyn);
	};

	m_r->drawQuad(cmdb);
}

} // end namespace anki
