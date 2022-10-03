// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Scale.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

DownscaleBlur::~DownscaleBlur()
{
	m_fbDescrs.destroy(getMemoryPool());
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
										  kDownscaleBurDownTo)
				  - 1;

	const UVec2 rez = m_r->getPostProcessResolution() / 2;
	ANKI_R_LOGV("Initializing downscale pyramid. Resolution %ux%u, mip count %u", rez.x(), rez.y(), m_passCount);

	const Bool preferCompute = getConfig().getRPreferCompute();

	// Create the miped texture
	TextureInitInfo texinit =
		m_r->create2DRenderTargetDescription(rez.x(), rez.y(), m_r->getHdrFormat(), "DownscaleBlur");
	texinit.m_usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute;
	if(preferCompute)
	{
		texinit.m_usage |= TextureUsageBit::kSampledCompute | TextureUsageBit::kImageComputeWrite;
	}
	else
	{
		texinit.m_usage |= TextureUsageBit::kFramebufferWrite;
	}
	texinit.m_mipmapCount = U8(m_passCount);
	m_rtTex = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledCompute);

	// FB descr
	if(!preferCompute)
	{
		m_fbDescrs.create(getMemoryPool(), m_passCount);
		for(U32 pass = 0; pass < m_passCount; ++pass)
		{
			m_fbDescrs[pass].m_colorAttachmentCount = 1;
			m_fbDescrs[pass].m_colorAttachments[0].m_surface.m_level = pass;
			m_fbDescrs[pass].bake();
		}
	}

	// Shader programs
	ANKI_CHECK(getResourceManager().loadResource((preferCompute) ? "ShaderBinaries/DownscaleBlurCompute.ankiprogbin"
																 : "ShaderBinaries/DownscaleBlurRaster.ankiprogbin",
												 m_prog));
	const ShaderProgramResourceVariant* variant = nullptr;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	return Error::kNone;
}

void DownscaleBlur::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.importRenderTarget(m_rtTex, TextureUsageBit::kSampledCompute);
}

void DownscaleBlur::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create passes
	static constexpr Array<CString, 8> passNames = {"DownBlur #0",  "Down/Blur #1", "Down/Blur #2", "Down/Blur #3",
													"Down/Blur #4", "Down/Blur #5", "Down/Blur #6", "Down/Blur #7"};
	const RenderTargetHandle inRt =
		(m_r->getScale().hasUpscaledHdrRt()) ? m_r->getScale().getUpscaledHdrRt() : m_r->getScale().getTonemappedRt();
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

				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kImageComputeWrite, renderSubresource);
				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kSampledCompute, sampleSubresource);
			}
			else
			{
				TextureSubresourceInfo renderSubresource;

				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kImageComputeWrite, renderSubresource);
				pass.newTextureDependency(inRt, TextureUsageBit::kSampledCompute);
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

				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite, renderSubresource);
				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kSampledFragment, sampleSubresource);
			}
			else
			{
				TextureSubresourceInfo renderSubresource;

				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite, renderSubresource);
				pass.newTextureDependency(inRt, TextureUsageBit::kSampledFragment);
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
		const RenderTargetHandle inRt = (m_r->getScale().hasUpscaledHdrRt()) ? m_r->getScale().getUpscaledHdrRt()
																			 : m_r->getScale().getTonemappedRt();
		rgraphCtx.bindColorTexture(0, 1, inRt);
	}

	rgraphCtx.bindImage(0, 2, m_r->getTonemapping().getRt());

	const Bool revertTonemap = passIdx == 0 && !m_r->getScale().hasUpscaledHdrRt();
	const UVec4 fbSize(vpWidth, vpHeight, revertTonemap, 0);
	cmdb->setPushConstants(&fbSize, sizeof(fbSize));

	if(getConfig().getRPreferCompute())
	{
		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx;
		rgraphCtx.bindImage(0, 3, m_runCtx.m_rt, sampleSubresource);

		dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], vpWidth, vpHeight);
	}
	else
	{
		cmdb->setViewport(0, 0, vpWidth, vpHeight);

		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
