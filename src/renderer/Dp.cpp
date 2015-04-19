// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Dp.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
Error Dp::init(const ConfigSet& config)
{
	m_smallDepthSize = UVec2(
		getAlignedRoundDown(16, m_r->getWidth() / 3),
		getAlignedRoundDown(16, m_r->getHeight() / 3));

	ANKI_CHECK(m_r->createRenderTarget(
		m_smallDepthSize.x(), m_smallDepthSize.y(),
		PixelFormat(ComponentFormat::D24, TransformFormat::FLOAT),
		1, SamplingFilter::LINEAR, 1, m_smallDepthRt));

	GrManager& gl = getGrManager();
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_smallDepthRt;
	ANKI_CHECK(m_smallDepthFb.create(cmdb, fbInit));

	cmdb.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Dp::run(CommandBufferHandle& cmdb)
{
	m_smallDepthFb.blit(
		cmdb, 
		m_r->getMs().getFramebuffer(), 
		{{0, 0, m_r->getWidth(), m_r->getHeight()}},
		{{0, 0, m_smallDepthSize.x(), m_smallDepthSize.y()}},
		GL_DEPTH_BUFFER_BIT, 
		false);

	return ErrorCode::NONE;
}

} // end namespace anki

