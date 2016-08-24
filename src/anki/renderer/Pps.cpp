// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

//==============================================================================
const PixelFormat Pps::RT_PIXEL_FORMAT(
	ComponentFormat::R8G8B8, TransformFormat::UNORM);

//==============================================================================
Pps::Pps(Renderer* r)
	: RenderingPass(r)
{
}

//==============================================================================
Pps::~Pps()
{
}

//==============================================================================
Error Pps::initInternal(const ConfigSet& config)
{
	ANKI_ASSERT("Initializing PPS");

	ANKI_CHECK(loadColorGradingTexture("engine_data/DefaultLut.ankitex"));
	m_sharpenEnabled = config.getNumber("pps.sharpen");

	if(!m_r->getDrawToDefaultFramebuffer())
	{
		m_r->createRenderTarget(m_r->getWidth(),
			m_r->getHeight(),
			RT_PIXEL_FORMAT,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
				| TextureUsageBit::SAMPLED_FRAGMENT,
			SamplingFilter::LINEAR,
			1,
			m_rt);

		FramebufferInitInfo fbInit;
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_rt;
		fbInit.m_colorAttachments[0].m_loadOperation =
			AttachmentLoadOperation::DONT_CARE;
		fbInit.m_colorAttachments[0].m_usageInsideRenderPass =
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
		m_fb = getGrManager().newInstance<Framebuffer>(fbInit);
	}

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
	m_lut.reset(nullptr);
	ANKI_CHECK(getResourceManager().loadResource(filename, m_lut));
	ANKI_ASSERT(m_lut->getWidth() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getHeight() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getDepth() == LUT_SIZE);

	m_lutDirty = true;
	return ErrorCode::NONE;
}

//==============================================================================
Error Pps::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Get the drawing parameters
	Bool drawToDefaultFb = ctx.m_outFb.isCreated();
	Bool dbgEnabled = m_r->getDbg().getEnabled();

	// Get or create the ppline
	PipelinePtr& ppline = m_ppline[drawToDefaultFb][dbgEnabled];

	if(!ppline)
	{
		// Need to create it

		ShaderResourcePtr& frag = m_frag[dbgEnabled];
		if(!frag)
		{
			StringAuto pps(ctx.m_tempAllocator);

			pps.sprintf("#define BLOOM_ENABLED %u\n"
						"#define SHARPEN_ENABLED %u\n"
						"#define FBO_WIDTH %u\n"
						"#define FBO_HEIGHT %u\n"
						"#define LUT_SIZE %u.0\n"
						"#define DBG_ENABLED %u\n",
				true,
				m_sharpenEnabled,
				m_r->getWidth(),
				m_r->getHeight(),
				LUT_SIZE,
				dbgEnabled);

			ANKI_CHECK(getResourceManager().loadResourceToCache(
				frag, "shaders/Pps.frag.glsl", pps.toCString(), "r_"));
		}

		ColorStateInfo colorState;
		colorState.m_attachmentCount = 1;
		if(!drawToDefaultFb)
		{
			colorState.m_attachments[0].m_format = RT_PIXEL_FORMAT;
		}
		m_r->createDrawQuadPipeline(frag->getGrShader(), colorState, ppline);
	}

	// Get or create the resource group
	ResourceGroupPtr& rsrc = m_rcGroup[dbgEnabled];
	if(!rsrc || m_lutDirty)
	{
		ResourceGroupInitInfo rcInit;
		rcInit.m_textures[0].m_texture = m_r->getIs().getRt();
		rcInit.m_textures[1].m_texture = m_r->getBloom().getFinalRt();
		rcInit.m_textures[2].m_texture = m_lut->getGrTexture();
		if(dbgEnabled)
		{
			rcInit.m_textures[3].m_texture = m_r->getDbg().getRt();
		}

		rcInit.m_storageBuffers[0].m_buffer =
			m_r->getTm().getAverageLuminanceBuffer();
		rcInit.m_storageBuffers[0].m_usage = BufferUsageBit::STORAGE_FRAGMENT;

		rsrc = getGrManager().newInstance<ResourceGroup>(rcInit);

		m_lutDirty = false;
	}

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
	cmdb->bindPipeline(ppline);
	cmdb->bindResourceGroup(rsrc, 0, nullptr);
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
