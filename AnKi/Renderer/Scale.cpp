// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Scale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Core/CVarSet.h>

#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Util/Tracer.h>

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
#include <ThirdParty/FidelityFX/ffx_fsr1.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#elif ANKI_COMPILER_MSVC
#	pragma warning(pop)
#endif

namespace anki {

static NumericCVar<U8> g_fsrQualityCVar(CVarSubsystem::kRenderer, "FsrQuality", 1, 0, 2, "0: Use bilinear, 1: FSR low quality, 2: FSR high quality");
static NumericCVar<U8> g_dlssQualityCVar(CVarSubsystem::kRenderer, "DlssQuality", 2, 0, 3, "0: Disabled, 1: Performance, 2: Balanced, 3: Quality");
static NumericCVar<F32> g_sharpnessCVar(CVarSubsystem::kRenderer, "Sharpness", (ANKI_PLATFORM_MOBILE) ? 0.0f : 0.8f, 0.0f, 1.0f,
										"Sharpen the image. It's a factor");

Error Scale::init()
{
	const Bool needsScaling = getRenderer().getPostProcessResolution() != getRenderer().getInternalResolution();
	const Bool needsSharpening = g_sharpnessCVar.get() > 0.0f;
	if(!needsScaling && !needsSharpening)
	{
		return Error::kNone;
	}

	const Bool preferCompute = g_preferComputeCVar.get();
	const U32 dlssQuality = g_dlssQualityCVar.get();
	const U32 fsrQuality = g_fsrQualityCVar.get();

	if(needsScaling)
	{
		if(dlssQuality > 0 && GrManager::getSingleton().getDeviceCapabilities().m_dlss)
		{
			m_upscalingMethod = UpscalingMethod::kGr;
		}
		else if(fsrQuality > 0)
		{
			m_upscalingMethod = UpscalingMethod::kFsr;
		}
		else
		{
			m_upscalingMethod = UpscalingMethod::kBilinear;
		}
	}
	else
	{
		m_upscalingMethod = UpscalingMethod::kNone;
	}

	m_sharpenMethod = (needsSharpening) ? SharpenMethod::kRcas : SharpenMethod::kNone;
	m_neeedsTonemapping = (m_upscalingMethod == UpscalingMethod::kGr); // Because GR upscaling spits HDR

	static constexpr Array<const Char*, U32(UpscalingMethod::kCount)> upscalingMethodNames = {"none", "bilinear", "FSR 1.0", "DLSS 2"};
	static constexpr Array<const Char*, U32(SharpenMethod::kCount)> sharpenMethodNames = {"none", "RCAS"};

	ANKI_R_LOGV("Initializing upscaling. Upscaling method %s, sharpenning method %s", upscalingMethodNames[m_upscalingMethod],
				sharpenMethodNames[m_sharpenMethod]);

	// Scale programs
	if(m_upscalingMethod == UpscalingMethod::kBilinear)
	{
		const CString shaderFname = (preferCompute) ? "ShaderBinaries/BlitCompute.ankiprogbin" : "ShaderBinaries/BlitRaster.ankiprogbin";

		ANKI_CHECK(ResourceManager::getSingleton().loadResource(shaderFname, m_scaleProg));

		const ShaderProgramResourceVariant* variant;
		m_scaleProg->getOrCreateVariant(variant);
		m_scaleGrProg.reset(&variant->getProgram());
	}
	else if(m_upscalingMethod == UpscalingMethod::kFsr)
	{
		const Array<SubMutation, 2> mutation = {{{"SHARPEN", 0}, {"FSR_QUALITY", MutatorValue(fsrQuality - 1)}}};
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Fsr.ankiprogbin", mutation, m_scaleProg, m_scaleGrProg));
	}
	else if(m_upscalingMethod == UpscalingMethod::kGr)
	{
		GrUpscalerInitInfo inf;
		inf.m_sourceTextureResolution = getRenderer().getInternalResolution();
		inf.m_targetTextureResolution = getRenderer().getPostProcessResolution();
		inf.m_upscalerType = GrUpscalerType::kDlss2;
		inf.m_qualityMode = GrUpscalerQualityMode(dlssQuality - 1);

		m_grUpscaler = GrManager::getSingleton().newGrUpscaler(inf);
	}

	// Sharpen programs
	if(m_sharpenMethod == SharpenMethod::kRcas)
	{
		const Array<SubMutation, 2> mutation = {{{"SHARPEN", 1}, {"FSR_QUALITY", 0}}};
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Fsr.ankiprogbin", mutation, m_sharpenProg, m_sharpenGrProg));
	}

	// Tonemapping programs
	if(m_neeedsTonemapping)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(
			(preferCompute) ? "ShaderBinaries/TonemapCompute.ankiprogbin" : "ShaderBinaries/TonemapRaster.ankiprogbin", m_tonemapProg));
		const ShaderProgramResourceVariant* variant;
		m_tonemapProg->getOrCreateVariant(variant);
		m_tonemapGrProg.reset(&variant->getProgram());
	}

	// Descriptors
	Format format;
	if(m_upscalingMethod == UpscalingMethod::kGr)
	{
		format = getRenderer().getHdrFormat();
	}
	else if(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats)
	{
		format = Format::kR8G8B8_Unorm;
	}
	else
	{
		format = Format::kR8G8B8A8_Unorm;
	}

	m_upscaleAndSharpenRtDescr = getRenderer().create2DRenderTargetDescription(getRenderer().getPostProcessResolution().x(),
																			   getRenderer().getPostProcessResolution().y(), format, "Scaling");
	m_upscaleAndSharpenRtDescr.bake();

	if(m_neeedsTonemapping)
	{
		const Format fmt =
			(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm : Format::kR8G8B8A8_Unorm;
		m_tonemapedRtDescr = getRenderer().create2DRenderTargetDescription(getRenderer().getPostProcessResolution().x(),
																		   getRenderer().getPostProcessResolution().y(), fmt, "Tonemapped");
		m_tonemapedRtDescr.bake();
	}

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	return Error::kNone;
}

