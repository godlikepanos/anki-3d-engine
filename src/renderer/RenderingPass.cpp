// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/RenderingPass.h"
#include "anki/renderer/Renderer.h"
#include "anki/util/Enum.h"

namespace anki {

//==============================================================================
GlDevice& RenderingPass::getGlDevice()
{
	return m_r->getGlDevice();
}

//==============================================================================
HeapAllocator<U8>& RenderingPass::getAllocator()
{
	return m_r->_getAllocator();
}

//==============================================================================
ResourceManager& RenderingPass::getResourceManager()
{
	return m_r->_getResourceManager();
}

//==============================================================================
void BlurringRenderingPass::initBlurring(
	Renderer& r, U width, U height, U samples, F32 blurringDistance)
{
	GlDevice& gl = getGlDevice();
	GlCommandBufferHandle jobs(&gl);

	Array<String, 2> pps = {{String(getAllocator()), String(getAllocator())}};

	pps[1].sprintf("#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n", 
		blurringDistance, height, samples);

	pps[0].sprintf("#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		blurringDistance, width, samples);

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

		dir.m_frag.load(ProgramResource::createSourceToCache(
			"shaders/VariableSamplingBlurGeneric.frag.glsl", 
			pps[i].toCString(), "r_", getResourceManager()).toCString(),
			&getResourceManager());

		dir.m_ppline = 
			r.createDrawQuadProgramPipeline(dir.m_frag->getGlProgram());
	}

	jobs.finish();
}

//==============================================================================
void BlurringRenderingPass::runBlurring(Renderer& r, GlCommandBufferHandle& jobs)
{
	// H pass input
	m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_rt.bind(jobs, 1); 

	// V pass input
	m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_rt.bind(jobs, 0); 

	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_fb.bind(jobs, true);
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_ppline.bind(jobs);
		r.drawQuad(jobs);

		// vpass
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_fb.bind(jobs, true);
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_ppline.bind(jobs);
		r.drawQuad(jobs);
	}
}

} // end namespace anki

