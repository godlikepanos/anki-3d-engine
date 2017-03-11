// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Lf.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Renderer.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/FrustumComponent.h>
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

Lf::~Lf()
{
	m_queries.destroy(getAllocator());
}

Error Lf::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing lens flare pass");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize lens flare pass");
	}

	return err;
}

Error Lf::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(initSprite(config));
	ANKI_CHECK(initOcclusion(config));

	return ErrorCode::NONE;
}

Error Lf::initSprite(const ConfigSet& config)
{
	m_maxSpritesPerFlare = config.getNumber("lf.maxSpritesPerFlare");
	m_maxFlares = config.getNumber("lf.maxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_R_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return ErrorCode::USER_DATA;
	}

	m_maxSprites = m_maxSpritesPerFlare * m_maxFlares;

	// Load shaders
	StringAuto pps(getAllocator());

	pps.sprintf("#define MAX_SPRITES %u\n", m_maxSprites);

	ANKI_CHECK(m_r->createShader("shaders/LfSpritePass.vert.glsl", m_realVert, pps.toCString()));

	ANKI_CHECK(m_r->createShader("shaders/LfSpritePass.frag.glsl", m_realFrag, pps.toCString()));

	// Create prog.
	m_realProg = getGrManager().newInstance<ShaderProgram>(m_realVert->getGrShader(), m_realFrag->getGrShader());

	return ErrorCode::NONE;
}

Error Lf::initOcclusion(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_queries.create(getAllocator(), m_maxFlares);

	m_queryResultBuff = gr.newInstance<Buffer>(m_maxFlares * sizeof(U32),
		BufferUsageBit::STORAGE_COMPUTE_READ | BufferUsageBit::QUERY_RESULT,
		BufferMapAccessBit::NONE);

	m_indirectBuff = gr.newInstance<Buffer>(m_maxFlares * sizeof(DrawArraysIndirectInfo),
		BufferUsageBit::INDIRECT | BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	ANKI_CHECK(getResourceManager().loadResource("shaders/LfOcclusion.vert.glsl", m_occlusionVert));
	ANKI_CHECK(getResourceManager().loadResource("shaders/LfOcclusion.frag.glsl", m_occlusionFrag));
	ANKI_CHECK(getResourceManager().loadResource("shaders/LfUpdateIndirectInfo.comp.glsl", m_writeIndirectBuffComp));

	m_updateIndirectBuffProg = gr.newInstance<ShaderProgram>(m_writeIndirectBuffComp->getGrShader());

	m_occlusionProg = gr.newInstance<ShaderProgram>(m_occlusionVert->getGrShader(), m_occlusionFrag->getGrShader());

	return ErrorCode::NONE;
}

void Lf::resetOcclusionQueries(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	if(ctx.m_visResults->getCount(VisibilityGroupType::FLARES) > m_maxFlares)
	{
		ANKI_R_LOGW("Visible flares exceed the limit. Increase lf.maxFlares");
	}

	const U count = min<U>(ctx.m_visResults->getCount(VisibilityGroupType::FLARES), m_maxFlares);
	for(U i = 0; i < count; ++i)
	{
		if(!m_queries[i])
		{
			m_queries[i] = getGrManager().newInstance<OcclusionQuery>();
		}

		cmdb->resetOcclusionQuery(m_queries[i]);
	}
}

void Lf::runOcclusionTests(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	const U count = min<U>(ctx.m_visResults->getCount(VisibilityGroupType::FLARES), m_maxFlares);
	Vec3* positions = nullptr;
	const Vec3* initialPositions;
	if(count)
	{
		// Setup MVP UBO
		Mat4* mvp = allocateAndBindUniforms<Mat4*>(sizeof(Mat4), cmdb, 0, 0);
		*mvp = ctx.m_viewProjMat;

		// Alloc dyn mem
		StagingGpuMemoryToken vertToken;
		positions = static_cast<Vec3*>(m_r->getStagingGpuMemoryManager().allocateFrame(
			sizeof(Vec3) * count, StagingGpuMemoryType::VERTEX, vertToken));
		initialPositions = positions;

		cmdb->bindVertexBuffer(0, vertToken.m_buffer, vertToken.m_offset, sizeof(Vec3));

		// Setup state
		cmdb->bindShaderProgram(m_occlusionProg);
		cmdb->setVertexAttribute(0, 0, PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT), 0);
		cmdb->setColorChannelWriteMask(0, ColorBit::NONE);
		cmdb->setColorChannelWriteMask(1, ColorBit::NONE);
		cmdb->setColorChannelWriteMask(2, ColorBit::NONE);
		cmdb->setDepthWrite(false);
	}

	for(U i = 0; i < count; ++i)
	{
		// Iterate lens flare
		auto it = ctx.m_visResults->getBegin(VisibilityGroupType::FLARES) + i;
		const LensFlareComponent& lf = (it->m_node)->getComponent<LensFlareComponent>();

		*positions = lf.getWorldPosition().xyz();

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

void Lf::updateIndirectInfo(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	U count = min<U>(ctx.m_visResults->getCount(VisibilityGroupType::FLARES), m_maxFlares);
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
	cmdb->bindShaderProgram(m_updateIndirectBuffProg);
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

void Lf::run(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	const U count = min<U>(ctx.m_visResults->getCount(VisibilityGroupType::FLARES), m_maxFlares);
	if(count == 0)
	{
		return;
	}

	cmdb->bindShaderProgram(m_realProg);
	cmdb->setDepthWrite(false);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	cmdb->setBlendFactors(
		0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendFactor::DST_ALPHA, BlendFactor::ONE);
	cmdb->setBlendOperation(0, BlendOperation::ADD, BlendOperation::REVERSE_SUBTRACT);

	for(U i = 0; i < count; ++i)
	{
		auto it = ctx.m_visResults->getBegin(VisibilityGroupType::FLARES) + i;
		const LensFlareComponent& lf = (it->m_node)->getComponent<LensFlareComponent>();

		// Compute position
		Vec4 lfPos = Vec4(lf.getWorldPosition().xyz(), 1.0);
		Vec4 posClip = ctx.m_viewProjMat * lfPos;

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
		sprites[c].m_scale = lf.getFirstFlareSize() * Vec2(1.0, m_r->getAspectRatio());
		sprites[c].m_depth = 0.0;
		F32 alpha = lf.getColorMultiplier().w() * (1.0 - pow(absolute(posNdc.x()), 6.0))
			* (1.0 - pow(absolute(posNdc.y()), 6.0)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(lf.getColorMultiplier().xyz(), alpha);
		++c;

		// Render
		cmdb->bindTexture(0, 0, lf.getTexture());

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
