// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Scale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Core/ConfigSet.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-function"
#	pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
#define A_CPU
#include <ThirdParty/FidelityFX/ffx_a.h>
#include <ThirdParty/FidelityFX/ffx_fsr1.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki {

Scale::~Scale()
{
}

Error Scale::init()
{
	ANKI_R_LOGV("Initializing scale");

	const Bool needsScaling = m_r->getPostProcessResolution() != m_r->getInternalResolution();
	const Bool needsSharpening = getConfig().getRSharpen();
	if(!needsScaling && !needsSharpening)
	{
		return Error::NONE;
	}

	const Bool preferCompute = getConfig().getRPreferCompute();
	const U32 fsrQuality = getConfig().getRFsr();
	m_fsr = fsrQuality != 0;

	// Program
	if(needsScaling)
	{
		CString shaderFname;
		if(m_fsr && preferCompute)
		{
			shaderFname = "Shaders/FsrCompute.ankiprog";
		}
		else if(m_fsr)
		{
			shaderFname = "Shaders/FsrRaster.ankiprog";
		}
		else if(preferCompute)
		{
			shaderFname = "Shaders/BlitCompute.ankiprog";
		}
		else
		{
			shaderFname = "Shaders/BlitRaster.ankiprog";
		}

		ANKI_CHECK(getResourceManager().loadResource(shaderFname, m_scaleProg));
		const ShaderProgramResourceVariant* variant;
		if(m_fsr)
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_scaleProg);
			variantInitInfo.addMutation("SHARPEN", 0);
			variantInitInfo.addMutation("FSR_QUALITY", fsrQuality - 1);
			m_scaleProg->getOrCreateVariant(variantInitInfo, variant);
		}
		else
		{
			m_scaleProg->getOrCreateVariant(variant);
		}
		m_scaleGrProg = variant->getProgram();
	}

	if(needsSharpening)
	{
		ANKI_CHECK(getResourceManager().loadResource(
			(preferCompute) ? "Shaders/FsrCompute.ankiprog" : "Shaders/FsrRaster.ankiprog", m_sharpenProg));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_sharpenProg);
		variantInitInfo.addMutation("SHARPEN", 1);
		variantInitInfo.addMutation("FSR_QUALITY", 0);
		const ShaderProgramResourceVariant* variant;
		m_sharpenProg->getOrCreateVariant(variantInitInfo, variant);
		m_sharpenGrProg = variant->getProgram();
	}

	// Descriptors
	m_rtDesc =
		m_r->create2DRenderTargetDescription(m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y(),
											 LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "Scaled");
	m_rtDesc.bake();

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	return Error::NONE;
}

void Scale::populateRenderGraph(RenderingContext& ctx)
{
	if(!doScaling() && !doSharpening())
	{
		m_runCtx.m_scaledRt = m_r->getTemporalAA().getTonemappedRt();
		m_runCtx.m_sharpenedRt = m_r->getTemporalAA().getTonemappedRt();
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = getConfig().getRPreferCompute();

	if(doScaling())
	{
		m_runCtx.m_scaledRt = rgraph.newRenderTarget(m_rtDesc);

		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Scale");
			pass.newDependency(
				RenderPassDependency(m_r->getTemporalAA().getTonemappedRt(), TextureUsageBit::SAMPLED_COMPUTE));
			pass.newDependency(RenderPassDependency(m_runCtx.m_scaledRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runScaling(rgraphCtx);
			});
		}
		else
		{
			GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Scale");
			pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_scaledRt});

			pass.newDependency(
				RenderPassDependency(m_r->getTemporalAA().getTonemappedRt(), TextureUsageBit::SAMPLED_FRAGMENT));
			pass.newDependency(
				RenderPassDependency(m_runCtx.m_scaledRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runScaling(rgraphCtx);
			});
		}
	}

	if(doSharpening())
	{
		m_runCtx.m_sharpenedRt = rgraph.newRenderTarget(m_rtDesc);

		if(preferCompute)
		{
			ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Sharpen");
			pass.newDependency(
				RenderPassDependency((!doScaling()) ? m_r->getTemporalAA().getTonemappedRt() : m_runCtx.m_scaledRt,
									 TextureUsageBit::SAMPLED_COMPUTE));
			pass.newDependency(RenderPassDependency(m_runCtx.m_sharpenedRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runSharpening(rgraphCtx);
			});
		}
		else
		{
			GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Sharpen");
			pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_sharpenedRt});

			pass.newDependency(
				RenderPassDependency((!doScaling()) ? m_r->getTemporalAA().getTonemappedRt() : m_runCtx.m_scaledRt,
									 TextureUsageBit::SAMPLED_FRAGMENT));
			pass.newDependency(
				RenderPassDependency(m_runCtx.m_sharpenedRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));

			pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
				runSharpening(rgraphCtx);
			});
		}
	}
}

void Scale::runScaling(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const Bool preferCompute = getConfig().getRPreferCompute();

	cmdb->bindShaderProgram(m_scaleGrProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_r->getTemporalAA().getTonemappedRt());

	if(preferCompute)
	{
		rgraphCtx.bindImage(0, 2, m_runCtx.m_scaledRt);
	}

	if(m_fsr)
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

		const Vec2 inRez(m_r->getInternalResolution());
		const Vec2 outRez(m_r->getPostProcessResolution());
		FsrEasuCon(&pc.m_fsrConsts0[0], &pc.m_fsrConsts1[0], &pc.m_fsrConsts2[0], &pc.m_fsrConsts3[0], inRez.x(),
				   inRez.y(), inRez.x(), inRez.y(), outRez.x(), outRez.y());

		pc.m_viewportSize = m_r->getPostProcessResolution();

		cmdb->setPushConstants(&pc, sizeof(pc));
	}
	else if(preferCompute)
	{
		class
		{
		public:
			Vec2 m_viewportSize;
			UVec2 m_viewportSizeU;
		} pc;
		pc.m_viewportSize = Vec2(m_r->getPostProcessResolution());
		pc.m_viewportSizeU = m_r->getPostProcessResolution();

		cmdb->setPushConstants(&pc, sizeof(pc));
	}

	if(preferCompute)
	{
		dispatchPPCompute(cmdb, 8, 8, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());
	}
	else
	{
		cmdb->setViewport(0, 0, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	}
}

void Scale::runSharpening(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const Bool preferCompute = getConfig().getRPreferCompute();

	cmdb->bindShaderProgram(m_sharpenGrProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, (!doScaling()) ? m_r->getTemporalAA().getTonemappedRt() : m_runCtx.m_scaledRt);

	if(preferCompute)
	{
		rgraphCtx.bindImage(0, 2, m_runCtx.m_sharpenedRt);
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

	FsrRcasCon(&pc.m_fsrConsts0[0], 0.2f);

	pc.m_viewportSize = m_r->getPostProcessResolution();

	cmdb->setPushConstants(&pc, sizeof(pc));

	if(preferCompute)
	{
		dispatchPPCompute(cmdb, 8, 8, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());
	}
	else
	{
		cmdb->setViewport(0, 0, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	}
}

} // end namespace anki
