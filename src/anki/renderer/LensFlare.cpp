// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/LensFlare.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/Renderer.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/Functions.h>

namespace anki
{

class Sprite
{
public:
	Vec2 m_pos; ///< Position in NDC
	Vec2 m_scale; ///< Scale of the quad
	Vec4 m_color;
	F32 m_depth; ///< Texture depth
	U32 m_padding[3];
};

LensFlare::~LensFlare()
{
	m_queries.destroy(getAllocator());
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

	return ErrorCode::NONE;
}

Error LensFlare::initSprite(const ConfigSet& config)
{
	m_maxSpritesPerFlare = config.getNumber("r.lensFlare.maxSpritesPerFlare");
	m_maxFlares = config.getNumber("r.lensFlare.maxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_R_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return ErrorCode::USER_DATA;
	}

	m_maxSprites = m_maxSpritesPerFlare * m_maxFlares;

	// Load prog
	ANKI_CHECK(getResourceManager().loadResource("programs/LensFlareSprite.ankiprog", m_realProg));

	ShaderProgramResourceConstantValueInitList<1> consts(m_realProg);
	consts.add("MAX_SPRITES", U32(m_maxSprites));
	const ShaderProgramResourceVariant* variant;
	m_realProg->getOrCreateVariant(consts.get(), variant);
	m_realGrProg = variant->getProgram();

	return ErrorCode::NONE;
}

Error LensFlare::initOcclusion(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_queries.create(getAllocator(), m_maxFlares);

	m_queryResultBuff = gr.newInstance<Buffer>(m_maxFlares * sizeof(U32),
		BufferUsageBit::STORAGE_COMPUTE_READ | BufferUsageBit::QUERY_RESULT,
		BufferMapAccessBit::NONE);

	m_indirectBuff = gr.newInstance<Buffer>(m_maxFlares * sizeof(DrawArraysIndirectInfo),
		BufferUsageBit::INDIRECT | BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	ANKI_CHECK(getResourceManager().loadResource("programs/LensFlareOcclusionTest.ankiprog", m_occlusionProg));
	const ShaderProgramResourceVariant* variant;
	m_occlusionProg->getOrCreateVariant(variant);
	m_occlusionGrProg = variant->getProgram();

	ANKI_CHECK(
		getResourceManager().loadResource("programs/LensFlareUpdateIndirectInfo.ankiprog", m_updateIndirectBuffProg));
	m_updateIndirectBuffProg->getOrCreateVariant(variant);
	m_updateIndirectBuffGrProg = variant->getProgram();

	return ErrorCode::NONE;
}

void LensFlare::resetOcclusionQueries(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	if(ctx.m_renderQueue->m_lensFlares.getSize() > m_maxFlares)
	{
		ANKI_R_LOGW("Visible flares exceed the limit. Increase lf.maxFlares");
	}

	const U count = min<U>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);
	for(U i = 0; i < count; ++i)
	{
		if(!m_queries[i])
		{
			m_queries[i] = getGrManager().newInstance<OcclusionQuery>();
		}

		cmdb->resetOcclusionQuery(m_queries[i]);
	}
}

void LensFlare::runOcclusionTests(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	const U count = min<U>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);
	Vec3* positions = nullptr;
	const Vec3* initialPositions;
	if(count)
	{
		// Setup MVP UBO
		Mat4* mvp = allocateAndBindUniforms<Mat4*>(sizeof(Mat4), cmdb, 0, 0);
		*mvp = ctx.m_renderQueue->m_viewProjectionMatrix;

		// Alloc dyn mem
		StagingGpuMemoryToken vertToken;
		positions = static_cast<Vec3*>(m_r->getStagingGpuMemoryManager().allocateFrame(
			sizeof(Vec3) * count, StagingGpuMemoryType::VERTEX, vertToken));
		initialPositions = positions;

		cmdb->bindVertexBuffer(0, vertToken.m_buffer, vertToken.m_offset, sizeof(Vec3));

		// Setup state
		cmdb->bindShaderProgram(m_occlusionGrProg);
		cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);
		cmdb->setColorChannelWriteMask(0, ColorBit::NONE);
		cmdb->setColorChannelWriteMask(1, ColorBit::NONE);
		cmdb->setColorChannelWriteMask(2, ColorBit::NONE);
		cmdb->setDepthWrite(false);
	}

	for(U i = 0; i < count; ++i)
	{
		*positions = ctx.m_renderQueue->m_lensFlares[i].m_worldPosition;

		// Draw and query
		cmdb->beginOcclusionQuery(m_queries[i]);
		cmdb->drawArrays(PrimitiveTopology::POINTS, 1, 1, positions - initialPositions);
		cmdb->endOcclusionQuery(m_queries[i]);

		++positions;
	}

	// Restore state
	if(count)
	{
		cmdb->setColorChannelWriteMask(0, ColorBit::ALL);
		cmdb->setColorChannelWriteMask(1, ColorBit::ALL);
		cmdb->setColorChannelWriteMask(2, ColorBit::ALL);
		cmdb->setDepthWrite(true);
	}
}

