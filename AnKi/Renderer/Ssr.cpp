// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Ssr.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Renderer/DownscaleBlur.h>
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

static NumericCVar<U32> g_ssrStepIncrementCVar(CVarSubsystem::kRenderer, "SsrStepIncrement", 32, 1, 256, "The number of steps for each loop");
static NumericCVar<U32> g_ssrMaxIterationsCVar(CVarSubsystem::kRenderer, "SsrMaxIterations", 64, 1, 256, "Max SSR raymarching loop iterations");
static NumericCVar<F32> g_ssrRoughnessCutoffCVar(CVarSubsystem::kRenderer, "SsrRoughnessCutoff", (ANKI_PLATFORM_MOBILE) ? 0.7f : 1.0f, 0.0f, 1.0f,
												 "Materials with roughness higher that this value will fallback to probe reflections");
static BoolCVar g_ssrQuarterResolution(CVarSubsystem::kRenderer, "SsrQuarterResolution", ANKI_PLATFORM_MOBILE);

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
	const UVec2 rez = (g_ssrQuarterResolution.get()) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

	ANKI_R_LOGV("Initializing SSR. Resolution %ux%u", rez.x(), rez.y());

	TextureUsageBit mipTexUsage = TextureUsageBit::kAllSampled;
	if(g_preferComputeCVar.get())
	{
		mipTexUsage |= TextureUsageBit::kStorageComputeWrite;
	}
	else
	{
		mipTexUsage |= TextureUsageBit::kFramebufferWrite;
	}

	m_ssrRtDescr = getRenderer().create2DRenderTargetDescription(rez.x(), rez.y(), Format::kR16G16B16A16_Sfloat, "SSR");
	m_ssrRtDescr.bake();

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssr.ankiprogbin", {}, m_prog, m_ssrGrProg, "Ssr"));

	return Error::kNone;
}

void Ssr::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Ssr);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	m_runCtx.m_ssrRt = rgraph.newRenderTarget(m_ssrRtDescr);

	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	RenderPassDescriptionBase* ppass;
	if(preferCompute)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSR");
		ppass = &pass;

		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kStorageComputeWrite;
	}
	else
	{
		// TODO
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSR");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_ssrRt}, {});

		ppass = &pass;

		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
	}

	ppass->newTextureDependency(getRenderer().getDownscaleBlur().getRt(), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(1), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
	ppass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage);
	ppass->newTextureDependency(m_runCtx.m_ssrRt, writeUsage);

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		const UVec2 rez = getRenderer().getInternalResolution() / ((g_ssrQuarterResolution.get()) ? 2 : 1);

		cmdb.bindShaderProgram(m_ssrGrProg.get());

		SsrUniforms consts = {};
		consts.m_viewportSizef = Vec2(rez);
		consts.m_frameCount = getRenderer().getFrameCount() % kMaxU32;
		consts.m_maxIterations = g_ssrMaxIterationsCVar.get();
		consts.m_roughnessCutoff = g_ssrRoughnessCutoffCVar.get();
		consts.m_stepIncrement = g_ssrStepIncrementCVar.get();
		consts.m_projMat00_11_22_23 = Vec4(ctx.m_matrices.m_projection(0, 0), ctx.m_matrices.m_projection(1, 1), ctx.m_matrices.m_projection(2, 2),
										   ctx.m_matrices.m_projection(2, 3));
		consts.m_unprojectionParameters = ctx.m_matrices.m_unprojectionParameters;
		consts.m_prevViewProjMatMulInvViewProjMat = ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
		consts.m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
		*allocateAndBindConstants<SsrUniforms>(cmdb, 0, 0) = consts;

		cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindColorTexture(0, 2, getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 3, getRenderer().getGBuffer().getColorRt(2));
		rgraphCtx.bindColorTexture(0, 4, getRenderer().getDepthDownscale().getRt());
		rgraphCtx.bindColorTexture(0, 5, getRenderer().getDownscaleBlur().getRt());

		if(g_preferComputeCVar.get())
		{
			rgraphCtx.bindStorageTexture(0, 6, m_runCtx.m_ssrRt);
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
