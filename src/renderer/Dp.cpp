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

	Error err = m_r->createRenderTarget(
		m_smallDepthSize.x(), m_smallDepthSize.y(),
		GL_DEPTH_COMPONENT24, 1, m_smallDepthRt);
	if(err) return err;

	GlDevice& gl = getGlDevice();
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err)	return err;

	m_smallDepthRt.setFilter(cmdb, GlTextureHandle::Filter::LINEAR);

	err = m_smallDepthFb.create(
		cmdb,
		{{m_smallDepthRt, GL_DEPTH_ATTACHMENT}});
	if(err)	return err;

	cmdb.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Dp::run(GlCommandBufferHandle& cmdb)
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

