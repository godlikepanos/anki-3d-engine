// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Functions.h>

namespace anki {

LensFlare::~LensFlare()
{
}

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
	ANKI_R_LOGV("Initializing lens flare");

	ANKI_CHECK(initSprite());
	ANKI_CHECK(initOcclusion());

	return Error::kNone;
}

Error LensFlare::initSprite()
{
	m_maxSpritesPerFlare = getConfig().getRLensFlareMaxSpritesPerFlare();
	m_maxFlares = getConfig().getRLensFlareMaxFlares();

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_R_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return Error::kUserData;
	}

	m_maxSprites = U16(m_maxSpritesPerFlare * m_maxFlares);

	// Load prog
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/LensFlareSprite.ankiprogbin", m_realProg));
	const ShaderProgramResourceVariant* variant;
	m_realProg->getOrCreateVariant(variant);
	m_realGrProg = variant->getProgram();

	return Error::kNone;
}

Error LensFlare::initOcclusion()
{
	GrManager& gr = getGrManager();

	m_indirectBuff = gr.newBuffer(BufferInitInfo(m_maxFlares * sizeof(DrawArraysIndirectInfo),
												 BufferUsageBit::kIndirectDraw | BufferUsageBit::kStorageComputeWrite,
												 BufferMapAccessBit::kNone, "LensFlares"));

	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/LensFlareUpdateIndirectInfo.ankiprogbin",
												 m_updateIndirectBuffProg));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_updateIndirectBuffProg);
	variantInitInfo.addConstant(
		"kInDepthMapSize", UVec2(m_r->getInternalResolution().x() / 2 / 2, m_r->getInternalResolution().y() / 2 / 2));
	const ShaderProgramResourceVariant* variant;
	m_updateIndirectBuffProg->getOrCreateVariant(variantInitInfo, variant);
	m_updateIndirectBuffGrProg = variant->getProgram();

	return Error::kNone;
}

void LensFlare::updateIndirectInfo(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	U32 count = min<U32>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);
	ANKI_ASSERT(count > 0);

	cmdb->bindShaderProgram(m_updateIndirectBuffGrProg);

	// Write flare info
	Vec4* flarePositions = allocateAndBindStorage<Vec4*>(sizeof(Mat4) + count * sizeof(Vec4), cmdb, 0, 0);
	*reinterpret_cast<Mat4*>(flarePositions) = ctx.m_matrices.m_viewProjectionJitter;
	flarePositions += 4;

	for(U32 i = 0; i < count; ++i)
	{
		*flarePositions = Vec4(ctx.m_renderQueue->m_lensFlares[i].m_worldPosition, 1.0f);
		++flarePositions;
	}

	rgraphCtx.bindStorageBuffer(0, 1, m_runCtx.m_indirectBuffHandle);
	// Bind neareset because you don't need high quality
	cmdb->bindSampler(0, 2, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindTexture(0, 3, m_r->getDepthDownscale().getHiZRt(), kHiZQuarterSurface);
	cmdb->dispatchCompute(count, 1, 1);
}

void LensFlare::populateRenderGraph(RenderingContext& ctx)
{
	if(ctx.m_renderQueue->m_lensFlares.getSize() == 0)
	{
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import buffer
	m_runCtx.m_indirectBuffHandle = rgraph.importBuffer(m_indirectBuff, BufferUsageBit::kNone);

	// Update the indirect buffer
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("LF Upd Ind/ct");

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			updateIndirectInfo(ctx, rgraphCtx);
		});

		rpass.newBufferDependency(m_runCtx.m_indirectBuffHandle, BufferUsageBit::kStorageComputeWrite);
		rpass.newTextureDependency(m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::kSampledCompute,
								   kHiZQuarterSurface);
	}
}

void LensFlare::runDrawFlares(const RenderingContext& ctx, CommandBufferPtr& cmdb)
{
	if(ctx.m_renderQueue->m_lensFlares.getSize() == 0)
	{
		return;
	}

	const U32 count = min<U32>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);

	cmdb->bindShaderProgram(m_realGrProg);
	cmdb->setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb->setDepthWrite(false);

	for(U32 i = 0; i < count; ++i)
	{
		const LensFlareQueueElement& flareEl = ctx.m_renderQueue->m_lensFlares[i];

		// Compute position
		Vec4 lfPos = Vec4(flareEl.m_worldPosition, 1.0);
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
		LensFlareSprite* tmpSprites =
			allocateAndBindStorage<LensFlareSprite*>(spritesCount * sizeof(LensFlareSprite), cmdb, 0, 0);
		WeakArray<LensFlareSprite> sprites(tmpSprites, spritesCount);

		// misc
		Vec2 posNdc = posClip.xy() / posClip.w();

		// First flare
		sprites[c].m_posScale = Vec4(posNdc, flareEl.m_firstFlareSize * Vec2(1.0f, m_r->getAspectRatio()));
		sprites[c].m_depthPad3 = Vec4(0.0f);
		const F32 alpha = flareEl.m_colorMultiplier.w() * (1.0f - pow(absolute(posNdc.x()), 6.0f))
						  * (1.0f - pow(absolute(posNdc.y()), 6.0f)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(flareEl.m_colorMultiplier.xyz(), alpha);
		++c;

		// Render
		ANKI_ASSERT(flareEl.m_textureView);
		cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearRepeat);
		cmdb->bindTexture(0, 2, TextureViewPtr(const_cast<TextureView*>(flareEl.m_textureView)));

		cmdb->drawArraysIndirect(PrimitiveTopology::kTriangleStrip, 1, i * sizeof(DrawArraysIndirectInfo),
								 m_indirectBuff);
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb->setDepthWrite(true);
}

} // end namespace anki
