// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Pps.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Sslf.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Sslr.h>
#include <anki/renderer/Tm.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Dbg.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/FrustumComponent.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================
struct Uniforms
{
	Vec4 m_nearFarPad2;
	Vec4 m_fogColorFogFactor;
};

//==============================================================================
// Pps                                                                         =
//==============================================================================

//==============================================================================
const PixelFormat Pps::RT_PIXEL_FORMAT(
	ComponentFormat::R8G8B8, TransformFormat::UNORM);

//==============================================================================
Pps::Pps(Renderer* r)
	: RenderingPass(r)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
Error Pps::initInternal(const ConfigSet& config)
{
	m_enabled = config.getNumber("pps.enabled");
	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	ANKI_ASSERT("Initializing PPS");

	m_ssao.reset(getAllocator().newInstance<Ssao>(m_r));
	ANKI_CHECK(m_ssao->init(config));

	m_sslr.reset(getAllocator().newInstance<Sslr>(m_r));
	ANKI_CHECK(m_sslr->init(config));

	m_tm.reset(getAllocator().newInstance<Tm>(m_r));
	ANKI_CHECK(m_tm->create(config));

	m_bloom.reset(getAllocator().newInstance<Bloom>(m_r));
	ANKI_CHECK(m_bloom->init(config));

	m_sslf.reset(getAllocator().newInstance<Sslf>(m_r));
	ANKI_CHECK(m_sslf->init(config));

	// FBO
	m_r->createRenderTarget(
		m_r->getWidth(), m_r->getHeight(),
		RT_PIXEL_FORMAT, 1, SamplingFilter::LINEAR, 1, m_rt);

	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// SProg
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define SSAO_ENABLED %u\n"
		"#define BLOOM_ENABLED %u\n"
		"#define SSLF_ENABLED %u\n"
		"#define SHARPEN_ENABLED %u\n"
		"#define GAMMA_CORRECTION_ENABLED %u\n"
		"#define FBO_WIDTH %u\n"
		"#define FBO_HEIGHT %u\n",
		m_ssao->getEnabled(),
		m_bloom->getEnabled(),
		m_sslf->getEnabled(),
		U(config.getNumber("pps.sharpen")),
		U(config.getNumber("pps.gammaCorrection")),
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_frag, "shaders/Pps.frag.glsl", pps.toCString(), "r_"));

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = RT_PIXEL_FORMAT;
	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorState, m_ppline);

	// LUT
	ANKI_CHECK(loadColorGradingTexture("engine_data/default_lut.ankitex"));

	// Uniforms
	m_uniformsBuff = getGrManager().newInstance<Buffer>(sizeof(Uniforms),
		BufferUsageBit::STORAGE, BufferAccessBit::CLIENT_WRITE);

	// RC goup
	ResourceGroupInitializer rcInit;
	rcInit.m_textures[0].m_texture = m_r->getIs().getRt();

	if(m_ssao->getEnabled())
	{
		rcInit.m_textures[1].m_texture = m_ssao->getRt();
	}

	if(m_bloom->getEnabled())
	{
		rcInit.m_textures[2].m_texture = m_bloom->getRt();
	}

	rcInit.m_textures[3].m_texture = m_lut->getGrTexture();

	if(m_sslf->getEnabled())
	{
		rcInit.m_textures[4].m_texture = m_sslf->getRt();
	}

	rcInit.m_textures[5].m_texture = m_r->getMs().getDepthRt();

	rcInit.m_storageBuffers[0].m_buffer = m_tm->getAverageLuminanceBuffer();
	rcInit.m_storageBuffers[1].m_buffer = m_uniformsBuff;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Pps::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init PPS");
	}

	return err;
}
//==============================================================================
Error Pps::loadColorGradingTexture(CString filename)
{
	ANKI_CHECK(getResourceManager().loadResource(filename, m_lut));
	return ErrorCode::NONE;
}

//==============================================================================
void Pps::run(CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(m_enabled);

	// First SSAO because it depends on MS where HDR depends on IS
	if(m_ssao->getEnabled())
	{
		m_ssao->run(cmdb);
	}

	// Then SSLR because HDR depends on it
	if(m_sslr->getEnabled())
	{
		m_sslr->run(cmdb);
	}

	m_r->getIs().generateMipmaps(cmdb);
	m_tm->run(cmdb);

	if(m_bloom->getEnabled())
	{
		m_bloom->run(cmdb);
	}

	if(m_sslf->getEnabled())
	{
		m_sslf->run(cmdb);
	}

	FramebufferPtr fb = m_fb;
	U32 width = m_r->getWidth();
	U32 height = m_r->getHeight();

	Bool isLastStage = !m_r->getDbg().getEnabled();
	if(isLastStage)
	{
		m_r->getOutputFramebuffer(fb, width, height);
	}

	cmdb->bindFramebuffer(fb);
	cmdb->setViewport(0, 0, width, height);
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rcGroup, 0, nullptr);

	if(m_uniformsDirty)
	{
		m_uniformsDirty = false;
		DynamicBufferToken token;
		Uniforms* unis = static_cast<Uniforms*>(
			getGrManager().allocateFrameHostVisibleMemory(
			sizeof(*unis), BufferUsage::TRANSFER, token));
		cmdb->writeBuffer(m_uniformsBuff, 0, token);
		unis->m_fogColorFogFactor = Vec4(m_fogColor, m_fogFactor);

		const FrustumComponent& frc = m_r->getActiveFrustumComponent();
		unis->m_nearFarPad2 = Vec4(frc.getFrustum().getNear(),
			frc.getFrustum().getFar(), 0.0, 0.0);
	}

	m_r->drawQuad(cmdb);
}

} // end namespace anki