void Scale::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Scale);
	if(m_upscalingMethod == UpscalingMethod::kNone && m_sharpenMethod == SharpenMethod::kNone)
	{
		m_runCtx.m_upscaledTonemappedRt = getRenderer().getTemporalAA().getTonemappedRt();
		m_runCtx.m_upscaledHdrRt = getRenderer().getTemporalAA().getHdrRt();
		m_runCtx.m_sharpenedRt = getRenderer().getTemporalAA().getTonemappedRt();
		m_runCtx.m_tonemappedRt = getRenderer().getTemporalAA().getTonemappedRt();
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	// Step 1: Upscaling
	if(m_upscalingMethod == UpscalingMethod::kGr)
	{
		m_runCtx.m_upscaledHdrRt = rgraph.newRenderTarget(m_upscaleAndSharpenRtDescr);
		m_runCtx.m_upscaledTonemappedRt = {};

		ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("DLSS");

		// DLSS says input textures in sampled state and out as storage image
		const TextureUsageBit readUsage = TextureUsageBit::kAllSampled & TextureUsageBit::kAllCompute;
		const TextureUsageBit writeUsage = TextureUsageBit::kAllUav & TextureUsageBit::kAllCompute;

		pass.newTextureDependency(getRenderer().getLightShading().getRt(), readUsage);
		pass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);
		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		pass.newTextureDependency(m_runCtx.m_upscaledHdrRt, writeUsage);

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			runGrUpscaling(ctx, rgraphCtx);
		});
	}
	else if(m_upscalingMethod == UpscalingMethod::kFsr || m_upscalingMethod == UpscalingMethod::kBilinear)
	{
		m_runCtx.m_upscaledTonemappedRt = rgraph.newRenderTarget(m_upscaleAndSharpenRtDescr);
		m_runCtx.m_upscaledHdrRt = {};
		const RenderTargetHandle inRt = getRenderer().getTemporalAA().getTonemappedRt();
		const RenderTargetHandle outRt = m_runCtx.m_upscaledTonemappedRt;

		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Scale");
			pass.newTextureDependency(inRt, TextureUsageBit::kSampledCompute);
			pass.newTextureDependency(outRt, TextureUsageBit::kUavComputeWrite);

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runFsrOrBilinearScaling(rgraphCtx);
			});
		}
		else
		{
			GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Scale");
			pass.setFramebufferInfo(m_fbDescr, {outRt});
			pass.newTextureDependency(inRt, TextureUsageBit::kSampledFragment);
			pass.newTextureDependency(outRt, TextureUsageBit::kFramebufferWrite);

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runFsrOrBilinearScaling(rgraphCtx);
			});
		}
	}
	else
	{
		ANKI_ASSERT(m_upscalingMethod == UpscalingMethod::kNone);
		// Pretend that it got scaled
		m_runCtx.m_upscaledTonemappedRt = getRenderer().getTemporalAA().getTonemappedRt();
		m_runCtx.m_upscaledHdrRt = getRenderer().getTemporalAA().getHdrRt();
	}

	// Step 2: Tonemapping
	if(m_neeedsTonemapping)
	{
		m_runCtx.m_tonemappedRt = rgraph.newRenderTarget(m_tonemapedRtDescr);
		const RenderTargetHandle inRt = m_runCtx.m_upscaledHdrRt;
		const RenderTargetHandle outRt = m_runCtx.m_tonemappedRt;

		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Tonemap");
			pass.newTextureDependency(inRt, TextureUsageBit::kSampledCompute);
			pass.newTextureDependency(outRt, TextureUsageBit::kUavComputeWrite);

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runTonemapping(rgraphCtx);
			});
		}
		else
		{
			GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Sharpen");
			pass.setFramebufferInfo(m_fbDescr, {outRt});
			pass.newTextureDependency(inRt, TextureUsageBit::kSampledFragment);
			pass.newTextureDependency(outRt, TextureUsageBit::kFramebufferWrite);

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runTonemapping(rgraphCtx);
			});
		}
	}
	else
	{
		m_runCtx.m_tonemappedRt = m_runCtx.m_upscaledTonemappedRt;
	}

	// Step 3: Sharpenning
	if(m_sharpenMethod == SharpenMethod::kRcas)
	{
		m_runCtx.m_sharpenedRt = rgraph.newRenderTarget(m_upscaleAndSharpenRtDescr);
		const RenderTargetHandle inRt = m_runCtx.m_tonemappedRt;
		const RenderTargetHandle outRt = m_runCtx.m_sharpenedRt;

		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Sharpen");
			pass.newTextureDependency(inRt, TextureUsageBit::kSampledCompute);
			pass.newTextureDependency(outRt, TextureUsageBit::kUavComputeWrite);

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runRcasSharpening(rgraphCtx);
			});
		}
		else
		{
			GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Sharpen");
			pass.setFramebufferInfo(m_fbDescr, {outRt});
			pass.newTextureDependency(inRt, TextureUsageBit::kSampledFragment);
			pass.newTextureDependency(outRt, TextureUsageBit::kFramebufferWrite);

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runRcasSharpening(rgraphCtx);
			});
		}
	}
	else
	{
		ANKI_ASSERT(m_sharpenMethod == SharpenMethod::kNone);
		// Pretend that it's sharpened
		m_runCtx.m_sharpenedRt = m_runCtx.m_tonemappedRt;
	}
}

