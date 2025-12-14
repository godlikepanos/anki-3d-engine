// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionBlur.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error MotionBlur::init()
{
	ANKI_CHECK(loadShaderProgram(
		"ShaderBinaries/MotionBlur.ankiprogbin",
		{{"TILE_SIZE", MutatorValue(g_cvarRenderMotionBlurTileSize)}, {"SAMPLE_COUNT", MutatorValue(g_cvarRenderMotionBlurSampleCount)}}, m_prog,
		m_maxVelocityGrProg, "MaxTileVelocity"));
	ANKI_CHECK(loadShaderProgram(
		"ShaderBinaries/MotionBlur.ankiprogbin",
		{{"TILE_SIZE", MutatorValue(g_cvarRenderMotionBlurTileSize)}, {"SAMPLE_COUNT", MutatorValue(g_cvarRenderMotionBlurSampleCount)}}, m_prog,
		m_maxNeightbourVelocityGrProg, "MaxNeighbourTileVelocity"));
	ANKI_CHECK(loadShaderProgram(
		"ShaderBinaries/MotionBlur.ankiprogbin",
		{{"TILE_SIZE", MutatorValue(g_cvarRenderMotionBlurTileSize)}, {"SAMPLE_COUNT", MutatorValue(g_cvarRenderMotionBlurSampleCount)}}, m_prog,
		m_reconstructGrProg, "Reconstruct"));

	const UVec2 tiledTexSize = (getRenderer().getPostProcessResolution() + g_cvarRenderMotionBlurTileSize - 1) / g_cvarRenderMotionBlurTileSize;
	m_maxVelocityRtDesc = getRenderer().create2DRenderTargetDescription(tiledTexSize.x, tiledTexSize.y, Format::kR16G16_Sfloat, "MaxTileVelocity");
	m_maxVelocityRtDesc.bake();

	m_maxNeighbourVelocityRtDesc =
		getRenderer().create2DRenderTargetDescription(tiledTexSize.x, tiledTexSize.y, Format::kR16G16_Sfloat, "MaxNeighbourTileVelocity");
	m_maxNeighbourVelocityRtDesc.bake();

	m_finalRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y,
		(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm : Format::kR8G8B8A8_Unorm,
		"MotionBlur");
	m_finalRtDesc.bake();

	return Error::kNone;
}

void MotionBlur::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(MotionBlur);
	if(g_cvarRenderMotionBlurSampleCount == 0)
	{
		m_runCtx.m_rt = {};
		return;
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// MaxTileVelocity
	const RenderTargetHandle maxVelRt = rgraph.newRenderTarget(m_maxVelocityRtDesc);
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Motion blur min velocity");

		pass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(maxVelRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, maxVelRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(MotionBlurMinTileVelocity);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_maxVelocityGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, getRenderer().getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindUav(0, 0, maxVelRt);

			const Vec4 consts(Vec2(getRenderer().getPostProcessResolution()), 0.0f, 0.0f);
			cmdb.setFastConstants(&consts, sizeof(consts));

			const UVec2 tiledTexSize =
				(getRenderer().getPostProcessResolution() + g_cvarRenderMotionBlurTileSize - 1) / g_cvarRenderMotionBlurTileSize;
			cmdb.dispatchCompute(tiledTexSize.x, tiledTexSize.y, 1);
		});
	}

	// MaxNeighbourTileVelocity
	const RenderTargetHandle maxNeighbourVelRt = rgraph.newRenderTarget(m_maxNeighbourVelocityRtDesc);
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Motion blur min neighbour vel");

		pass.newTextureDependency(maxVelRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(maxNeighbourVelRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, maxVelRt, maxNeighbourVelRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(MotionBlurMaxNeighbourTileVelocity);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_maxNeightbourVelocityGrProg.get());

			rgraphCtx.bindSrv(0, 0, maxVelRt);
			rgraphCtx.bindUav(0, 0, maxNeighbourVelRt);

			const UVec2 tiledTexSize =
				(getRenderer().getPostProcessResolution() + g_cvarRenderMotionBlurTileSize - 1) / g_cvarRenderMotionBlurTileSize;
			cmdb.dispatchCompute(tiledTexSize.x, tiledTexSize.y, 1);
		});
	}

	// Recustruct
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_finalRtDesc);

		TextureUsageBit readUsage, writeUsage;
		RenderPassBase* ppass;
		if(g_cvarRenderPreferCompute)
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Motion blur reconstruct");
			ppass = &pass;
			readUsage = TextureUsageBit::kSrvCompute;
			writeUsage = TextureUsageBit::kUavCompute;
		}
		else
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Motion blur reconstruct");
			pass.setRenderpassInfo({m_runCtx.m_rt});
			ppass = &pass;
			readUsage = TextureUsageBit::kSrvPixel;
			writeUsage = TextureUsageBit::kRtvDsvWrite;
		}

		ppass->newTextureDependency(getRenderer().getTonemapping().getRt(), readUsage);
		ppass->newTextureDependency(getGBuffer().getDepthRt(), readUsage);
		ppass->newTextureDependency(maxNeighbourVelRt, readUsage);
		ppass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);
		ppass->newTextureDependency(m_runCtx.m_rt, writeUsage);

		ppass->setWork([this, maxNeighbourVelRt, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(MotionBlurReconstruct);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_reconstructGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getTonemapping().getRt());
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 0, maxNeighbourVelRt);
			rgraphCtx.bindSrv(3, 0, getRenderer().getMotionVectors().getMotionVectorsRt());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			class Constants
			{
			public:
				Vec2 m_depthLinearizationParams;
				U32 m_frame;
				F32 m_far;
			} consts;
			consts.m_depthLinearizationParams.x = (ctx.m_matrices.m_near - ctx.m_matrices.m_far) / ctx.m_matrices.m_near;
			consts.m_depthLinearizationParams.y = ctx.m_matrices.m_far / ctx.m_matrices.m_near;
			consts.m_frame = getRenderer().getFrameCount() % 32;
			consts.m_far = ctx.m_matrices.m_far;
			cmdb.setFastConstants(&consts, sizeof(consts));

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, m_runCtx.m_rt);
				dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y);
			}
			else
			{
				cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y);
				drawQuad(cmdb);
			}
		});
	}
}

} // end namespace anki
