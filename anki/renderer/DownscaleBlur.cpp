// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/TemporalAA.h>

namespace anki
{

DownscaleBlur::~DownscaleBlur()
{
	m_fbDescrs.destroy(getAllocator());
}

Error DownscaleBlur::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize downscale blur");
	}

	return err;
}

Error DownscaleBlur::initInternal(const ConfigSet&)
{
	m_passCount = computeMaxMipmapCount2d(m_r->getWidth(), m_r->getHeight(), DOWNSCALE_BLUR_DOWN_TO) - 1;
	ANKI_R_LOGI("Initializing dowscale blur (passCount: %u)", m_passCount);

	// Create the miped texture
	TextureInitInfo texinit = m_r->create2DRenderTargetDescription(
		m_r->getWidth() / 2, m_r->getHeight() / 2, LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "DownscaleBlur");
	texinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE;
	if(m_useCompute)
	{
		texinit.m_usage |= TextureUsageBit::SAMPLED_COMPUTE | TextureUsageBit::IMAGE_COMPUTE_WRITE;
	}
	else
	{
		texinit.m_usage |= TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	}
	texinit.m_mipmapCount = U8(m_passCount);
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_COMPUTE;
	m_rtTex = m_r->createAndClearRenderTarget(texinit);

	// FB descr
	if(!m_useCompute)
	{
		m_fbDescrs.create(getAllocator(), m_passCount);
		for(U32 pass = 0; pass < m_passCount; ++pass)
		{
			m_fbDescrs[pass].m_colorAttachmentCount = 1;
			m_fbDescrs[pass].m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
			m_fbDescrs[pass].m_colorAttachments[0].m_surface.m_level = pass;
			m_fbDescrs[pass].bake();
		}
	}

	// Shader programs
	const ShaderProgramResourceVariant* variant = nullptr;
	if(m_useCompute)
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/DownscaleBlurCompute.ankiprog", m_prog));
		m_prog->getOrCreateVariant(variant);
		m_workgroupSize[0] = variant->getWorkgroupSizes()[0];
		m_workgroupSize[1] = variant->getWorkgroupSizes()[1];
		ANKI_ASSERT(variant->getWorkgroupSizes()[2] == 1);
	}
	else
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/DownscaleBlur.ankiprog", m_prog));
		m_prog->getOrCreateVariant(variant);
	}
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void DownscaleBlur::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.importRenderTarget(m_rtTex, TextureUsageBit::SAMPLED_COMPUTE);
}

void DownscaleBlur::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_crntPassIdx = 0;

	// Create passes
	static const Array<CString, 8> passNames = {"DownBlur #0",  "Down/Blur #1", "Down/Blur #2", "Down/Blur #3",
												"Down/Blur #4", "Down/Blur #5", "Down/Blur #6", "Down/Blur #7"};
	if(m_useCompute)
	{
		for(U32 i = 0; i < m_passCount; ++i)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passNames[i]);
			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<DownscaleBlur*>(rgraphCtx.m_userData)->run(rgraphCtx);
				},
				this, 0);

			if(i > 0)
			{
				TextureSubresourceInfo sampleSubresource;
				TextureSubresourceInfo renderSubresource;

				sampleSubresource.m_firstMipmap = i - 1;
				renderSubresource.m_firstMipmap = i;

				pass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE, renderSubresource});
				pass.newDependency({m_runCtx.m_rt, TextureUsageBit::SAMPLED_COMPUTE, sampleSubresource});
			}
			else
			{
				TextureSubresourceInfo renderSubresource;

				pass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE, renderSubresource});
				pass.newDependency({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
			}
		}
	}
	else
	{
		for(U32 i = 0; i < m_passCount; ++i)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[i]);
			pass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<DownscaleBlur*>(rgraphCtx.m_userData)->run(rgraphCtx);
				},
				this, 0);
			pass.setFramebufferInfo(m_fbDescrs[i], {m_runCtx.m_rt}, {});

			if(i > 0)
			{
				TextureSubresourceInfo sampleSubresource;
				TextureSubresourceInfo renderSubresource;

				sampleSubresource.m_firstMipmap = i - 1;
				renderSubresource.m_firstMipmap = i;

				pass.newDependency({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, renderSubresource});
				pass.newDependency({m_runCtx.m_rt, TextureUsageBit::SAMPLED_FRAGMENT, sampleSubresource});
			}
			else
			{
				TextureSubresourceInfo renderSubresource;

				pass.newDependency({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, renderSubresource});
				pass.newDependency({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
			}
		}
	}
}

void DownscaleBlur::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	const U32 passIdx = m_runCtx.m_crntPassIdx++;
	const U32 vpWidth = m_rtTex->getWidth() >> passIdx;
	const U32 vpHeight = m_rtTex->getHeight() >> passIdx;

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);

	if(passIdx > 0)
	{
		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx - 1;
		rgraphCtx.bindTexture(0, 1, m_runCtx.m_rt, sampleSubresource);
	}
	else
	{
		rgraphCtx.bindColorTexture(0, 1, m_r->getTemporalAA().getRt());
	}

	if(m_useCompute)
	{
		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx;
		rgraphCtx.bindImage(0, 2, m_runCtx.m_rt, sampleSubresource);

		UVec4 fbSize(vpWidth, vpHeight, 0, 0);
		cmdb->setPushConstants(&fbSize, sizeof(fbSize));
	}

	if(m_useCompute)
	{
		dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], vpWidth, vpHeight);
	}
	else
	{
		cmdb->setViewport(0, 0, vpWidth, vpHeight);
		drawQuad(cmdb);
	}
}

} // end namespace anki
