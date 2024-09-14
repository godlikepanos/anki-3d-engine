// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Ssr.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Renderer/Bloom2.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/DepthDownscale.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-function"
#	pragma GCC diagnostic ignored "-Wignored-qualifiers"
#elif ANKI_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable : 4505)
#endif
#define A_CPU
#include <ThirdParty/FidelityFX/ffx_a.h>
#include <ThirdParty/FidelityFX/ffx_spd.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#elif ANKI_COMPILER_MSVC
#	pragma warning(pop)
#endif

namespace anki {

Error Ssr::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize SSR");
	}
	return err;
}

Error Ssr::initInternal()
{
	const UVec2 rez = (g_ssrQuarterResolutionCVar) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

	ANKI_R_LOGV("Initializing SSR. Resolution %ux%u", rez.x(), rez.y());

	TextureUsageBit mipTexUsage = TextureUsageBit::kAllSrv;
	if(g_preferComputeCVar)
	{
		mipTexUsage |= TextureUsageBit::kUavCompute;
	}
	else
	{
		mipTexUsage |= TextureUsageBit::kRtvDsvWrite;
	}

	m_ssrRtDescr = getRenderer().create2DRenderTargetDescription(rez.x(), rez.y(), Format::kR16G16B16A16_Sfloat, "SSR");
	m_ssrRtDescr.bake();

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssr.ankiprogbin", {}, m_prog, m_ssrGrProg, "Ssr"));

	return Error::kNone;
}

void Ssr::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Ssr);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar;

	m_runCtx.m_ssrRt = rgraph.newRenderTarget(m_ssrRtDescr);

	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	RenderPassBase* ppass;
	if(preferCompute)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("SSR");
		ppass = &pass;

		readUsage = TextureUsageBit::kSrvCompute;
		writeUsage = TextureUsageBit::kUavCompute;
	}
	else
	{
		// TODO
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("SSR");

		pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_ssrRt)});

		ppass = &pass;

		readUsage = TextureUsageBit::kSrvPixel;
		writeUsage = TextureUsageBit::kRtvDsvWrite;
	}

	ppass->newTextureDependency(getRenderer().getBloom2().getPyramidRt(), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(1), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
	ppass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage);
	ppass->newTextureDependency(m_runCtx.m_ssrRt, writeUsage);

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		const UVec2 rez = getRenderer().getInternalResolution() / ((g_ssrQuarterResolutionCVar) ? 2 : 1);

		cmdb.bindShaderProgram(m_ssrGrProg.get());

		SsrConstants consts = {};
		consts.m_viewportSizef = Vec2(rez);
		consts.m_frameCount = getRenderer().getFrameCount() % kMaxU32;
		consts.m_maxIterations = g_ssrMaxIterationsCVar;
		consts.m_roughnessCutoff = g_ssrRoughnessCutoffCVar;
		consts.m_stepIncrement = g_ssrStepIncrementCVar;
		consts.m_projMat00_11_22_23 = Vec4(ctx.m_matrices.m_projection(0, 0), ctx.m_matrices.m_projection(1, 1), ctx.m_matrices.m_projection(2, 2),
										   ctx.m_matrices.m_projection(2, 3));
		consts.m_unprojectionParameters = ctx.m_matrices.m_unprojectionParameters;
		consts.m_prevViewProjMatMulInvViewProjMat = ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
		consts.m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
		*allocateAndBindConstants<SsrConstants>(cmdb, 0, 0) = consts;

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(2));
		rgraphCtx.bindSrv(2, 0, getRenderer().getDepthDownscale().getRt());
		rgraphCtx.bindSrv(3, 0, getRenderer().getBloom2().getPyramidRt());

		if(g_preferComputeCVar)
		{
			rgraphCtx.bindUav(0, 0, m_runCtx.m_ssrRt);
			dispatchPPCompute(cmdb, 8, 8, rez.x(), rez.y());
		}
		else
		{
			cmdb.setViewport(0, 0, rez.x(), rez.y());
			drawQuad(cmdb);
		}
	});
}

} // end namespace anki
