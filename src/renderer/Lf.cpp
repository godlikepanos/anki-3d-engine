// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Lf.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

namespace {

//==============================================================================
class Flare
{
public:
	Vec2 m_pos; ///< Position in NDC
	Vec2 m_scale; ///< Scale of the quad
	F32 m_alpha; ///< Alpha value
	F32 m_depth; ///< Texture depth
	U32 m_padding[2];
};

//==============================================================================
class LightSortFunctor
{
public:
	Bool operator()(const Light* lightA, const Light* lightB)
	{
		ANKI_ASSERT(lightA && lightB);
		ANKI_ASSERT(lightA->hasLensFlare() && lightB->hasLensFlare());

		return lightA->getLensFlareTexture() <
			lightB->getLensFlareTexture();
	}
};

} // end namespace anonymous

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

	m_maxFlaresPerLight = config.get("pps.lf.maxFlaresPerLight");
	m_maxLightsWithFlares = config.get("pps.lf.maxLightsWithFlares");

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
	err = pps.sprintf(getAllocator(), "#define MAX_FLARES %u\n",
		m_maxFlaresPerLight * m_maxLightsWithFlares);
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

	PtrSize blockSize = 
		sizeof(Flare) * m_maxFlaresPerLight * m_maxLightsWithFlares;

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

	// Iterate the visible light and get those that have lens flare
	SceneFrameDArrayAuto<Light*> lights(scene.getFrameAllocator());
	err = lights.create(m_maxLightsWithFlares, nullptr);
	if(err)	return err;

	U lightsCount = 0;
	auto it = vi.getLightsBegin();
	auto end = vi.getLightsEnd();
	for(; it != end; ++it)
	{
		SceneNode& sn = *(it->m_node);
		ANKI_ASSERT(sn.tryGetComponent<LightComponent>() != nullptr);
		Light* light = staticCastPtr<Light*>(&sn);

		if(light->hasLensFlare())
		{
			lights[lightsCount++] = light;

			if(lightsCount == m_maxLightsWithFlares)
			{
				break;
			}
		}
	}

	// Check the light count
	if(lightsCount != 0)
	{
		// Sort the lights using their lens flare texture
		std::sort(lights.begin(), lights.begin() + lightsCount, 
			LightSortFunctor());

		// Write the UBO and get the groups
		//
		GlClientBufferHandle flaresCBuff;
		err = flaresCBuff.create(cmdBuff,
			sizeof(Flare) * lightsCount * m_maxFlaresPerLight, nullptr);
		if(err)	return err;

		Flare* flares = (Flare*)flaresCBuff.getBaseAddress();
		U flaresCount = 0;

		// Contains the number of flares per flare texture
		SceneFrameDArrayAuto<U> groups(scene.getFrameAllocator());
		err = groups.create(lightsCount, 0U);
		if(err)	return err;

		SceneFrameDArrayAuto<const GlTextureHandle*> texes(
			scene.getFrameAllocator());
		err = texes.create(lightsCount, nullptr);
		if(err)	return err;

		U groupsCount = 0;

		GlTextureHandle lastTex;

		// Iterate all lights and update the flares as well as the groups
		while(lightsCount-- != 0)
		{
			Light& light = *lights[lightsCount];
			const GlTextureHandle& tex = light.getLensFlareTexture();
			const U depth = light.getLensFlareTextureDepth();

			// Transform
			Vec3 posWorld = light.getWorldTransform().getOrigin().xyz();
			Vec4 posClip = cam.getViewProjectionMatrix() * Vec4(posWorld, 1.0);

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

			// New group?
			if(lastTex != tex)
			{
				texes[groupsCount] = &tex;
				lastTex = tex;

				++groupsCount;
			}

			// First flare 
			F32 stretchFactor = 1.0 - posNdc.getLength();
			stretchFactor *= stretchFactor;

			Vec2 stretch = 
				light.getLensFlaresStretchMultiplier() * stretchFactor;

			flares[flaresCount].m_pos = posNdc;
			flares[flaresCount].m_scale =
				light.getLensFlaresSize() * Vec2(1.0, m_r->getAspectRatio())
				* stretch;
			flares[flaresCount].m_depth = 0.0;
			flares[flaresCount].m_alpha = 
				light.getLensFlaresAlpha() * stretchFactor;
			++flaresCount;
			++groups[groupsCount - 1];

			// The rest of the flares
			for(U d = 1; d < depth; d++)
			{
				// Write the "flares"
				F32 factor = d / ((F32)depth - 1.0);

				F32 flen = len * 2.0 * factor;

				flares[flaresCount].m_pos = posNdc + dir * flen;

				flares[flaresCount].m_scale =
					light.getLensFlaresSize() * Vec2(1.0, m_r->getAspectRatio())
					* ((len - flen) * 2.0);

				flares[flaresCount].m_depth = d;

				flares[flaresCount].m_alpha = light.getLensFlaresAlpha();

				// Advance
				++flaresCount;
				++groups[groupsCount - 1];
			}
		}

		// Time to render
		//

		// Write the buffer
		m_flareDataBuff.write(
			cmdBuff, flaresCBuff, 0, 0, sizeof(Flare) * flaresCount);

		// Set the common state
		m_realPpline.bind(cmdBuff);

		cmdBuff.enableBlend(true);
		cmdBuff.setBlendFunctions(GL_ONE, GL_ONE);

		PtrSize offset = 0;
		for(U i = 0; i < groupsCount; i++)
		{
			GlTextureHandle tex = *texes[i];
			U instances = groups[i];
			PtrSize buffSize = sizeof(Flare) * instances;

			tex.bind(cmdBuff, 0);
			m_flareDataBuff.bindShaderBuffer(cmdBuff, offset, buffSize, 0);

			m_r->drawQuadInstanced(cmdBuff, instances);

			offset += buffSize;
		}
	}
	else
	{
		// No lights

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
