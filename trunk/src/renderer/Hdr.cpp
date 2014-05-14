#include "anki/renderer/Hdr.h"
#include "anki/renderer/Renderer.h"
#include <sstream>

namespace anki {

//==============================================================================
Hdr::~Hdr()
{}

//==============================================================================
void Hdr::initFb(GlFramebufferHandle& fb, GlTextureHandle& rt)
{
	GlManager& gl = GlManagerSingleton::get();

	m_r->createRenderTarget(m_width, m_height, GL_RGB8, GL_RGB, 
		GL_UNSIGNED_BYTE, 1, rt);

	// Set to bilinear because the blurring techniques take advantage of that
	GlJobChainHandle jobs(&gl);
	rt.setFilter(jobs, GlTextureHandle::Filter::LINEAR);

	// Create FB
	fb = GlFramebufferHandle(jobs, {{rt, GL_COLOR_ATTACHMENT0}});
	jobs.flush();
}

//==============================================================================
void Hdr::initInternal(const RendererInitializer& initializer)
{
	m_enabled = initializer.get("pps.hdr.enabled");

	if(!m_enabled)
	{
		return;
	}

	const F32 renderingQuality = initializer.get("pps.hdr.renderingQuality");

	m_width = renderingQuality * (F32)m_r->getWidth();
	alignRoundUp(16, m_width);
	m_height = renderingQuality * (F32)m_r->getHeight();
	alignRoundUp(16, m_height);

	m_exposure = initializer.get("pps.hdr.exposure");
	m_blurringDist = initializer.get("pps.hdr.blurringDist");
	m_blurringIterationsCount = 
		initializer.get("pps.hdr.blurringIterationsCount");

	initFb(m_hblurFb, m_hblurRt);
	initFb(m_vblurFb, m_vblurRt);

	// init shaders
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	m_commonBuff = GlBufferHandle(jobs, GL_SHADER_STORAGE_BUFFER, 
		sizeof(Vec4), GL_DYNAMIC_STORAGE_BIT);

	updateDefaultBlock(jobs);

	jobs.flush();

	m_toneFrag.load("shaders/PpsHdr.frag.glsl");

	m_tonePpline = 
		m_r->createDrawQuadProgramPipeline(m_toneFrag->getGlProgram());

	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	std::stringstream pps;
	pps << "#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(" << m_blurringDist << ")\n"
		"#define IMG_DIMENSION " << m_height << "\n"
		"#define SAMPLES " << (U)initializer.get("pps.hdr.samples") << "\n";

	m_hblurFrag.load(ProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());

	m_hblurPpline = 
		m_r->createDrawQuadProgramPipeline(m_hblurFrag->getGlProgram());

	pps.str("");
	pps << "#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(" << m_blurringDist << ")\n"
		"#define IMG_DIMENSION " << m_width << "\n"
		"#define SAMPLES " << (U)initializer.get("pps.hdr.samples") << "\n";

	m_vblurFrag.load(ProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());

	m_vblurPpline = 
		m_r->createDrawQuadProgramPipeline(m_vblurFrag->getGlProgram());

	// Set timestamps
	m_parameterUpdateTimestamp = getGlobTimestamp();
	m_commonUboUpdateTimestamp = getGlobTimestamp();
}

//==============================================================================
void Hdr::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS HDR") << e;
	}
}

//==============================================================================
void Hdr::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	// For the passes it should be NEAREST
	//vblurFai.setFiltering(Texture::TFrustumType::NEAREST);

	// pass 0
	m_vblurFb.bind(jobs, true);
	jobs.setViewport(0, 0, m_width, m_height);
	m_tonePpline.bind(jobs);

	if(m_parameterUpdateTimestamp > m_commonUboUpdateTimestamp)
	{
		updateDefaultBlock(jobs);
		m_commonUboUpdateTimestamp = getGlobTimestamp();
	}

	m_r->getIs()._getRt().bind(jobs, 0);
	m_commonBuff.bindShaderBuffer(jobs, 0);

	m_r->drawQuad(jobs);

	// Blurring passes
	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		if(i == 0)
		{
			m_vblurRt.bind(jobs, 1); // H pass input
			m_hblurRt.bind(jobs, 0); // V pass input
		}

		// hpass
		m_hblurFb.bind(jobs, true);
		m_hblurPpline.bind(jobs);		
		m_r->drawQuad(jobs);

		// vpass
		m_vblurFb.bind(jobs, true);
		m_vblurPpline.bind(jobs);
		m_r->drawQuad(jobs);
	}

	// For the next stage it should be LINEAR though
	//vblurFai.setFiltering(Texture::TFrustumType::LINEAR);
}

//==============================================================================
void Hdr::updateDefaultBlock(GlJobChainHandle& jobs)
{
	GlClientBufferHandle tempBuff(jobs, sizeof(Vec4), nullptr);
	
	*((Vec4*)tempBuff.getBaseAddress()) = Vec4(m_exposure, 0.0, 0.0, 0.0);

	m_commonBuff.write(jobs, tempBuff, 0, 0, tempBuff.getSize());
}

} // end namespace anki
