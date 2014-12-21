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
	return m_r->_getGlDevice();
}

//==============================================================================
const GlDevice& RenderingPass::getGlDevice() const
{
	return m_r->_getGlDevice();
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
Error BlurringRenderingPass::initBlurring(
	Renderer& r, U width, U height, U samples, F32 blurringDistance)
{
	Error err = ErrorCode::NONE;
	GlDevice& gl = getGlDevice();
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err) return err;

	Array<String, 2> pps;
	String::ScopeDestroyer ppsd0(&pps[0], getAllocator());
	String::ScopeDestroyer ppsd1(&pps[1], getAllocator());

	err = pps[1].sprintf(getAllocator(),
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n", 
		blurringDistance, height, samples);
	if(err) return err;

	err = pps[0].sprintf(getAllocator(),
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		blurringDistance, width, samples);
	if(err) return err;

	for(U i = 0; i < 2; i++)
	{
		Direction& dir = m_dirs[i];

		err = r.createRenderTarget(width, height, GL_RGB8, GL_RGB, 
			GL_UNSIGNED_BYTE, 1, dir.m_rt);
		if(err) return err;

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(cmdb, GlTextureHandle::Filter::LINEAR);

		// Create FB
		err = dir.m_fb.create(
			cmdb, {{dir.m_rt, GL_COLOR_ATTACHMENT0}});
		if(err) return err;

		err = dir.m_frag.loadToCache(&getResourceManager(),
			"shaders/VariableSamplingBlurGeneric.frag.glsl", 
			pps[i].toCString(), "r_");
		if(err) return err;

		err = r.createDrawQuadPipeline(
			dir.m_frag->getGlProgram(), dir.m_ppline);
		if(err) return err;
	}

	cmdb.finish();

	return err;
}

//==============================================================================
Error BlurringRenderingPass::runBlurring(
	Renderer& r, GlCommandBufferHandle& cmdb)
{
	// H pass input
	m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_rt.bind(cmdb, 1); 

	// V pass input
	m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_rt.bind(cmdb, 0); 

	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_fb.bind(cmdb, true);
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_ppline.bind(cmdb);
		r.drawQuad(cmdb);

		// vpass
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_fb.bind(cmdb, true);
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_ppline.bind(cmdb);
		r.drawQuad(cmdb);
	}

	return ErrorCode::NONE;
}

} // end namespace anki

