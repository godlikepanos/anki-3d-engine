// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

DownscaleBlur::~DownscaleBlur()
{
	m_fbDescrs.destroy(getAllocator());
}

Error DownscaleBlur::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize downscale blur");
	}

	return err;
}

Error DownscaleBlur::initInternal()
{
	m_passCount = computeMaxMipmapCount2d(m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y(),
										  DOWNSCALE_BLUR_DOWN_TO)
				  - 1;

	const UVec2 rez = m_r->getPostProcessResolution() / 2;
	ANKI_R_LOGV("Initializing downscale pyramid. Resolution %ux%u, mip count %u", rez.x(), rez.y(), m_passCount);

	const Bool preferCompute = getConfig().getRPreferCompute();

	// Create the miped texture
	TextureInitInfo texinit =
		m_r->create2DRenderTargetDescription(rez.x(), rez.y(), m_r->getHdrFormat(), "DownscaleBlur");
	texinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE;
	if(preferCompute)
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
	if(!preferCompute)
	{
		m_fbDescrs.create(getAllocator(), m_passCount);
		for(U32 pass = 0; pass < m_passCount; ++pass)
		{
			m_fbDescrs[pass].m_colorAttachmentCount = 1;
			m_fbDescrs[pass].m_colorAttachments[0].m_surface.m_level = pass;
			m_fbDescrs[pass].bake();
		}
	}

	// Shader programs
	ANKI_CHECK(getResourceManager().loadResource(
		(preferCompute) ? "Shaders/DownscaleBlurCompute.ankiprog" : "Shaders/DownscaleBlurRaster.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant = nullptr;
	m_prog->getOrCreateVariant(variant);
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

	// Create passes
	static const Array<CString, 8> passNames = {"DownBlur #0",  "Down/Blur #1", "Down/Blur #2", "Down/Blur #3",
												"Down/Blur #4", "Down/Blur #5", "Down/Blur #6", "Down/Blur #7"};
	if(getConfig().getRPreferCompute())
	{
		for(U32 i = 0; i < m_passCount; ++i)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passNames[i]);
			pass.setWork([this, i](RenderPassWorkContext& rgraphCtx) {
				run(i, rgraphCtx);
			});

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
				pass.newDependency({m_r->getTemporalAA().getHdrRt(), TextureUsageBit::SAMPLED_COMPUTE});
			}
		}
	}
	else
	{
		for(U32 i = 0; i < m_passCount; ++i)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[i]);
			pass.setWork([this, i](RenderPassWorkContext& rgraphCtx) {
				run(i, rgraphCtx);
			});
			pass.setFramebufferInfo(m_fbDescrs[i], {m_runCtx.m_rt});

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
				pass.newDependency({m_r->getTemporalAA().getHdrRt(), TextureUsageBit::SAMPLED_FRAGMENT});
			}
		}
	}
}

void DownscaleBlur::run(U32 passIdx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

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
		rgraphCtx.bindColorTexture(0, 1, m_r->getTemporalAA().getHdrRt());
	}

	if(getConfig().getRPreferCompute())
	{
		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx;
		rgraphCtx.bindImage(0, 2, m_runCtx.m_rt, sampleSubresource);

		UVec4 fbSize(vpWidth, vpHeight, 0, 0);
		cmdb->setPushConstants(&fbSize, sizeof(fbSize));

		dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], vpWidth, vpHeight);
	}
	else
	{
		cmdb->setViewport(0, 0, vpWidth, vpHeight);

		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	}
}

} // end namespace anki
