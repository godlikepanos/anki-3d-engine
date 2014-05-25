#include "anki/renderer/Sslr.h"
#include "anki/renderer/Renderer.h"
#include <sstream>

namespace anki {

//==============================================================================
void Sslr::init(const RendererInitializer& initializer)
{
	m_enabled = initializer.get("pps.sslr.enabled");

	if(!m_enabled)
	{
		return;
	}

	// Size
	const F32 quality = initializer.get("pps.sslr.renderingQuality");
	m_blurringIterationsCount = 
		initializer.get("pps.sslr.blurringIterationsCount");

	m_width = quality * (F32)m_r->getWidth();
	alignRoundUp(16, m_width);
	m_height = quality * (F32)m_r->getHeight();
	alignRoundUp(16, m_height);

	// Programs
	std::stringstream pps;

	pps << "#define WIDTH " << m_width 
		<< "\n#define HEIGHT " << m_height
		<< "\n";

	m_reflectionFrag.load(ProgramResource::createSrcCodeToCache(
		"shaders/PpsSslr.frag.glsl", pps.str().c_str(), "r_").c_str());

	m_reflectionPpline = m_r->createDrawQuadProgramPipeline(
		m_reflectionFrag->getGlProgram());

	// Blit
	m_blitFrag.load("shaders/Blit.frag.glsl");
	m_blitPpline = m_r->createDrawQuadProgramPipeline(
		m_blitFrag->getGlProgram());

	// Init FBOs and RTs and blurring
	if(m_blurringIterationsCount > 0)
	{
		initBlurring(*m_r, m_width, m_height, 7, 0.5);
	}
	else
	{
		GlManager& gl = GlManagerSingleton::get();
		GlJobChainHandle jobs(&gl);

		Direction& dir = m_dirs[(U)DirectionEnum::VERTICAL];

		m_r->createRenderTarget(m_width, m_height, GL_RGB8, GL_RGB, 
			GL_UNSIGNED_BYTE, 1, dir.m_rt);

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(jobs, GlTextureHandle::Filter::LINEAR);

		// Create FB
		dir.m_fb = GlFramebufferHandle(
			jobs, {{dir.m_rt, GL_COLOR_ATTACHMENT0}});

		jobs.finish();
	}
}

//==============================================================================
void Sslr::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	// Compute the reflection
	//
	m_dirs[(U)DirectionEnum::VERTICAL].m_fb.bind(jobs, true);
	jobs.setViewport(0, 0, m_width, m_height);

	m_reflectionPpline.bind(jobs);
	m_r->getIs()._getRt().bind(jobs, 0);
	m_r->getMs()._getDepthRt().bind(jobs, 1);
	m_r->getMs()._getRt1().bind(jobs, 2);
	m_r->getPps().getSsao().m_uniformsBuff.bindShaderBuffer(jobs, 0);

	m_r->drawQuad(jobs);

	// Blurring
	//
	if(m_blurringIterationsCount > 0)
	{
		runBlurring(*m_r, jobs);
	}

	// Write the reflection back to IS RT
	//
	m_r->getIs().m_fb.bind(jobs, false);
	jobs.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	jobs.enableBlend(true);
	jobs.setBlendFunctions(GL_ONE, GL_ONE);

	m_dirs[(U)DirectionEnum::VERTICAL].m_rt.bind(jobs, 0);

	m_blitPpline.bind(jobs);
	m_r->drawQuad(jobs);

	jobs.enableBlend(false);
}

} // end namespace anki

