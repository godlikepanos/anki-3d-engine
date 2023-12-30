// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Ssr.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Renderer/LightShading.h>
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

// TODO remove the _ in the names
static NumericCVar<U32> g_ssrStepIncrementCVar(CVarSubsystem::kRenderer, "SsrStepIncrement", 32, 1, 256, "The number of steps for each loop");
static NumericCVar<U32> g_ssrMaxIterationsCVar(CVarSubsystem::kRenderer, "SsrMaxIterations", 64, 1, 256, "Max SSR raymarching loop iterations");
static NumericCVar<F32> g_ssrRoughnessCutoffCVar(CVarSubsystem::kRenderer, "SsrRoughnessCutoff_", (ANKI_PLATFORM_MOBILE) ? 0.7f : 1.0f, 0.0f, 1.0f,
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
		mipTexUsage |= TextureUsageBit::kUavComputeWrite;
	}
	else
	{
		mipTexUsage |= TextureUsageBit::kFramebufferWrite;
	}

	TextureInitInfo mipTexInit =
		getRenderer().create2DRenderTargetInitInfo(getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2,
												   getRenderer().getHdrFormat(), mipTexUsage, "Downscaled light shading");
	mipTexInit.m_mipmapCount = U8(computeMaxMipmapCount2d(mipTexInit.m_width, mipTexInit.m_height, 16));
	m_mipmappedLightShdingTex = getRenderer().createAndClearRenderTarget(mipTexInit, TextureUsageBit::kAllSampled);

	m_ssrRtDescr = getRenderer().create2DRenderTargetDescription(rez.x(), rez.y(), Format::kR16G16B16A16_Sfloat, "SSR");
	m_ssrRtDescr.bake();

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssr.ankiprogbin", {}, m_prog, m_mipmapLightShadingGrProg, "MipGeneration"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Ssr.ankiprogbin", {}, m_prog, m_ssrGrProg, "Ssr"));

	BufferInitInfo buffInit("SsrCounterBuffer");
	buffInit.m_size = sizeof(U32);
	buffInit.m_usage = BufferUsageBit::kUavComputeWrite | BufferUsageBit::kTransferDestination;
	m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);
	zeroBuffer(m_counterBuffer.get());

	return Error::kNone;
}

void Ssr::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Ssr);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	if(m_mipmappedLightShdingTexImportedOnce) [[likely]]
	{
		m_runCtx.m_mipmappedLightShadingRt = rgraph.importRenderTarget(m_mipmappedLightShdingTex.get());
	}
	else
	{
		m_runCtx.m_mipmappedLightShadingRt = rgraph.importRenderTarget(m_mipmappedLightShdingTex.get(), TextureUsageBit::kAllSampled);
		m_mipmappedLightShdingTexImportedOnce = true;
	}

	m_runCtx.m_ssrRt = rgraph.newRenderTarget(m_ssrRtDescr);

	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	RenderPassDescriptionBase* ppass;
	if(preferCompute)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSR");
		ppass = &pass;

		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kUavComputeWrite;
	}
	else
	{
		// TODO
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSR");
		ppass = &pass;

		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
	}

	ppass->newTextureDependency(m_runCtx.m_mipmappedLightShadingRt, readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(1), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
	ppass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage);
	ppass->newTextureDependency(m_runCtx.m_ssrRt, writeUsage);

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		const UVec2 rez = getRenderer().getInternalResolution() / ((g_ssrQuarterResolution.get()) ? 2 : 1);

		cmdb.bindShaderProgram(m_ssrGrProg.get());

		SsrConstants2 consts = {};
		consts.m_viewportSizef = Vec2(rez);
		consts.m_frameCount = getRenderer().getFrameCount() % kMaxU32;
		consts.m_maxIterations = g_ssrMaxIterationsCVar.get();
		consts.m_roughnessCutoff = g_ssrRoughnessCutoffCVar.get();
		consts.m_stepIncrement = g_ssrStepIncrementCVar.get();
		consts.m_prevViewProjMatMulInvViewProjMat = ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
		consts.m_projMat = ctx.m_matrices.m_projectionJitter;
		consts.m_invProjMat = ctx.m_matrices.m_invertedProjectionJitter;
		consts.m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
		*allocateAndBindConstants<SsrConstants2>(cmdb, 0, 0) = consts;

		cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindColorTexture(0, 2, getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 3, getRenderer().getGBuffer().getColorRt(2));
		rgraphCtx.bindColorTexture(0, 4, getRenderer().getDepthDownscale().getRt());
		rgraphCtx.bindColorTexture(0, 5, m_runCtx.m_mipmappedLightShadingRt);

		if(g_preferComputeCVar.get())
		{
			rgraphCtx.bindUavTexture(0, 6, m_runCtx.m_ssrRt);
			dispatchPPCompute(cmdb, 8, 8, rez.x(), rez.y());
		}
		else
		{
			cmdb.setViewport(0, 0, rez.x(), rez.y());
			drawQuad(cmdb);
		}
	});
}

void Ssr::populateRenderGraphPostLightShading(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(SsrPost);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	RenderPassDescriptionBase* ppass;
	if(preferCompute)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSR: Light downscale");
		ppass = &pass;

		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kUavComputeWrite;
	}
	else
	{
		// TODO
		ANKI_ASSERT(0);
		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
	}

	ppass->newTextureDependency(getRenderer().getLightShading().getRt(), readUsage);
	ppass->newTextureDependency(m_runCtx.m_mipmappedLightShadingRt, writeUsage);

	ppass->setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_mipmapLightShadingGrProg.get());

		const U32 mipsToCompute = m_mipmappedLightShdingTex->getMipmapCount();

		varAU2(dispatchThreadGroupCountXY);
		varAU2(workGroupOffset); // needed if Left and Top are not 0,0
		varAU2(numWorkGroupsAndMips);
		varAU4(rectInfo) = initAU4(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, mipsToCompute);

		class Constants
		{
		public:
			Vec2 m_invSrcTexSize;
			U32 m_threadGroupCount;
			U32 m_mipmapCount;
		} consts;
		consts.m_invSrcTexSize = 1.0f / Vec2(getRenderer().getInternalResolution());
		consts.m_threadGroupCount = numWorkGroupsAndMips[0];
		consts.m_mipmapCount = numWorkGroupsAndMips[1];

		cmdb.setPushConstants(&consts, sizeof(consts));

		for(U32 mip = 0; mip < kMaxMipsSinglePassDownsamplerCanProduce; ++mip)
		{
			TextureSubresourceInfo subresource;
			if(mip < mipsToCompute)
			{
				subresource.m_firstMipmap = mip;
			}
			else
			{
				subresource.m_firstMipmap = 0; // Put something random
			}

			rgraphCtx.bindUavTexture(0, 0, m_runCtx.m_mipmappedLightShadingRt, subresource, mip);
		}

		cmdb.bindUavBuffer(0, 1, m_counterBuffer.get(), 0, sizeof(U32));
		rgraphCtx.bindColorTexture(0, 2, getRenderer().getLightShading().getRt());
		cmdb.bindSampler(0, 3, getRenderer().getSamplers().m_trilinearClamp.get());

		cmdb.dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
	});
}

} // end namespace anki
