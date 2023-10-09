// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectSpecular.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static NumericCVar<U32> g_ssrFirstStepPixelsCVar(CVarSubsystem::kRenderer, "SsrFirstStepPixels", 32, 1, 256, "The 1st step in ray marching");
static NumericCVar<U32> g_ssrDepthLodCVar(CVarSubsystem::kRenderer, "SsrDepthLod", (ANKI_PLATFORM_MOBILE) ? 2 : 0, 0, 1000,
										  "Texture LOD of the depth texture that will be raymarched");
static NumericCVar<U32> g_ssrMaxStepsCVar(CVarSubsystem::kRenderer, "SsrMaxSteps", 64, 1, 256, "Max SSR raymarching steps");
static BoolCVar g_ssrStochasticCVar(CVarSubsystem::kRenderer, "SsrStochastic", false, "Stochastic reflections");
static NumericCVar<F32> g_ssrRoughnessCutoffCVar(CVarSubsystem::kRenderer, "SsrRoughnessCutoff", (ANKI_PLATFORM_MOBILE) ? 0.7f : 1.0f, 0.0f, 1.0f,
												 "Materials with roughness higher that this value will fallback to probe reflections");

Error IndirectSpecular::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize indirect specular pass");
	}
	return err;
}

Error IndirectSpecular::initInternal()
{
	const UVec2 size = getRenderer().getInternalResolution() / 2;
	const Bool preferCompute = g_preferComputeCVar.get();

	ANKI_R_LOGV("Initializing indirect specular. Resolution %ux%u", size.x(), size.y());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	// Create RT
	TextureUsageBit usage = TextureUsageBit::kAllSampled;

	usage |= (preferCompute) ? TextureUsageBit::kUavComputeWrite : TextureUsageBit::kFramebufferWrite;

	TextureInitInfo texInit = getRenderer().create2DRenderTargetInitInfo(size.x(), size.y(), getRenderer().getHdrFormat(), usage, "SSR #1");
	m_rts[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);
	texInit.setName("SSR #2");
	m_rts[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	// Create shader
	ANKI_CHECK(ResourceManager::getSingleton().loadResource((g_preferComputeCVar.get()) ? "ShaderBinaries/IndirectSpecularCompute.ankiprogbin"
																						: "ShaderBinaries/IndirectSpecularRaster.ankiprogbin",
															m_prog));

	ShaderProgramResourceVariantInitInfo variantInit(m_prog);
	variantInit.addMutation("EXTRA_REJECTION", false);
	variantInit.addMutation("STOCHASTIC", g_ssrStochasticCVar.get());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInit, variant);
	m_grProg.reset(&variant->getProgram());

	return Error::kNone;
}

void IndirectSpecular::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectSpecular);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();
	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get() && !preferCompute;
	const Bool fbDescrHasVrs = m_fbDescr.m_shadingRateAttachmentTexelWidth > 0;

	// Create/import RTs
	const U32 readRtIdx = getRenderer().getFrameCount() & 1;
	const U32 writeRtIdx = !readRtIdx;
	if(m_rtsImportedOnce) [[likely]]
	{
		m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rts[readRtIdx].get());
		m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rts[writeRtIdx].get());
	}
	else
	{
		m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rts[readRtIdx].get(), TextureUsageBit::kAllSampled);
		m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rts[writeRtIdx].get(), TextureUsageBit::kAllSampled);
		m_rtsImportedOnce = true;
	}

	// Re-bake FB descriptor
	if(!preferCompute && enableVrs != fbDescrHasVrs)
	{
		// Re-bake the FB descriptor if the VRS state has changed

		if(enableVrs)
		{
			m_fbDescr.m_shadingRateAttachmentTexelWidth = getRenderer().getVrsSriGeneration().getSriTexelDimension();
			m_fbDescr.m_shadingRateAttachmentTexelHeight = getRenderer().getVrsSriGeneration().getSriTexelDimension();
		}
		else
		{
			m_fbDescr.m_shadingRateAttachmentTexelWidth = 0;
			m_fbDescr.m_shadingRateAttachmentTexelHeight = 0;
		}

		m_fbDescr.bake();
	}

	// Create pass
	{
		RenderPassDescriptionBase* ppass;
		TextureUsageBit readUsage;
		TextureUsageBit writeUsage;
		BufferUsageBit readBufferUsage;
		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("SSR");

			ppass = &pass;
			readUsage = TextureUsageBit::kSampledCompute;
			writeUsage = TextureUsageBit::kUavComputeWrite;
			readBufferUsage = BufferUsageBit::kUavComputeRead;
		}
		else
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("SSR");
			pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rts[kWrite]}, {},
									(enableVrs) ? getRenderer().getVrsSriGeneration().getDownscaledSriRt() : RenderTargetHandle());

			ppass = &pass;
			readUsage = TextureUsageBit::kSampledFragment;
			writeUsage = TextureUsageBit::kFramebufferWrite;
			readBufferUsage = BufferUsageBit::kUavFragmentRead;

			if(enableVrs)
			{
				ppass->newTextureDependency(getRenderer().getVrsSriGeneration().getDownscaledSriRt(), TextureUsageBit::kFramebufferShadingRate);
			}
		}

		ppass->newTextureDependency(m_runCtx.m_rts[kWrite], writeUsage);
		ppass->newTextureDependency(m_runCtx.m_rts[kRead], readUsage);
		ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(1), readUsage);
		ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
		ppass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage);

		if(getRenderer().getProbeReflections().getHasCurrentlyRefreshedReflectionRt())
		{
			ppass->newTextureDependency(getRenderer().getProbeReflections().getCurrentlyRefreshedReflectionRt(), readUsage);
		}

		ppass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);
		ppass->newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), readUsage);

		ppass->newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kReflectionProbe),
								   readBufferUsage);
		ppass->newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), readBufferUsage);

		ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			run(ctx, rgraphCtx);
		});
	}
}

