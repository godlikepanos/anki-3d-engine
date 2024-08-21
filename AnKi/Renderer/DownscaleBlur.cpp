// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

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
	m_passCount =
		computeMaxMipmapCount2d(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), kDownscaleBurDownTo) - 1;

	const UVec2 rez = getRenderer().getInternalResolution() / 2;
	ANKI_R_LOGV("Initializing downscale pyramid. Resolution %ux%u, mip count %u", rez.x(), rez.y(), m_passCount);

	const Bool preferCompute = g_preferComputeCVar.get();

	// Create the miped texture
	TextureInitInfo texinit = getRenderer().create2DRenderTargetDescription(rez.x(), rez.y(), getRenderer().getHdrFormat(), "DownscaleBlur");
	texinit.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kSrvCompute;
	if(preferCompute)
	{
		texinit.m_usage |= TextureUsageBit::kUavCompute;
	}
	else
	{
		texinit.m_usage |= TextureUsageBit::kRtvDsvWrite;
	}
	texinit.m_mipmapCount = U8(m_passCount);
	m_rtTex = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvCompute);

	// Shader programs
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/DownscaleBlur.ankiprogbin", m_prog, m_grProg));

	return Error::kNone;
}

void DownscaleBlur::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.importRenderTarget(m_rtTex.get(), TextureUsageBit::kSrvCompute);
}

void DownscaleBlur::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(DownscaleBlur);
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create passes
	static constexpr Array<CString, 8> passNames = {"Down/Blur #0", "Down/Blur #1", "Down/Blur #2", "Down/Blur #3",
													"Down/Blur #4", "Down/Blur #5", "Down/Blur #6", "Down/Blur #7"};
	const RenderTargetHandle inRt = getRenderer().getLightShading().getRt();

	for(U32 i = 0; i < m_passCount; ++i)
	{
		RenderPassBase* ppass;
		if(g_preferComputeCVar.get())
		{
			ppass = &rgraph.newNonGraphicsRenderPass(passNames[i]);
		}
		else
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(passNames[i]);

			GraphicsRenderPassTargetDesc rtInf(m_runCtx.m_rt);
			rtInf.m_subresource.m_mipmap = i;
			pass.setRenderpassInfo({rtInf});

			ppass = &pass;
		}

		const TextureUsageBit readUsage = (g_preferComputeCVar.get()) ? TextureUsageBit::kSrvCompute : TextureUsageBit::kSrvPixel;
		const TextureUsageBit writeUsage = (g_preferComputeCVar.get()) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;

		if(i > 0)
		{
			const TextureSubresourceDesc sampleSubresource = TextureSubresourceDesc::surface(i - 1, 0, 0);
			const TextureSubresourceDesc renderSubresource = TextureSubresourceDesc::surface(i, 0, 0);

			ppass->newTextureDependency(m_runCtx.m_rt, writeUsage, renderSubresource);
			ppass->newTextureDependency(m_runCtx.m_rt, readUsage, sampleSubresource);
		}
		else
		{
			ppass->newTextureDependency(m_runCtx.m_rt, writeUsage, TextureSubresourceDesc::firstSurface());
			ppass->newTextureDependency(inRt, readUsage);
		}

		ppass->setWork([this, i](RenderPassWorkContext& rgraphCtx) {
			run(i, rgraphCtx);
		});
	}
}

void DownscaleBlur::run(U32 passIdx, RenderPassWorkContext& rgraphCtx)
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram(m_grProg.get());

	const U32 vpWidth = m_rtTex->getWidth() >> passIdx;
	const U32 vpHeight = m_rtTex->getHeight() >> passIdx;

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

	if(passIdx > 0)
	{
		rgraphCtx.bindSrv(0, 0, m_runCtx.m_rt, TextureSubresourceDesc::surface(passIdx - 1, 0, 0));
	}
	else
	{
		rgraphCtx.bindSrv(0, 0, getRenderer().getLightShading().getRt());
	}

	rgraphCtx.bindUav(0, 0, getRenderer().getTonemapping().getRt());

	if(g_preferComputeCVar.get())
	{
		const Vec4 fbSize(F32(vpWidth), F32(vpHeight), 0.0f, 0.0f);
		cmdb.setFastConstants(&fbSize, sizeof(fbSize));

		rgraphCtx.bindUav(1, 0, m_runCtx.m_rt, TextureSubresourceDesc::surface(passIdx, 0, 0));

		dispatchPPCompute(cmdb, 8, 8, vpWidth, vpHeight);
	}
	else
	{
		cmdb.setViewport(0, 0, vpWidth, vpHeight);

		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
