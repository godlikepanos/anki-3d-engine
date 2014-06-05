// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/RenderingPass.h"
#include "anki/renderer/Renderer.h"
#include <sstream>

namespace anki {

//==============================================================================
void BlurringRenderingPass::initBlurring(
	Renderer& r, U width, U height, U samples, F32 blurringDistance)
{
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	Array<std::stringstream, 2> pps;

	pps[1] << "#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(" << blurringDistance << ")\n"
		"#define IMG_DIMENSION " << height << "\n"
		"#define SAMPLES " << samples << "\n";

	pps[0] << "#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(" << blurringDistance << ")\n"
		"#define IMG_DIMENSION " << width << "\n"
		"#define SAMPLES " << samples << "\n";

	for(U i = 0; i < 2; i++)
	{
		Direction& dir = m_dirs[i];

		r.createRenderTarget(width, height, GL_RGB8, GL_RGB, 
			GL_UNSIGNED_BYTE, 1, dir.m_rt);

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(jobs, GlTextureHandle::Filter::LINEAR);

		// Create FB
		dir.m_fb = GlFramebufferHandle(
			jobs, {{dir.m_rt, GL_COLOR_ATTACHMENT0}});

		dir.m_frag.load(ProgramResource::createSrcCodeToCache(
			"shaders/VariableSamplingBlurGeneric.frag.glsl", 
			pps[i].str().c_str(), "r_").c_str());

		dir.m_ppline = 
			r.createDrawQuadProgramPipeline(dir.m_frag->getGlProgram());
	}

	jobs.finish();
}

//==============================================================================
void BlurringRenderingPass::runBlurring(Renderer& r, GlJobChainHandle& jobs)
{
	m_dirs[(U)DirectionEnum::VERTICAL].m_rt.bind(jobs, 1); // H pass input
	m_dirs[(U)DirectionEnum::HORIZONTAL].m_rt.bind(jobs, 0); // V pass input

	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		m_dirs[(U)DirectionEnum::HORIZONTAL].m_fb.bind(jobs, true);
		m_dirs[(U)DirectionEnum::HORIZONTAL].m_ppline.bind(jobs);
		r.drawQuad(jobs);

		// vpass
		m_dirs[(U)DirectionEnum::VERTICAL].m_fb.bind(jobs, true);
		m_dirs[(U)DirectionEnum::VERTICAL].m_ppline.bind(jobs);
		r.drawQuad(jobs);
	}
}

} // end namespace anki

