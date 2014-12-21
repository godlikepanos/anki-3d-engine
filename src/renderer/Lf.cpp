// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
Error Lf::initInternal(const ConfigSet& config)
{
	Error err = ErrorCode::NONE;
	m_enabled = config.get("pps.lf.enabled") 
		&& config.get("pps.hdr.enabled");
	if(!m_enabled)
	{
		return err;
	}

	GlCommandBufferHandle cmdBuff;
	err = cmdBuff.create(&getGlDevice());
	if(err) return err;

	m_maxSpritesPerFlare = config.get("pps.lf.maxSpritesPerFlare");
	m_maxFlares = config.get("pps.lf.maxFlares");

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

	err = m_r->createDrawQuadProgramPipeline(
		m_pseudoFrag->getGlProgram(), m_pseudoPpline);
	if(err) return err;

	// Load program 2
	pps.destroy(getAllocator());
	err = pps.sprintf(getAllocator(), "#define MAX_SPRITES %u\n",
		m_maxSpritesPerFlare);
	if(err)
	{
		return err;
	}

	err = m_realVert.loadToCache(&getResourceManager(), 
		"shaders/PpsLfSpritePass.vert.glsl", pps.toCString(), "r_");
	if(err) return err;

	err = m_realFrag.loadToCache(&getResourceManager(), 
		"shaders/PpsLfSpritePass.frag.glsl", pps.toCString(), "r_");
	if(err) return err;

	err = m_realPpline.create(cmdBuff,
		{m_realVert->getGlProgram(), m_realFrag->getGlProgram()});
	if(err) return err;

	PtrSize uboAlignment = 
		m_r->_getGlDevice().getBufferOffsetAlignment(GL_UNIFORM_BUFFER);
	m_flareSize = getAlignedRoundUp(
		uboAlignment, sizeof(Sprite) * m_maxSpritesPerFlare);
	PtrSize blockSize = m_flareSize * m_maxFlares;

	// Init buffer
	err = m_flareDataBuff.create(
		cmdBuff, GL_UNIFORM_BUFFER, blockSize, GL_DYNAMIC_STORAGE_BIT);
	if(err) return err;

	// Create the render target
	err = m_r->createRenderTarget(m_r->getPps().getHdr()._getWidth(), 
		m_r->getPps().getHdr()._getHeight(), 
		GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 1, m_rt);
	if(err) return err;

	err = m_fb.create(cmdBuff, {{m_rt, GL_COLOR_ATTACHMENT0}});
	if(err) return err;
	
	// Textures
	err = m_lensDirtTex.load(
		"engine_data/lens_dirt.ankitex", &getResourceManager());
	if(err) return err;

	// Blit
	err = m_blitFrag.load("shaders/Blit.frag.glsl", &getResourceManager());
	if(err) return err;

	err = m_r->createDrawQuadProgramPipeline(
		m_blitFrag->getGlProgram(), m_blitPpline);
	if(err) return err;

	cmdBuff.finish();

	return err;
}

//==============================================================================
Error Lf::runOcclusionTests(GlCommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	// Retrieve some things
	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	VisibilityTestResults& vi = cam.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		// Setup state
		cmdb.setColorWriteMask(false, false, false, false);
		cmdb.setDepthWriteMask(false);
		cmdb.enableDepthTest(true);

		// Iterate lens flare
		auto it = vi.getLensFlaresBegin();
		auto end = vi.getLensFlaresEnd();
		for(; it != end; ++it)
		{
			LensFlareComponent& lf = 
				(it->m_node)->getComponent<LensFlareComponent>();

			GlOcclusionQueryHandle& query = lf.getOcclusionQueryToTest();
			query.begin(cmdb);

			query.end(cmdb);
		}

		// Restore state
		cmdb.setColorWriteMask(true, true, true, true);
		cmdb.setDepthWriteMask(true);
		cmdb.enableDepthTest(false);
	}

	return err;
}

//==============================================================================
Error Lf::run(GlCommandBufferHandle& cmdBuff)
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

	Array<GlTextureHandle, 2> tarr = {{
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
	VisibilityTestResults& vi = cam.getVisibilityTestResults();

	U totalCount = min<U>(vi.getLensFlaresCount(), m_maxFlares);
	if(totalCount > 0)
	{
		// Allocate client buffer
		U uboAlignment = 
			m_r->_getGlDevice().getBufferOffsetAlignment(GL_UNIFORM_BUFFER);
		U bufferSize = m_flareSize * totalCount;

		GlClientBufferHandle flaresCBuff;
		err = flaresCBuff.create(cmdBuff, bufferSize, nullptr);
		if(err)	return err;

		Sprite* sprites = static_cast<Sprite*>(flaresCBuff.getBaseAddress());
		U8* spritesInitialPtr = reinterpret_cast<U8*>(sprites);

		// Set common rendering state
		m_realPpline.bind(cmdBuff);
		cmdBuff.enableBlend(true);
		cmdBuff.setBlendFunctions(GL_ONE, GL_ONE);

		// Send the command to write the buffer now
		m_flareDataBuff.write(
			cmdBuff, flaresCBuff, 0, 0, 
			bufferSize);

		// Iterate lens flare
		auto it = vi.getLensFlaresBegin();
		auto end = vi.getLensFlaresEnd();
		for(; it != end; ++it)
		{			
			LensFlareComponent& lf = 
				(it->m_node)->getComponent<LensFlareComponent>();
			U count = 0;

			// Compute position
			Vec4 lfPos = Vec4(lf.getWorldPosition().xyz(), 1.0);
			Vec4 posClip = cam.getViewProjectionMatrix() * lfPos;

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
			m_flareDataBuff.bindShaderBuffer(
				cmdBuff, 
				reinterpret_cast<U8*>(sprites) - spritesInitialPtr, 
				sizeof(Sprite) * count,
				0);

			m_r->drawQuad(cmdBuff);

			// Advance
			U advancementSize = 
				getAlignedRoundUp(uboAlignment, sizeof(Sprite) * count);

			sprites = reinterpret_cast<Sprite*>(
				reinterpret_cast<U8*>(sprites) + advancementSize);
		}
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
