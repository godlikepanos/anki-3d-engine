// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/RenderingPass.h"
#include "anki/renderer/Renderer.h"
#include "anki/util/Enum.h"

namespace anki {

//==============================================================================
Timestamp RenderingPass::getGlobalTimestamp() const
{
	return m_r->getGlobalTimestamp();
}

//==============================================================================
GrManager& RenderingPass::getGrManager()
{
	return m_r->_getGrManager();
}

//==============================================================================
const GrManager& RenderingPass::getGrManager() const
{
	return m_r->_getGrManager();
}

//==============================================================================
HeapAllocator<U8> RenderingPass::getAllocator() const
{
	return m_r->getAllocator();
}

//==============================================================================
StackAllocator<U8> RenderingPass::getFrameAllocator() const
{
	return m_r->getFrameAllocator();
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
	Array<StringAuto, 2> pps = {{getAllocator(), getAllocator()}};

	pps[1].sprintf(
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		blurringDistance, height, samples);

	pps[0].sprintf(
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		blurringDistance, width, samples);

	for(U i = 0; i < 2; i++)
	{
		Direction& dir = m_dirs[i];

		ANKI_CHECK(r.createRenderTarget(width, height,
			PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM),
			1, SamplingFilter::LINEAR, 1, dir.m_rt));

		// Create FB
		FramebufferPtr::Initializer fbInit;
		fbInit.m_colorAttachmentsCount = 1;
		fbInit.m_colorAttachments[0].m_texture = dir.m_rt;
		fbInit.m_colorAttachments[0].m_loadOperation =
			AttachmentLoadOperation::DONT_CARE;
		ANKI_CHECK(dir.m_fb.create(&getGrManager(), fbInit));

		ANKI_CHECK(dir.m_frag.loadToCache(&getResourceManager(),
			"shaders/VariableSamplingBlurGeneric.frag.glsl",
			pps[i].toCString(), "r_"));

		ANKI_CHECK(r.createDrawQuadPipeline(
			dir.m_frag->getGrShader(), dir.m_ppline));
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error BlurringRenderingPass::runBlurring(
	Renderer& r, CommandBufferPtr& cmdb)
{
	// H pass input
	m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_rt.bind(cmdb, 1);

	// V pass input
	m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_rt.bind(cmdb, 0);

	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_fb.bind(cmdb);
		m_dirs[enumToValue(DirectionEnum::HORIZONTAL)].m_ppline.bind(cmdb);
		r.drawQuad(cmdb);

		// vpass
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_fb.bind(cmdb);
		m_dirs[enumToValue(DirectionEnum::VERTICAL)].m_ppline.bind(cmdb);
		r.drawQuad(cmdb);
	}

	return ErrorCode::NONE;
}

} // end namespace anki