void Scale::runFsrOrBilinearScaling(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(Scale);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
	const Bool preferCompute = g_preferComputeCVar.get();
	const RenderTargetHandle inRt = getRenderer().getTemporalAA().getTonemappedRt();
	const RenderTargetHandle outRt = m_runCtx.m_upscaledTonemappedRt;

	cmdb.bindShaderProgram(m_scaleGrProg.get());

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
	rgraphCtx.bindColorTexture(0, 1, inRt);

	if(preferCompute)
	{
		rgraphCtx.bindUavTexture(0, 2, outRt);
	}

	if(m_upscalingMethod == UpscalingMethod::kFsr)
	{
		class
		{
		public:
			UVec4 m_fsrConsts0;
			UVec4 m_fsrConsts1;
			UVec4 m_fsrConsts2;
			UVec4 m_fsrConsts3;
			UVec2 m_viewportSize;
			UVec2 m_padding;
		} pc;

		const Vec2 inRez(getRenderer().getInternalResolution());
		const Vec2 outRez(getRenderer().getPostProcessResolution());
		FsrEasuCon(&pc.m_fsrConsts0[0], &pc.m_fsrConsts1[0], &pc.m_fsrConsts2[0], &pc.m_fsrConsts3[0], inRez.x(), inRez.y(), inRez.x(), inRez.y(),
				   outRez.x(), outRez.y());

		pc.m_viewportSize = getRenderer().getPostProcessResolution();

		cmdb.setPushConstants(&pc, sizeof(pc));
	}
	else if(preferCompute)
	{
		class
		{
		public:
			Vec2 m_viewportSize;
			UVec2 m_viewportSizeU;
		} pc;
		pc.m_viewportSize = Vec2(getRenderer().getPostProcessResolution());
		pc.m_viewportSizeU = getRenderer().getPostProcessResolution();

		cmdb.setPushConstants(&pc, sizeof(pc));
	}

	if(preferCompute)
	{
		dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
	}
	else
	{
		cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

void Scale::runRcasSharpening(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(Scale);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
	const Bool preferCompute = g_preferComputeCVar.get();
	const RenderTargetHandle inRt = m_runCtx.m_tonemappedRt;
	const RenderTargetHandle outRt = m_runCtx.m_sharpenedRt;

	cmdb.bindShaderProgram(m_sharpenGrProg.get());

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
	rgraphCtx.bindColorTexture(0, 1, inRt);

	if(preferCompute)
	{
		rgraphCtx.bindUavTexture(0, 2, outRt);
	}

	class
	{
	public:
		UVec4 m_fsrConsts0;
		UVec4 m_fsrConsts1;
		UVec4 m_fsrConsts2;
		UVec4 m_fsrConsts3;
		UVec2 m_viewportSize;
		UVec2 m_padding;
	} pc;

	F32 sharpness = g_sharpnessCVar.get(); // [0, 1]
	sharpness *= 3.0f; // [0, 3]
	sharpness = 3.0f - sharpness; // [3, 0], RCAS translates 0 to max sharpness
	FsrRcasCon(&pc.m_fsrConsts0[0], sharpness);

	pc.m_viewportSize = getRenderer().getPostProcessResolution();

	cmdb.setPushConstants(&pc, sizeof(pc));

	if(preferCompute)
	{
		dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
	}
	else
	{
		cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

void Scale::runGrUpscaling(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(Scale);
	const Vec2 srcRes(getRenderer().getInternalResolution());
	const Bool reset = getRenderer().getFrameCount() == 0;
	const Vec2 mvScale = srcRes; // UV space to Pixel space factor
	// In [-texSize / 2, texSize / 2] -> sub-pixel space {-0.5, 0.5}
	const Vec2 jitterOffset = ctx.m_matrices.m_jitter.getTranslationPart().xy() * srcRes * 0.5f;

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	TextureViewPtr srcView = rgraphCtx.createTextureView(getRenderer().getLightShading().getRt());
	TextureViewPtr motionVectorsView = rgraphCtx.createTextureView(getRenderer().getMotionVectors().getMotionVectorsRt());
	TextureViewPtr depthView = rgraphCtx.createTextureView(getRenderer().getGBuffer().getDepthRt());
	TextureViewPtr exposureView = rgraphCtx.createTextureView(getRenderer().getTonemapping().getRt());
	TextureViewPtr dstView = rgraphCtx.createTextureView(m_runCtx.m_upscaledHdrRt);

	cmdb.upscale(m_grUpscaler.get(), srcView.get(), dstView.get(), motionVectorsView.get(), depthView.get(), exposureView.get(), reset, jitterOffset,
				 mvScale);
}

void Scale::runTonemapping(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(Scale);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
	const Bool preferCompute = g_preferComputeCVar.get();
	const RenderTargetHandle inRt = m_runCtx.m_upscaledHdrRt;
	const RenderTargetHandle outRt = m_runCtx.m_tonemappedRt;

	cmdb.bindShaderProgram(m_tonemapGrProg.get());

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
	rgraphCtx.bindColorTexture(0, 1, inRt);

	rgraphCtx.bindUavTexture(0, 2, getRenderer().getTonemapping().getRt());

	if(preferCompute)
	{
		class
		{
		public:
			Vec2 m_viewportSizeOverOne;
			UVec2 m_viewportSize;
		} pc;
		pc.m_viewportSizeOverOne = 1.0f / Vec2(getRenderer().getPostProcessResolution());
		pc.m_viewportSize = getRenderer().getPostProcessResolution();
		cmdb.setPushConstants(&pc, sizeof(pc));
		rgraphCtx.bindUavTexture(0, 3, outRt);

		dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
	}
	else
	{
		cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
		cmdb.draw(PrimitiveTopology::kTriangles, 3);
	}
}

} // end namespace anki
