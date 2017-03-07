// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Pps.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Sslf.h>
#include <anki/renderer/Tm.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/Smaa.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

const PixelFormat Pps::RT_PIXEL_FORMAT(ComponentFormat::R8G8B8, TransformFormat::UNORM);

Pps::Pps(Renderer* r)
	: RenderingPass(r)
{
}

Pps::~Pps()
{
}

Error Pps::initInternal(const ConfigSet& config)
{
	ANKI_ASSERT("Initializing PPS");

	ANKI_CHECK(loadColorGradingTexture("engine_data/DefaultLut.ankitex"));
	m_sharpenEnabled = config.getNumber("pps.sharpen");

	if(!m_r->getDrawToDefaultFramebuffer())
	{
		m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
			m_r->getHeight(),
			RT_PIXEL_FORMAT,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
			SamplingFilter::LINEAR));

		FramebufferInitInfo fbInit;
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_rt;
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fb = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_blueNoise));

	return ErrorCode::NONE;
}

Error Pps::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to init PPS");
	}

	return err;
}

Error Pps::loadColorGradingTexture(CString filename)
{
	m_lut.reset(nullptr);
	ANKI_CHECK(getResourceManager().loadResource(filename, m_lut));
	ANKI_ASSERT(m_lut->getWidth() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getHeight() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getDepth() == LUT_SIZE);

	return ErrorCode::NONE;
}

Error Pps::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Get the drawing parameters
	Bool drawToDefaultFb = ctx.m_outFb.isCreated();
	Bool dbgEnabled = m_r->getDbg().getEnabled();

	// Get or create the ppline
	ShaderProgramPtr& prog = m_prog[drawToDefaultFb][dbgEnabled];

	if(!prog)
	{
		// Need to create it

		ShaderResourcePtr& frag = m_frag[drawToDefaultFb][dbgEnabled];
		if(!frag)
		{
			ANKI_CHECK(m_r->createShaderf("shaders/Pps.frag.glsl",
				frag,
				"#define BLOOM_ENABLED %u\n"
				"#define SHARPEN_ENABLED %u\n"
				"#define FBO_WIDTH %u\n"
				"#define FBO_HEIGHT %u\n"
				"#define LUT_SIZE %u.0\n"
				"#define DBG_ENABLED %u\n"
				"#define DRAW_TO_DEFAULT %u\n"
				"#define SMAA_ENABLED 1\n"
				"#define SMAA_RT_METRICS vec4(%f, %f, %f, %f)\n"
				"#define SMAA_PRESET_%s\n"
				"#define FB_SIZE vec2(float(%u), float(%u))\n",
				true,
				m_sharpenEnabled,
				m_r->getWidth(),
				m_r->getHeight(),
				LUT_SIZE,
				dbgEnabled,
				drawToDefaultFb,
				1.0 / m_r->getWidth(),
				1.0 / m_r->getHeight(),
				F32(m_r->getWidth()),
				F32(m_r->getHeight()),
				&m_r->getSmaa().m_qualityPerset[0],
				m_r->getWidth(),
				m_r->getHeight()));
		}

		if(!m_vert)
		{
			ANKI_CHECK(m_r->createShaderf("shaders/Pps.vert.glsl",
				m_vert,
				"#define SMAA_ENABLED 1\n"
				"#define SMAA_RT_METRICS vec4(%f, %f, %f, %f)\n"
				"#define SMAA_PRESET_%s\n",
				1.0 / m_r->getWidth(),
				1.0 / m_r->getHeight(),
				F32(m_r->getWidth()),
				F32(m_r->getHeight()),
				&m_r->getSmaa().m_qualityPerset[0]));
		}

		prog = getGrManager().newInstance<ShaderProgram>(m_vert->getGrShader(), frag->getGrShader());
	}

	// Bind stuff
	cmdb->bindTexture(0, 0, m_r->getIs().getRt());
	cmdb->bindTexture(0, 1, m_r->getBloom().m_upscale.m_rt);
	cmdb->bindTexture(0, 2, m_lut->getGrTexture());
	cmdb->bindTexture(0, 3, m_blueNoise->getGrTexture());
	cmdb->bindTexture(0, 4, m_r->getSmaa().m_weights.m_rt);
	if(dbgEnabled)
	{
		cmdb->bindTexture(0, 5, m_r->getDbg().getRt());
	}

	cmdb->bindStorageBuffer(0, 0, m_r->getTm().m_luminanceBuff, 0, MAX_PTR_SIZE);

	Vec4* uniforms = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	uniforms->x() = F32(m_r->getFrameCount() % m_blueNoise->getLayerCount());

	// Get or create FB
	FramebufferPtr* fb = nullptr;
	U width, height;
	if(drawToDefaultFb)
	{
		fb = &ctx.m_outFb;
		width = ctx.m_outFbWidth;
		height = ctx.m_outFbHeight;
	}
	else
	{
		width = m_r->getWidth();
		height = m_r->getHeight();
		fb = &m_fb;
	}

	cmdb->beginRenderPass(*fb);
	cmdb->setViewport(0, 0, width, height);
	cmdb->bindShaderProgram(prog);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	if(!drawToDefaultFb)
	{
		cmdb->setTextureSurfaceBarrier(m_rt,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
	}

	return ErrorCode::NONE;
}

} // end namespace anki