void LensFlare::updateIndirectInfo(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	U count = min<U>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);
	if(count == 0)
	{
		return;
	}

	// Write results to buffer
	for(U i = 0; i < count; ++i)
	{
		cmdb->writeOcclusionQueryResultToBuffer(m_queries[i], sizeof(U32) * i, m_queryResultBuff);
	}

	// Set barrier
	cmdb->setBufferBarrier(m_queryResultBuff,
		BufferUsageBit::QUERY_RESULT,
		BufferUsageBit::STORAGE_COMPUTE_READ,
		0,
		sizeof(DrawArraysIndirectInfo) * count);

	// Update the indirect info
	cmdb->bindShaderProgram(m_updateIndirectBuffGrProg);
	cmdb->bindStorageBuffer(0, 0, m_queryResultBuff, 0, MAX_PTR_SIZE);
	cmdb->bindStorageBuffer(0, 1, m_indirectBuff, 0, MAX_PTR_SIZE);
	cmdb->dispatchCompute(count, 1, 1);

	// Set barrier
	cmdb->setBufferBarrier(m_indirectBuff,
		BufferUsageBit::STORAGE_COMPUTE_WRITE,
		BufferUsageBit::INDIRECT,
		0,
		sizeof(DrawArraysIndirectInfo) * count);
}

void LensFlare::run(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	const U count = min<U>(ctx.m_renderQueue->m_lensFlares.getSize(), m_maxFlares);
	if(count == 0)
	{
		return;
	}

	cmdb->bindShaderProgram(m_realGrProg);
	cmdb->setDepthWrite(false);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	cmdb->setBlendFactors(
		0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendFactor::DST_ALPHA, BlendFactor::ONE);
	cmdb->setBlendOperation(0, BlendOperation::ADD, BlendOperation::REVERSE_SUBTRACT);

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
		Sprite* tmpSprites = allocateAndBindUniforms<Sprite*>(spritesCount * sizeof(Sprite), cmdb, 0, 0);
		WeakArray<Sprite> sprites(tmpSprites, spritesCount);

		// misc
		Vec2 posNdc = posClip.xy() / posClip.w();

		// First flare
		sprites[c].m_pos = posNdc;
		sprites[c].m_scale = flareEl.m_firstFlareSize * Vec2(1.0, m_r->getAspectRatio());
		sprites[c].m_depth = 0.0;
		F32 alpha = flareEl.m_colorMultiplier.w() * (1.0 - pow(absolute(posNdc.x()), 6.0))
			* (1.0 - pow(absolute(posNdc.y()), 6.0)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(flareEl.m_colorMultiplier.xyz(), alpha);
		++c;

		// Render
		ANKI_ASSERT(flareEl.m_texture);
		cmdb->bindTexture(0, 0, TexturePtr(flareEl.m_texture));

		cmdb->drawArraysIndirect(
			PrimitiveTopology::TRIANGLE_STRIP, 1, i * sizeof(DrawArraysIndirectInfo), m_indirectBuff);
	}

	// Restore state
	cmdb->setDepthWrite(true);
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setBlendOperation(0, BlendOperation::ADD);
}

} // end namespace anki
