#include "anki/renderer/Lf.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include <sstream>

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
void Lf::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init lens flare pass") << e;
	}
}

//==============================================================================
void Lf::initInternal(const RendererInitializer& initializer)
{
	m_enabled = initializer.get("pps.lf.enabled") 
		&& initializer.get("pps.hdr.enabled");
	if(!m_enabled)
	{
		return;
	}

	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	m_maxFlaresPerLight = initializer.get("pps.lf.maxFlaresPerLight");
	m_maxLightsWithFlares = initializer.get("pps.lf.maxLightsWithFlares");

	// Load program 1
	std::stringstream pps;
	pps << "#define TEX_DIMENSIONS vec2(" 
		<< (U)m_r->getPps().getHdr()._getRt().getWidth() << ".0, "
		<< (U)m_r->getPps().getHdr()._getRt().getHeight() << ".0)\n";

	std::string fname = ProgramResource::createSrcCodeToCache(
		"shaders/PpsLfPseudoPass.frag.glsl", pps.str().c_str(), "r_");
	m_pseudoFrag.load(fname.c_str());
	m_pseudoPpline = m_r->createDrawQuadProgramPipeline(
		m_pseudoFrag->getGlProgram());

	// Load program 2
	pps.str("");
	pps << "#define MAX_FLARES "
		<< (U)(m_maxFlaresPerLight * m_maxLightsWithFlares) << "\n";

	fname = ProgramResource::createSrcCodeToCache(
		"shaders/PpsLfSpritePass.vert.glsl", pps.str().c_str(), "r_");
	m_realVert.load(fname.c_str());

	fname = ProgramResource::createSrcCodeToCache(
		"shaders/PpsLfSpritePass.frag.glsl", pps.str().c_str(), "r_");
	m_realFrag.load(fname.c_str());

	m_realPpline = GlProgramPipelineHandle(jobs,
		{m_realVert->getGlProgram(), m_realFrag->getGlProgram()});

	PtrSize blockSize = 
		sizeof(Flare) * m_maxFlaresPerLight * m_maxLightsWithFlares;
	if(m_realVert->getGlProgram().findBlock("bFlares").getSize() 
		!= blockSize)
	{
		throw ANKI_EXCEPTION("Incorrect block size");
	}

	// Init buffer
	m_flareDataBuff = GlBufferHandle(
		jobs, GL_SHADER_STORAGE_BUFFER, blockSize, GL_DYNAMIC_STORAGE_BIT);

	// Create the render target
	m_r->createRenderTarget(m_r->getPps().getHdr()._getRt().getWidth(), 
		m_r->getPps().getHdr()._getRt().getHeight(), 
		GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 1, m_rt);

	m_fb = GlFramebufferHandle(jobs, {{m_rt, GL_COLOR_ATTACHMENT0}}); 
	
	// Textures
	m_lensDirtTex.load("engine_data/lens_dirt.ankitex");

	// Blit
	m_blitFrag.load("shaders/Blit.frag.glsl");
	m_blitPpline = m_r->createDrawQuadProgramPipeline(
		m_blitFrag->getGlProgram());

	jobs.finish();
}

//==============================================================================
void Lf::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	//
	// First pass
	//

	// Set the common state
	m_fb.bind(jobs, true);
	jobs.setViewport(0, 0, m_r->getPps().getHdr()._getRt().getWidth(), 
		m_r->getPps().getHdr()._getRt().getHeight());

	m_pseudoPpline.bind(jobs);

	jobs.bindTextures(0, {
		m_r->getPps().getHdr()._getRt(), 
		m_lensDirtTex->getGlTexture()});

	m_r->drawQuad(jobs);

	//
	// Rest of the passes
	//

	// Retrieve some things
	SceneGraph& scene = m_r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	VisibilityTestResults& vi = cam.getVisibilityTestResults();

	// Iterate the visible light and get those that have lens flare
	SceneFrameVector<Light*> lights(
		m_maxLightsWithFlares, nullptr, scene.getFrameAllocator());

	U lightsCount = 0;
	for(auto& it : vi.m_lights)
	{
		SceneNode& sn = *it.m_node;
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
		GlClientBufferHandle flaresCBuff(jobs,
			sizeof(Flare) * lightsCount * m_maxFlaresPerLight, nullptr);
		Flare* flares = (Flare*)flaresCBuff.getBaseAddress();
		U flaresCount = 0;

		// Contains the number of flares per flare texture
		SceneFrameVector<U> groups(lightsCount, 0U, scene.getFrameAllocator());
		SceneFrameVector<const GlTextureHandle*> texes(
			lightsCount, nullptr, scene.getFrameAllocator());
		U groupsCount = 0;

		GlTextureHandle lastTex;

		// Iterate all lights and update the flares as well as the groups
		while(lightsCount-- != 0)
		{
			Light& light = *lights[lightsCount];
			const GlTextureHandle& tex = light.getLensFlareTexture();
			const U depth = tex.getDepth();

			// Transform
			Vec3 posWorld = light.getWorldTransform().getOrigin();
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
			jobs, flaresCBuff, 0, 0, sizeof(Flare) * flaresCount);

		// Set the common state
		m_realPpline.bind(jobs);

		jobs.enableBlend(true);
		jobs.setBlendFunctions(GL_ONE, GL_ONE);

		PtrSize offset = 0;
		for(U i = 0; i < groupsCount; i++)
		{
			GlTextureHandle tex = *texes[i];
			U instances = groups[i];
			PtrSize buffSize = sizeof(Flare) * instances;

			tex.bind(jobs, 0);
			m_flareDataBuff.bindShaderBuffer(jobs, offset, buffSize, 0);

			m_r->drawQuadInstanced(jobs, instances);

			offset += buffSize;
		}
	}
	else
	{
		// No lights

		jobs.enableBlend(true);
		jobs.setBlendFunctions(GL_ONE, GL_ONE);
	}

	// Blit the HDR RT back to LF RT
	//
	m_r->getPps().getHdr()._getRt().bind(jobs, 0);
	m_blitPpline.bind(jobs);
	m_r->drawQuad(jobs);

	jobs.enableBlend(false);
}

} // end namespace anki