void IndirectSpecular::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
	cmdb.bindShaderProgram(m_grProg.get());

	const U32 depthLod = min(g_ssrDepthLodCVar.get(), getRenderer().getDepthDownscale().getMipmapCount() - 1);

	// Bind uniforms
	SsrConstants* unis = allocateAndBindConstants<SsrConstants>(cmdb, 0, 0);
	unis->m_depthBufferSize = getRenderer().getInternalResolution() >> (depthLod + 1);
	unis->m_framebufferSize = UVec2(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y()) / 2;
	unis->m_frameCount = getRenderer().getFrameCount() & kMaxU32;
	unis->m_depthMipCount = getRenderer().getDepthDownscale().getMipmapCount();
	unis->m_maxSteps = g_ssrMaxStepsCVar.get();
	unis->m_lightBufferMipCount = getRenderer().getDownscaleBlur().getMipmapCount();
	unis->m_firstStepPixels = g_ssrFirstStepPixelsCVar.get();
	unis->m_prevViewProjMatMulInvViewProjMat = ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	unis->m_projMat = ctx.m_matrices.m_projectionJitter;
	unis->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
	unis->m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
	unis->m_roughnessCutoff = g_ssrRoughnessCutoffCVar.get();

	// Bind all
	cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());

	rgraphCtx.bindColorTexture(0, 2, getRenderer().getGBuffer().getColorRt(1));
	rgraphCtx.bindColorTexture(0, 3, getRenderer().getGBuffer().getColorRt(2));

	TextureSubresourceInfo hizSubresource;
	hizSubresource.m_mipmapCount = depthLod + 1;
	rgraphCtx.bindTexture(0, 4, getRenderer().getDepthDownscale().getRt(), hizSubresource);

	rgraphCtx.bindColorTexture(0, 5, getRenderer().getDownscaleBlur().getRt());

	rgraphCtx.bindColorTexture(0, 6, m_runCtx.m_rts[kRead]);
	rgraphCtx.bindColorTexture(0, 7, getRenderer().getMotionVectors().getMotionVectorsRt());
	rgraphCtx.bindColorTexture(0, 8, getRenderer().getMotionVectors().getHistoryLengthRt());

	cmdb.bindSampler(0, 9, getRenderer().getSamplers().m_trilinearRepeat.get());
	cmdb.bindTexture(0, 10, &m_noiseImage->getTextureView());

	BufferOffsetRange buff = getRenderer().getClusterBinning().getClusteredShadingConstants();
	cmdb.bindConstantBuffer(0, 11, buff.m_buffer, buff.m_offset, buff.m_range);

	buff = getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kReflectionProbe);
	cmdb.bindUavBuffer(0, 12, buff.m_buffer, buff.m_offset, buff.m_range);

	buff = getRenderer().getClusterBinning().getClustersBuffer();
	cmdb.bindUavBuffer(0, 13, buff.m_buffer, buff.m_offset, buff.m_range);

	cmdb.bindAllBindless(1);

	if(g_preferComputeCVar.get())
	{
		rgraphCtx.bindUavTexture(0, 14, m_runCtx.m_rts[kWrite], TextureSubresourceInfo());

		dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
	}
	else
	{
		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);

		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

void IndirectSpecular::getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
											[[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
{
	if(rtName == "SSR")
	{
		handles[0] = m_runCtx.m_rts[kWrite];
	}
}

} // end namespace anki
