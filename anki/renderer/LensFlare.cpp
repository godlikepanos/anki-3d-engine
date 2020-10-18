// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LensFlare.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/Functions.h>
#include <anki/shaders/include/LensFlareTypes.h>

namespace anki
{

LensFlare::~LensFlare()
{
}

Error LensFlare::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing lens flare pass");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize lens flare pass");
	}

	return err;
}

Error LensFlare::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(initSprite(config));
	ANKI_CHECK(initOcclusion(config));

	return Error::NONE;
}

Error LensFlare::initSprite(const ConfigSet& config)
{
	m_maxSpritesPerFlare = config.getNumberU8("r_lensFlareMaxSpritesPerFlare");
	m_maxFlares = config.getNumberU8("r_lensFlareMaxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_R_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return Error::USER_DATA;
	}

	m_maxSprites = U16(m_maxSpritesPerFlare * m_maxFlares);

	// Load prog
	ANKI_CHECK(getResourceManager().loadResource("shaders/LensFlareSprite.ankiprog", m_realProg));
	const ShaderProgramResourceVariant* variant;
	m_realProg->getOrCreateVariant(variant);
	m_realGrProg = variant->getProgram();

	return Error::NONE;
}

Error LensFlare::initOcclusion(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_indirectBuff = gr.newBuffer(BufferInitInfo(m_maxFlares * sizeof(DrawArraysIndirectInfo),
												 BufferUsageBit::INDIRECT_DRAW | BufferUsageBit::STORAGE_COMPUTE_WRITE,
												 BufferMapAccessBit::NONE, "LensFlares"));

	ANKI_CHECK(
		getResourceManager().loadResource("shaders/LensFlareUpdateIndirectInfo.ankiprog", m_updateIndirectBuffProg));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_updateIndirectBuffProg);
	variantInitInfo.addConstant("IN_DEPTH_MAP_SIZE", UVec2(m_r->getWidth() / 2 / 2, m_r->getHeight() / 2 / 2));
	const ShaderProgramResourceVariant* variant;
	m_updateIndirectBuffProg->getOrCreateVariant(variantInitInfo, variant);
	m_updateIndirectBuffGrProg = variant->getProgram();

	return Error::NONE;
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
	rgraphCtx.bindTexture(0, 3, m_r->getDepthDownscale().getHiZRt(), HIZ_QUARTER_DEPTH);
	cmdb->dispatchCompute(count, 1, 1);
}

void LensFlare::populateRenderGraph(RenderingContext& ctx)
{
	if(ctx.m_renderQueue->m_lensFlares.getSize() == 0)
	{
		return;
	}

	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import buffer
	m_runCtx.m_indirectBuffHandle = rgraph.importBuffer(m_indirectBuff, BufferUsageBit::NONE);

	// Update the indirect buffer
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("LF Upd Ind/ct");

		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				LensFlare* const self = static_cast<LensFlare*>(rgraphCtx.m_userData);
				self->updateIndirectInfo(*self->m_runCtx.m_ctx, rgraphCtx);
			},
			this, 0);

		rpass.newDependency({m_runCtx.m_indirectBuffHandle, BufferUsageBit::STORAGE_COMPUTE_WRITE});
		rpass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, HIZ_QUARTER_DEPTH});
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
	cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);
	cmdb->setDepthWrite(false);

	for(U32 i = 0; i < count; ++i)
	{
		const LensFlareQueueElement& flareEl = ctx.m_renderQueue->m_lensFlares[i];

		// Compute position
		Vec4 lfPos = Vec4(flareEl.m_worldPosition, 1.0);
		Vec4 posClip = ctx.m_renderQueue->m_viewProjectionMatrix * lfPos;

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
		cmdb->bindTexture(0, 2, TextureViewPtr(const_cast<TextureView*>(flareEl.m_textureView)),
						  TextureUsageBit::SAMPLED_FRAGMENT);

		cmdb->drawArraysIndirect(PrimitiveTopology::TRIANGLE_STRIP, 1, i * sizeof(DrawArraysIndirectInfo),
								 m_indirectBuff);
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setDepthWrite(true);
}

} // end namespace anki
