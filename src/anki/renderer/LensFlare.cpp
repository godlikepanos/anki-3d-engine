// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LensFlare.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/Renderer.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/Functions.h>
#include <shaders/glsl_cpp_common/LensFlareSprite.h>

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
	m_maxSpritesPerFlare = config.getNumber("r.lensFlare.maxSpritesPerFlare");
	m_maxFlares = config.getNumber("r.lensFlare.maxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_R_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return Error::USER_DATA;
	}

	m_maxSprites = m_maxSpritesPerFlare * m_maxFlares;

	// Load prog
	ANKI_CHECK(getResourceManager().loadResource("shaders/LensFlareSprite.glslp", m_realProg));

	ShaderProgramResourceConstantValueInitList<1> consts(m_realProg);
	consts.add("MAX_SPRITES", U32(m_maxSprites));
	const ShaderProgramResourceVariant* variant;
	m_realProg->getOrCreateVariant(consts.get(), variant);
	m_realGrProg = variant->getProgram();

	return Error::NONE;
}

Error LensFlare::initOcclusion(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_indirectBuff = gr.newBuffer(BufferInitInfo(m_maxFlares * sizeof(DrawArraysIndirectInfo),
		BufferUsageBit::INDIRECT_GRAPHICS | BufferUsageBit::STORAGE_COMPUTE_WRITE,
		BufferMapAccessBit::NONE,
		"LensFlares"));

	ANKI_CHECK(
		getResourceManager().loadResource("shaders/LensFlareUpdateIndirectInfo.glslp", m_updateIndirectBuffProg));

	ShaderProgramResourceConstantValueInitList<1> consts(m_updateIndirectBuffProg);
	consts.add("IN_DEPTH_MAP_SIZE", Vec2(m_r->getWidth() / 2 / 2, m_r->getHeight() / 2 / 2));

	const ShaderProgramResourceVariant* variant;
	m_updateIndirectBuffProg->getOrCreateVariant(consts.get(), variant);
	m_updateIndirectBuffGrProg = variant->getProgram();

	return Error::NONE;
}

void LensFlare::updateIndirectInfo(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	U count = min<U>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);
	ANKI_ASSERT(count > 0);

	cmdb->bindShaderProgram(m_updateIndirectBuffGrProg);

	// Write flare info
	Vec4* flarePositions = allocateAndBindStorage<Vec4*>(sizeof(Mat4) + count * sizeof(Vec4), cmdb, 0, 0);
	*reinterpret_cast<Mat4*>(flarePositions) = ctx.m_matrices.m_viewProjectionJitter;
	flarePositions += 4;

	for(U i = 0; i < count; ++i)
	{
		*flarePositions = Vec4(ctx.m_renderQueue->m_lensFlares[i].m_worldPosition, 1.0f);
		++flarePositions;
	}

	rgraphCtx.bindStorageBuffer(0, 1, m_runCtx.m_indirectBuffHandle);
	// Bind neareset because you don't need high quality
	rgraphCtx.bindTextureAndSampler(
		0, 0, m_r->getDepthDownscale().getHiZRt(), HIZ_QUARTER_DEPTH, m_r->getNearestSampler());
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

		rpass.setWork(runUpdateIndirectCallback, this, 0);

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

	const U count = min<U>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);

	cmdb->bindShaderProgram(m_realGrProg);
	cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);
	cmdb->setDepthWrite(false);

	for(U i = 0; i < count; ++i)
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

		U c = 0;
		U spritesCount = max<U>(1, m_maxSpritesPerFlare);

		// Get uniform memory
		LensFlareSprite* tmpSprites =
			allocateAndBindUniforms<LensFlareSprite*>(spritesCount * sizeof(LensFlareSprite), cmdb, 0, 0);
		WeakArray<LensFlareSprite> sprites(tmpSprites, spritesCount);

		// misc
		Vec2 posNdc = posClip.xy() / posClip.w();

		// First flare
		sprites[c].m_posScale = Vec4(posNdc, flareEl.m_firstFlareSize * Vec2(1.0f, m_r->getAspectRatio()));
		sprites[c].m_depthPad3 = Vec4(0.0f);
		const F32 alpha = flareEl.m_colorMultiplier.w() * (1.0 - pow(absolute(posNdc.x()), 6.0f))
						  * (1.0 - pow(absolute(posNdc.y()), 6.0)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(flareEl.m_colorMultiplier.xyz(), alpha);
		++c;

		// Render
		ANKI_ASSERT(flareEl.m_textureView);
		cmdb->bindTextureAndSampler(0,
			0,
			TextureViewPtr(const_cast<TextureView*>(flareEl.m_textureView)),
			m_r->getTrilinearRepeatSampler(),
			TextureUsageBit::SAMPLED_FRAGMENT);

		cmdb->drawArraysIndirect(
			PrimitiveTopology::TRIANGLE_STRIP, 1, i * sizeof(DrawArraysIndirectInfo), m_indirectBuff);
	}

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setDepthWrite(true);
}

} // end namespace anki
