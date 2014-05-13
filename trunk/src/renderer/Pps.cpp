#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Pps::Pps(Renderer* r)
	:	OptionalRenderingPass(r), 
		m_hdr(r), 
		m_ssao(r), 
		m_bl(r), 
		m_lf(r),
		m_sslr(r)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
void Pps::initInternal(const RendererInitializer& initializer)
{
	m_enabled = initializer.get("pps.enabled");
	if(!m_enabled)
	{
		return;
	}

	ANKI_ASSERT("Initializing PPS");

	m_ssao.init(initializer);
	m_hdr.init(initializer);
	m_lf.init(initializer);
	m_sslr.init(initializer);

	// FBO
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGB8, GL_RGB,
		GL_UNSIGNED_BYTE, 1, m_rt);

	m_fb = GlFramebufferHandle(jobs, {{m_rt, GL_COLOR_ATTACHMENT0}});

	// SProg
	std::stringstream pps;

	pps << "#define SSAO_ENABLED " << (U)m_ssao.getEnabled() << "\n"
		<< "#define HDR_ENABLED " << (U)m_hdr.getEnabled() << "\n"
		<< "#define SHARPEN_ENABLED " << (U)initializer.get("pps.sharpen") 
			<< "\n"
		<< "#define GAMMA_CORRECTION_ENABLED " 
			<< (U)initializer.get("pps.gammaCorrection") << "\n"
		<< "#define FBO_WIDTH " << (U)m_r->getWidth() << "\n"
		<< "#define FBO_HEIGHT " << (U)m_r->getHeight() << "\n";

	m_frag.load(ProgramResource::createSrcCodeToCache(
		"shaders/Pps.frag.glsl", pps.str().c_str(), "r_").c_str());

	m_ppline = m_r->createDrawQuadProgramPipeline(m_frag->getGlProgram());

	jobs.finish();
}

//==============================================================================
void Pps::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS") << e;
	}
}

//==============================================================================
void Pps::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	// First SSAO because it depends on MS where HDR depends on IS
	if(m_ssao.getEnabled())
	{
		m_ssao.run(jobs);
	}

	// Then SSLR because HDR depends on it
	if(m_sslr.getEnabled())
	{
		m_sslr.run(jobs);
	}

	if(m_hdr.getEnabled())
	{
		m_hdr.run(jobs);
	}

	if(m_lf.getEnabled())
	{
		m_lf.run(jobs);
	}

	Bool drawToDefaultFbo = 
		!m_r->getDbg().getEnabled() 
		&& !m_r->getIsOffscreen()
		&& m_r->getRenderingQuality() == 1.0;

	if(drawToDefaultFbo)
	{
		m_r->getDefaultFramebuffer().bind(jobs, true);
		jobs.setViewport(0, 0, m_r->getWindowWidth(), m_r->getWindowHeight());
	}
	else
	{
		m_fb.bind(jobs, true);
		jobs.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}

	m_ppline.bind(jobs);

	m_r->getIs().getRt().bind(jobs, 0);

	if(m_ssao.getEnabled())
	{
		m_ssao.getRt().bind(jobs, 1);
	}

	if(m_lf.getEnabled())
	{
		m_lf.getRt().bind(jobs, 2);
	}
	else if(m_hdr.getEnabled())
	{
		m_hdr.getRt().bind(jobs, 2);
	}

	m_r->drawQuad(jobs);
}

} // end namespace anki
