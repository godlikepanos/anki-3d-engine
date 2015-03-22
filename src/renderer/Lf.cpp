// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Lf.h"
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
Error Lf::initPseudo(const ConfigSet& config, 
	CommandBufferHandle& cmdBuff)
{
	Error err = ErrorCode::NONE;

	m_maxSpritesPerFlare = config.get("pps.lf.maxSpritesPerFlare");
	m_maxFlares = config.get("pps.lf.maxFlares");

	if(m_maxSpritesPerFlare < 1 || m_maxFlares < 1)
	{
		ANKI_LOGE("Incorrect m_maxSpritesPerFlare or m_maxFlares");
		return ErrorCode::USER_DATA;
	}

	// Load program 1
	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());

	err = pps.sprintf(getAllocator(), 
		"#define TEX_DIMENSIONS vec2(%u.0, %u.0)\n", 
		m_r->getPps().getHdr()._getWidth(),
		m_r->getPps().getHdr()._getHeight());
	if(err) return err;

	err = m_pseudoFrag.loadToCache(&getResourceManager(), 
		"shaders/PpsLfPseudoPass.frag.glsl", pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadPipeline(
		m_pseudoFrag->getGrShader(), m_pseudoPpline);
	if(err) return err;

	// Textures
	err = m_lensDirtTex.load(
		"engine_data/lens_dirt.ankitex", &getResourceManager());
	if(err) return err;

	return err;
}

//==============================================================================
Error Lf::initSprite(const ConfigSet& config, 
	CommandBufferHandle& cmdBuff)
{
	Error err = ErrorCode::NONE;

	// Load program + ppline
	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());

	err = pps.sprintf(getAllocator(), "#define MAX_SPRITES %u\n",
		m_maxSpritesPerFlare);
	if(err)
	{
		return err;
	}

	err = m_realVert.loadToCache(&getResourceManager(), 
		"shaders/PpsLfSpritePass.vert.glsl", pps.toCString(), "r_");
	if(err)
	{
		return err;
	}

	err = m_realFrag.loadToCache(&getResourceManager(), 
		"shaders/PpsLfSpritePass.frag.glsl", pps.toCString(), "r_");
	if(err)
	{
		return err;
	}

	err = m_realPpline.create(cmdBuff,
		{m_realVert->getGrShader(), m_realFrag->getGrShader()});
	if(err)
	{
		return err;
	}

	// Create buffer
	PtrSize uboAlignment = 
		m_r->_getGrManager().getBufferOffsetAlignment(GL_UNIFORM_BUFFER);
	m_flareSize = getAlignedRoundUp(
		uboAlignment, sizeof(Sprite) * m_maxSpritesPerFlare);
	PtrSize blockSize = m_flareSize * m_maxFlares;

	for(U i = 0; i < m_flareDataBuff.getSize(); ++i)
	{
		err = m_flareDataBuff[i].create(
			cmdBuff, GL_UNIFORM_BUFFER, nullptr, blockSize, 
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		if(err)
		{
			return err;
		}
	}

	return err;
}

