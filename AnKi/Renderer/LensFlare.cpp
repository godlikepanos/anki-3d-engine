// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error LensFlare::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize lens flare pass");
	}

	return err;
}

Error LensFlare::initInternal()
{
	m_maxSpritesPerFlare = g_lensFlareMaxSpritesPerFlareCVar;
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/LensFlareSprite.ankiprogbin", m_realProg, m_realGrProg));

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/LensFlareUpdateIndirectInfo.ankiprogbin", m_updateIndirectBuffProg, m_updateIndirectBuffGrProg));

	return Error::kNone;
}

void LensFlare::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(LensFlare);
	const U32 flareCount = SceneGraph::getSingleton().getComponentArrays().getLensFlares().getSize();
	if(flareCount == 0)
	{
		m_runCtx = {};
		return;
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create indirect buffer
	m_runCtx.m_indirectBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<DrawIndirectArgs>(flareCount);
	m_runCtx.m_indirectBuffHandle = rgraph.importBuffer(m_runCtx.m_indirectBuff, BufferUsageBit::kNone);

	// Create the pass
	NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Lens flare indirect");

	rpass.newBufferDependency(m_runCtx.m_indirectBuffHandle, BufferUsageBit::kUavCompute);
	rpass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSrvCompute, DepthDownscale::kEighthInternalResolution);

	rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(LensFlare);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		const U32 flareCount = SceneGraph::getSingleton().getComponentArrays().getLensFlares().getSize();
		ANKI_ASSERT(flareCount > 0);

		cmdb.bindShaderProgram(m_updateIndirectBuffGrProg.get());

		cmdb.setFastConstants(&ctx.m_matrices.m_viewProjectionJitter, sizeof(ctx.m_matrices.m_viewProjectionJitter));

		// Write flare info
		WeakArray<Vec4> flarePositions = allocateAndBindSrvStructuredBuffer<Vec4>(cmdb, 0, 0, flareCount);
		U32 count = 0;
		for(const LensFlareComponent& comp : SceneGraph::getSingleton().getComponentArrays().getLensFlares())
		{
			flarePositions[count++] = Vec4(comp.getWorldPosition(), 1.0f);
		}

		rgraphCtx.bindUav(0, 0, m_runCtx.m_indirectBuffHandle);
		// Bind neareset because you don't need high quality
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		rgraphCtx.bindSrv(1, 0, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kEighthInternalResolution);

		cmdb.dispatchCompute(flareCount, 1, 1);
	});
}

void LensFlare::runDrawFlares(const RenderingContext& ctx, CommandBuffer& cmdb)
{
	const U32 flareCount = SceneGraph::getSingleton().getComponentArrays().getLensFlares().getSize();

	if(flareCount == 0)
	{
		return;
	}

	cmdb.bindShaderProgram(m_realGrProg.get());
	cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb.setDepthWrite(false);

	U32 count = 0;
	for(const LensFlareComponent& comp : SceneGraph::getSingleton().getComponentArrays().getLensFlares())
	{
		// Compute position
		Vec4 lfPos = Vec4(comp.getWorldPosition(), 1.0f);
		Vec4 posClip = ctx.m_matrices.m_viewProjectionJitter * lfPos;

		/*if(posClip.x() > posClip.w() || posClip.x() < -posClip.w() || posClip.y() > posClip.w()
			|| posClip.y() < -posClip.w())
		{
			// Outside clip
			ANKI_ASSERT(0 && "Check that before");
		}*/

		U32 c = 0;
		U32 spritesCount = max<U32>(1, m_maxSpritesPerFlare);

		// Get uniform memory
		WeakArray<LensFlareSprite> sprites = allocateAndBindSrvStructuredBuffer<LensFlareSprite>(cmdb, 0, 0, spritesCount);

		// misc
		Vec2 posNdc = posClip.xy() / posClip.w();

		// First flare
		sprites[c].m_posScale = Vec4(posNdc, comp.getFirstFlareSize() * Vec2(1.0f, getRenderer().getAspectRatio()));
		sprites[c].m_depthPad3 = Vec4(0.0f);
		const F32 alpha = comp.getColorMultiplier().w() * (1.0f - pow(absolute(posNdc.x()), 6.0f))
						  * (1.0f - pow(absolute(posNdc.y()), 6.0f)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(comp.getColorMultiplier().xyz(), alpha);
		++c;

		// Render
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
		cmdb.bindSrv(1, 0, TextureView(&comp.getImage().getTexture(), TextureSubresourceDesc::all()));

		cmdb.drawIndirect(PrimitiveTopology::kTriangleStrip, BufferView(m_runCtx.m_indirectBuff).incrementOffset(count * sizeof(DrawIndirectArgs)));

		++count;
	}

	// Restore state
	cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb.setDepthWrite(true);
}

} // end namespace anki
