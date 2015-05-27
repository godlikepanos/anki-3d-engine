// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Lf.h"
#include "anki/renderer/Bloom.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/Camera.h"
#include "anki/misc/ConfigSet.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct Sprite
{
	Vec2 m_pos; ///< Position in NDC
	Vec2 m_scale; ///< Scale of the quad
	F32 m_alpha; ///< Alpha value
	F32 m_depth; ///< Texture depth
	U32 m_padding[2];
};

//==============================================================================
// Lf                                                                          =
//==============================================================================

//==============================================================================
Lf::~Lf()
{}

//==============================================================================
Error Lf::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init lens flare pass");
	}

	return err;
}

//==============================================================================
Error Lf::initSprite(const ConfigSet& config)
{
	m_maxSpritesPerFlare = config.get("lf.maxSpritesPerFlare");
	m_maxFlares = config.get("lf.maxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return ErrorCode::USER_DATA;
	}

	// Load program + ppline
	StringAuto pps(getAllocator());

	pps.sprintf("#define MAX_SPRITES %u\n", m_maxSpritesPerFlare);

	ANKI_CHECK(m_realVert.loadToCache(&getResourceManager(),
		"shaders/LfSpritePass.vert.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_realFrag.loadToCache(&getResourceManager(),
		"shaders/LfSpritePass.frag.glsl", pps.toCString(), "r_"));

	PipelinePtr::Initializer init;
	init.m_shaders[U(ShaderType::VERTEX)] = m_realVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = m_realFrag->getGrShader();
	m_realPpline.create(&getGrManager(), init);

	// Create buffer
	PtrSize uboAlignment =
		m_r->getGrManager().getBufferOffsetAlignment(GL_UNIFORM_BUFFER);
	m_flareSize = getAlignedRoundUp(
		uboAlignment, sizeof(Sprite) * m_maxSpritesPerFlare);
	PtrSize blockSize = m_flareSize * m_maxFlares;

	for(U i = 0; i < m_flareDataBuff.getSize(); ++i)
	{
		m_flareDataBuff[i].create(
			&getGrManager(), GL_UNIFORM_BUFFER, nullptr, blockSize,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Lf::initOcclusion(const ConfigSet& config)
{
	// Init vert buff
	U buffSize = sizeof(Vec3) * m_maxFlares;

	for(U i = 0; i < m_positionsVertBuff.getSize(); ++i)
	{
		m_positionsVertBuff[i].create(
			&getGrManager(), GL_ARRAY_BUFFER, nullptr, buffSize,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

	// Init MVP buff
	m_mvpBuff.create(&getGrManager(), GL_UNIFORM_BUFFER,
		nullptr, sizeof(Mat4), GL_DYNAMIC_STORAGE_BIT);

	// Shaders
	ANKI_CHECK(m_occlusionVert.load("shaders/LfOcclusion.vert.glsl",
		&getResourceManager()));

	ANKI_CHECK(m_occlusionFrag.load("shaders/LfOcclusion.frag.glsl",
		&getResourceManager()));

	PipelinePtr::Initializer init;
		init.m_shaders[U(ShaderType::VERTEX)] = m_occlusionVert->getGrShader();
		init.m_shaders[U(ShaderType::FRAGMENT)] =
			m_occlusionFrag->getGrShader();
	m_occlusionPpline.create(&getGrManager(), init);

	return ErrorCode::NONE;
}

//==============================================================================
Error Lf::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(initSprite(config));
	ANKI_CHECK(initOcclusion(config));

	return ErrorCode::NONE;
}

//==============================================================================
void Lf::runOcclusionTests(CommandBufferPtr& cmdb)
{
	// Retrieve some things
	FrustumComponent& camFr =
		m_r->getActiveCamera().getComponent<FrustumComponent>();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		if(vi.getLensFlaresCount() > m_maxFlares)
		{
			ANKI_LOGW("Visible flares exceed the limit");
		}

		// Setup state
		cmdb.setColorWriteMask(false, false, false, false);
		cmdb.enableDepthTest(true);
		m_occlusionPpline.bind(cmdb);

		// Setup MVP UBO
		Mat4 mvp = camFr.getViewProjectionMatrix().getTransposed();
		m_mvpBuff.write(cmdb, &mvp, sizeof(mvp), 0, 0, sizeof(mvp));
		m_mvpBuff.bindShaderBuffer(cmdb, 0, sizeof(Mat4), 0);

		// Allocate vertices and fire write job
		BufferPtr& positionsVertBuff = m_positionsVertBuff[
			getGlobalTimestamp() % m_positionsVertBuff.getSize()];
		ANKI_ASSERT(sizeof(Vec3) * totalCount <= positionsVertBuff.getSize());

		positionsVertBuff.bindVertexBuffer(
			cmdb, 3, GL_FLOAT, false, sizeof(Vec3), 0, 0);

		Vec3* positions = static_cast<Vec3*>(
			positionsVertBuff.getPersistentMappingAddress());
		Vec3* initialPositions = positions;

		// Iterate lens flare
		auto it = vi.getLensFlaresBegin();
		auto end = vi.getLensFlaresBegin() + totalCount;
		for(; it != end; ++it)
		{
			LensFlareComponent& lf =
				(it->m_node)->getComponent<LensFlareComponent>();

			*positions = lf.getWorldPosition().xyz();

			// Draw and query
			OcclusionQueryPtr& query = lf.getOcclusionQueryToTest();
			query.begin(cmdb);

			cmdb.drawArrays(GL_POINTS, 1, 1, positions - initialPositions);

			query.end(cmdb);

			++positions;
		}

		ANKI_ASSERT(positions == initialPositions + totalCount);

		// Restore state
		cmdb.setColorWriteMask(true, true, true, true);
		cmdb.enableDepthTest(false);
	}
}

//==============================================================================
void Lf::run(CommandBufferPtr& cmdb)
{
	// Retrieve some things
	SceneNode& cam = m_r->getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		// Allocate client buffer
		const U uboAlignment =
			m_r->getGrManager().getBufferOffsetAlignment(GL_UNIFORM_BUFFER);
		const U bufferSize = m_flareSize * totalCount;

		// Set common rendering state
		m_realPpline.bind(cmdb);
		cmdb.enableBlend(true);
		cmdb.setBlendFunctions(GL_ONE, GL_ONE);

		// Send the command to write the buffer now
		BufferPtr& flareDataBuff = m_flareDataBuff[
			getGlobalTimestamp() % m_flareDataBuff.getSize()];

		Sprite* sprites = static_cast<Sprite*>(
			flareDataBuff.getPersistentMappingAddress());
		U8* spritesInitialPtr = reinterpret_cast<U8*>(sprites);

		// Iterate lens flare
		auto it = vi.getLensFlaresBegin();
		auto end = vi.getLensFlaresBegin() + totalCount;
		for(; it != end; ++it)
		{
			LensFlareComponent& lf =
				(it->m_node)->getComponent<LensFlareComponent>();
			U count = 0;

			// Compute position
			Vec4 lfPos = Vec4(lf.getWorldPosition().xyz(), 1.0);
			Vec4 posClip = camFr.getViewProjectionMatrix() * lfPos;

			if(posClip.x() > posClip.w() || posClip.x() < -posClip.w()
				|| posClip.y() > posClip.w() || posClip.y() < -posClip.w())
			{
				// Outside clip
				continue;
			}

			Vec2 posNdc = posClip.xy() / posClip.w();

			Vec2 dir = -posNdc;
			F32 len = dir.getLength();
			dir /= len; // Normalize dir

			// First flare
			sprites[count].m_pos = posNdc;
			sprites[count].m_scale =
				lf.getFirstFlareSize() * Vec2(1.0, m_r->getAspectRatio());
			sprites[count].m_depth = 0.0;
			sprites[count].m_alpha = lf.getColorMultiplier().w();
			++count;

			// Render
			lf.getTexture().bind(cmdb, 0);
			flareDataBuff.bindShaderBuffer(
				cmdb,
				reinterpret_cast<U8*>(sprites) - spritesInitialPtr,
				sizeof(Sprite) * count,
				0);

			OcclusionQueryPtr query;
			Bool queryInvalid;
			lf.getOcclusionQueryToCheck(query, queryInvalid);

			if(!queryInvalid)
			{
				m_r->drawQuadConditional(query, cmdb);
			}
			else
			{
				// Skip the drawcall. If the flare appeared suddenly inside the
				// view we don't want to draw it.
			}

			ANKI_ASSERT(count <= m_maxSpritesPerFlare);

			// Advance
			U advancementSize =
				getAlignedRoundUp(uboAlignment, sizeof(Sprite) * count);
			sprites = reinterpret_cast<Sprite*>(
				reinterpret_cast<U8*>(sprites) + advancementSize);

		}

		ANKI_ASSERT(
			reinterpret_cast<U8*>(sprites) <= spritesInitialPtr + bufferSize);

		cmdb.enableBlend(false);
	}
}

} // end namespace anki
