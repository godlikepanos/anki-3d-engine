// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
#include <ThirdParty/Fsr/ffx_a.h>
#include <ThirdParty/Fsr/ffx_fsr1.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki
{

Scale::~Scale()
{
}

Error Scale::init(const ConfigSet& cfg)
{
	const Bool needsScale = m_r->getPostProcessResolution() != m_r->getInternalResolution();
	if(!needsScale)
	{
		return Error::NONE;
	}

	ANKI_R_LOGI("Initializing (up|down)scale pass");

	m_fsr = cfg.getBool("r_fsr");

	// Program
	ANKI_CHECK(
		getResourceManager().loadResource((m_fsr) ? "Shaders/Fsr.ankiprog" : "Shaders/BlitCompute.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	// The RT desc
	m_rtDesc =
		m_r->create2DRenderTargetDescription(m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y(),
											 LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "Scaled");
	m_rtDesc.bake();

	return Error::NONE;
}

void Scale::populateRenderGraph(RenderingContext& ctx)
{
	const Bool needsScale = m_grProg.isCreated();
	if(!needsScale)
	{
		m_runCtx.m_upscaledRt = m_r->getTemporalAA().getRt();
	}
	else
	{
		RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

		m_runCtx.m_upscaledRt = rgraph.newRenderTarget(m_rtDesc);

		ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Scale");
		pass.newDependency(RenderPassDependency(m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_COMPUTE));
		pass.newDependency(RenderPassDependency(m_runCtx.m_upscaledRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));

		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				Scale* const self = static_cast<Scale*>(rgraphCtx.m_userData);
				self->run(rgraphCtx);
			},
			this, 0);
	}
}

void Scale::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_r->getTemporalAA().getRt());
	rgraphCtx.bindImage(0, 2, m_runCtx.m_upscaledRt);

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
	else
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

	dispatchPPCompute(cmdb, 8, 8, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());
}

} // end namespace anki
