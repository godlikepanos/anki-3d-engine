// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Refl.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/FrustumComponent.h>

namespace anki {

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
{}

//==============================================================================
Refl::~Refl()
{}

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
	const F32 quality = config.getNumber("refl.renderingQuality");

	m_width = getAlignedRoundUp(Renderer::TILE_SIZE, quality * m_r->getWidth());
	m_height =
		getAlignedRoundUp(Renderer::TILE_SIZE, quality * m_r->getHeight());

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
	// Create shader
	StringAuto pps(getFrameAllocator());

	pps.sprintf(
		"#define WIDTH %u\n"
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

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_frag,
		"shaders/Refl.frag.glsl", pps.toCString(), "r_refl_"));

	// Create ppline
	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorState, m_ppline);

	// Create uniform buffer
	m_uniforms = getGrManager().newInstance<Buffer>(sizeof(ReflUniforms),
		BufferUsageBit::UNIFORM, BufferAccessBit::CLIENT_WRITE);

	// Create RC group
	ResourceGroupInitializer rcInit;
	rcInit.m_textures[0].m_texture = m_r->getMs().getDepthRt();
	rcInit.m_textures[1].m_texture = m_r->getMs().getRt1();
	rcInit.m_textures[2].m_texture = m_r->getMs().getRt2();

	if(m_sslrEnabled)
	{
		rcInit.m_textures[3].m_texture = m_r->getIs().getRt();
	}

	if(m_irEnabled)
	{
		rcInit.m_textures[4].m_texture = m_ir->getCubemapArray();
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
	m_r->createRenderTarget(m_width, m_height,
		Is::RT_PIXEL_FORMAT, 1, SamplingFilter::LINEAR, 1, m_rt);

	// Create FB
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Refl::init2ndPass()
{
	// Create RC group
	ResourceGroupInitializer rcInit;
	rcInit.m_textures[0].m_texture = m_rt;

	m_blitRcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	// Shader
	ANKI_CHECK(getResourceManager().loadResource(
		"shaders/Blit.frag.glsl", m_blitFrag));

	// Ppline
	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	colorState.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	colorState.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;

	m_r->createDrawQuadPipeline(m_blitFrag->getGrShader(), colorState,
		m_blitPpline);

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
	cmdb->bindFramebuffer(m_isFb);
	cmdb->bindPipeline(m_blitPpline);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->bindResourceGroup(m_blitRcGroup, 0, nullptr);

	m_r->drawQuad(cmdb);
}

//==============================================================================
void Refl::writeUniforms(CommandBufferPtr cmdb)
{
	const FrustumComponent& frc = m_r->getActiveFrustumComponent();

	if(m_uniformsUpdateTimestamp
			< m_r->getProjectionParametersUpdateTimestamp()
		|| m_uniformsUpdateTimestamp < frc.getTimestamp()
		|| m_uniformsUpdateTimestamp == 1)
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