//==============================================================================
Error Lf::initOcclusion(
	const ConfigSet& config, CommandBufferHandle& cmdBuff)
{
	Error err = ErrorCode::NONE;

	// Init vert buff
	U buffSize = sizeof(Vec3) * m_maxFlares;

	for(U i = 0; i < m_positionsVertBuff.getSize(); ++i)
	{
		err = m_positionsVertBuff[i].create(
			cmdBuff, GL_ARRAY_BUFFER, nullptr, buffSize, 
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		if(err)
		{
			return err;
		}
	}

	// Init MVP buff
	err = m_mvpBuff.create(cmdBuff, GL_UNIFORM_BUFFER, 
		nullptr, sizeof(Mat4), GL_DYNAMIC_STORAGE_BIT);
	if(err)
	{
		return err;
	}

	// Shaders
	err = m_occlusionVert.load("shaders/PpsLfOcclusion.vert.glsl",
		&getResourceManager());
	if(err)
	{
		return err;
	}

	err = m_occlusionFrag.load("shaders/PpsLfOcclusion.frag.glsl",
		&getResourceManager());
	if(err)
	{
		return err;
	}

	err = m_occlusionPpline.create(cmdBuff,
		{m_occlusionVert->getGrShader(), m_occlusionFrag->getGrShader()});
	if(err)
	{
		return err;
	}

	return err;
}

//==============================================================================
Error Lf::initInternal(const ConfigSet& config)
{
	Error err = ErrorCode::NONE;
	m_enabled = config.get("pps.lf.enabled") 
		&& config.get("pps.hdr.enabled");
	if(!m_enabled)
	{
		return err;
	}

	CommandBufferHandle cmdBuff;
	err = cmdBuff.create(&getGrManager());
	if(err)
	{
		return err;
	}

	err = initPseudo(config, cmdBuff);
	if(err)
	{
		return err;
	}

	err = initSprite(config, cmdBuff);
	if(err)
	{
		return err;
	}

	err = initOcclusion(config, cmdBuff);
	if(err)
	{
		return err;
	}

	// Create the render target
	err = m_r->createRenderTarget(m_r->getPps().getHdr()._getWidth(), 
		m_r->getPps().getHdr()._getHeight(), GL_RGB8, 1, m_rt);
	if(err)
	{
		return err;
	}

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	err = m_fb.create(cmdBuff, fbInit);
	if(err)
	{
		return err;
	}

	// Blit
	err = m_blitFrag.load("shaders/Blit.frag.glsl", &getResourceManager());
	if(err)
	{
		return err;
	}

	err = m_r->createDrawQuadPipeline(
		m_blitFrag->getGrShader(), m_blitPpline);
	if(err)
	{
		return err;
	}

	cmdBuff.flush();

	return err;
}

//==============================================================================
Error Lf::runOcclusionTests(CommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	// Retrieve some things
	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
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
		cmdb.setDepthWriteMask(false);
		cmdb.enableDepthTest(true);
		m_occlusionPpline.bind(cmdb);

		// Setup MVP UBO
		Mat4 mvp = camFr.getViewProjectionMatrix().getTransposed();
		m_mvpBuff.write(cmdb, &mvp, sizeof(mvp), 0, 0, sizeof(mvp));
		m_mvpBuff.bindShaderBuffer(cmdb, 0, sizeof(Mat4), 0);

		// Allocate vertices and fire write job
		BufferHandle& positionsVertBuff = m_positionsVertBuff[
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
			OcclusionQueryHandle& query = lf.getOcclusionQueryToTest();
			query.begin(cmdb);

			cmdb.drawArrays(GL_POINTS, 1, 1, positions - initialPositions);
			
			query.end(cmdb);

			++positions;
		}

		ANKI_ASSERT(positions == initialPositions + totalCount);

		// Restore state
		cmdb.setColorWriteMask(true, true, true, true);
		cmdb.setDepthWriteMask(true);
		cmdb.enableDepthTest(false);
	}

	return err;
}

//==============================================================================
Error Lf::run(CommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	//
	// First pass
	//

	// Set the common state
	m_fb.bind(cmdBuff, true);
	cmdBuff.setViewport(0, 0, m_r->getPps().getHdr()._getWidth(), 
		m_r->getPps().getHdr()._getHeight());

	m_pseudoPpline.bind(cmdBuff);

	Array<TextureHandle, 2> tarr = {{
		m_r->getPps().getHdr()._getRt(), 
		m_lensDirtTex->getGlTexture()}};
	cmdBuff.bindTextures(0, tarr.begin(), tarr.getSize());

	m_r->drawQuad(cmdBuff);

	//
	// Rest of the passes
	//

	// Retrieve some things
	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
	VisibilityTestResults& vi = camFr.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		// Allocate client buffer
		const U uboAlignment = 
			m_r->_getGrManager().getBufferOffsetAlignment(GL_UNIFORM_BUFFER);
		const U bufferSize = m_flareSize * totalCount;

		// Set common rendering state
		m_realPpline.bind(cmdBuff);
		cmdBuff.enableBlend(true);
		cmdBuff.setBlendFunctions(GL_ONE, GL_ONE);

		// Send the command to write the buffer now
		BufferHandle& flareDataBuff = m_flareDataBuff[
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
			lf.getTexture().bind(cmdBuff, 0);
			flareDataBuff.bindShaderBuffer(
				cmdBuff, 
				reinterpret_cast<U8*>(sprites) - spritesInitialPtr, 
				sizeof(Sprite) * count,
				0);

			OcclusionQueryHandle query;
			Bool queryInvalid;
			lf.getOcclusionQueryToCheck(query, queryInvalid);
			
			if(!queryInvalid)
			{
				m_r->drawQuadConditional(query, cmdBuff);
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
	}
	else
	{
		// No flares

		cmdBuff.enableBlend(true);
		cmdBuff.setBlendFunctions(GL_ONE, GL_ONE);
	}

	// Blit the HDR RT back to LF RT
	//
	m_r->getPps().getHdr()._getRt().bind(cmdBuff, 0);
	m_blitPpline.bind(cmdBuff);
	m_r->drawQuad(cmdBuff);

	cmdBuff.enableBlend(false);

	return err;
}

} // end namespace anki
