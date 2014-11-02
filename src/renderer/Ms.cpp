// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Ms.h"
#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"

#include "anki/util/Logger.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Ms::~Ms()
{}

//==============================================================================
Error Ms::createRt(U32 index, U32 samples)
{
	Error err = ErrorCode::NONE;

	Plane& plane = m_planes[index];

	err = m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
		GL_UNSIGNED_INT, samples, plane.m_depthRt);
	if(err) return err;

	err = m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGBA8,
			GL_RGBA, GL_UNSIGNED_BYTE, samples, plane.m_rt0);
	if(err) return err;

	err = m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGBA8,
		GL_RGBA, GL_UNSIGNED_BYTE, samples, plane.m_rt1);
	if(err) return err;

	GlDevice& gl = getGlDevice();
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err)	return err;

	err = plane.m_fb.create(
		cmdb,
		{{plane.m_rt0, GL_COLOR_ATTACHMENT0}, 
		{plane.m_rt1, GL_COLOR_ATTACHMENT1},
		{plane.m_depthRt, GL_DEPTH_ATTACHMENT}});
	if(err)	return err;

	cmdb.finish();

	return err;
}

//==============================================================================
Error Ms::init(const ConfigSet& initializer)
{
	Error err = initInternal(initializer);
	if(err)
	{
		ANKI_LOGE("Failed to initialize material stage");
	}

	return err;
}

//==============================================================================
Error Ms::initInternal(const ConfigSet& initializer)
{
	Error err = ErrorCode::NONE;

	if(initializer.get("samples") > 1)
	{
		err = createRt(0, initializer.get("samples"));
		if(err)	return err;
	}
	err = createRt(1, 1);
	if(err)	return err;

	// Init small depth 
	{
		m_smallDepthSize = UVec2(
			getAlignedRoundDown(16, m_r->getWidth() / 3),
			getAlignedRoundDown(16, m_r->getHeight() / 3));

		err = m_r->createRenderTarget(
			m_smallDepthSize.x(), 
			m_smallDepthSize.y(),
			GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
			GL_UNSIGNED_INT, 1, m_smallDepthRt);
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
	}

	err = m_ez.init(initializer);
	return err;
}

//==============================================================================
Error Ms::run(GlCommandBufferHandle& cmdb)
{
	Error err = ErrorCode::NONE;

	// Chose the multisampled or the singlesampled framebuffer
	if(m_r->getSamples() > 1)
	{
		m_planes[0].m_fb.bind(cmdb, true);
	}
	else
	{
		m_planes[1].m_fb.bind(cmdb, true);
	}

	cmdb.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb.clearBuffers(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
#if ANKI_DEBUG
		| GL_COLOR_BUFFER_BIT
#endif
			);

	cmdb.enableDepthTest(true);
	cmdb.setDepthWriteMask(true);

	if(m_ez.getEnabled())
	{
		cmdb.setDepthFunction(GL_LESS);
		cmdb.setColorWriteMask(false, false, false, false);

		err = m_ez.run(cmdb);
		if(err) return err;

		cmdb.setDepthFunction(GL_LEQUAL);
		cmdb.setColorWriteMask(true, true, true, true);
	}

	// render all
	m_r->getSceneDrawer().prepareDraw(RenderingStage::MATERIAL, Pass::COLOR,
		cmdb);

	Camera& cam = m_r->getSceneGraph().getActiveCamera();
	
	VisibilityTestResults& vi =
		m_r->getSceneGraph().getActiveCamera().getVisibilityTestResults();

	auto it = vi.getRenderablesBegin();
	auto end = vi.getRenderablesEnd();
	for(; it != end; ++it)
	{
		err = m_r->getSceneDrawer().render(cam, *it);
		if(err) return err;
	}

	m_r->getSceneDrawer().finishDraw();

	// If there is multisampling then resolve to singlesampled
	if(m_r->getSamples() > 1)
	{
#if 0
		fbo[1].blitFrom(fbo[0], UVec2(0U), UVec2(r->getWidth(), r->getHeight()),
			UVec2(0U), UVec2(r->getWidth(), r->getHeight()),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST);
#endif
		ANKI_ASSERT(0 && "TODO");
	}

	// Blit big depth buffer to small one
	m_smallDepthFb.blit(cmdb, m_planes[1].m_fb, 
		{{0, 0, m_r->getWidth(), m_r->getHeight()}},
		{{0, 0, m_smallDepthSize.x(), m_smallDepthSize.y()}},
		GL_DEPTH_BUFFER_BIT, false);

	cmdb.enableDepthTest(false);

	return err;
}

} // end namespace anki
