// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Ms.h"
#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"

#include "anki/core/Logger.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
Ms::~Ms()
{}

//==============================================================================
void Ms::createRt(U32 index, U32 samples)
{
	Plane& plane = m_planes[index];

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
		GL_UNSIGNED_INT, samples, plane.m_depthRt);

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGBA8,
			GL_RGBA, GL_UNSIGNED_BYTE, samples, plane.m_rt0);

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGBA8,
		GL_RGBA, GL_UNSIGNED_BYTE, samples, plane.m_rt1);

	GlDevice& gl = GlDeviceSingleton::get();
	GlCommandBufferHandle jobs(&gl);

	plane.m_fb = GlFramebufferHandle(
		jobs,
		{{plane.m_rt0, GL_COLOR_ATTACHMENT0}, 
		{plane.m_rt1, GL_COLOR_ATTACHMENT1},
		{plane.m_depthRt, GL_DEPTH_ATTACHMENT}});

	jobs.finish();
}

//==============================================================================
void Ms::init(const ConfigSet& initializer)
{
	try
	{
		if(initializer.get("samples") > 1)
		{
			createRt(0, initializer.get("samples"));
		}
		createRt(1, 1);

		// Init small depth 
		{
			m_r->createRenderTarget(
				getAlignedRoundDown(16, m_r->getWidth() / 3) , 
				getAlignedRoundDown(16, m_r->getHeight() / 3),
				GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
				GL_UNSIGNED_INT, 1, m_smallDepthRt);

			GlDevice& gl = GlDeviceSingleton::get();
			GlCommandBufferHandle jobs(&gl);

			m_smallDepthRt.setFilter(jobs, GlTextureHandle::Filter::LINEAR);

			m_smallDepthFb = GlFramebufferHandle(
				jobs,
				{{m_smallDepthRt, GL_DEPTH_ATTACHMENT}});

			jobs.finish();
		}

		m_ez.init(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to initialize material stage") << e;
	}
}

//==============================================================================
void Ms::run(GlCommandBufferHandle& jobs)
{
	// Chose the multisampled or the singlesampled framebuffer
	if(m_r->getSamples() > 1)
	{
		m_planes[0].m_fb.bind(jobs, true);
	}
	else
	{
		m_planes[1].m_fb.bind(jobs, true);
	}

	jobs.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	jobs.clearBuffers(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
#if ANKI_DEBUG
		| GL_COLOR_BUFFER_BIT
#endif
			);

	jobs.enableDepthTest(true);
	jobs.setDepthWriteMask(true);

	if(m_ez.getEnabled())
	{
		jobs.setDepthFunction(GL_LESS);
		jobs.setColorWriteMask(false, false, false, false);

		m_ez.run(jobs);

		jobs.setDepthFunction(GL_LEQUAL);
		jobs.setColorWriteMask(true, true, true, true);
	}

	// render all
	m_r->getSceneDrawer().prepareDraw(RenderingStage::MATERIAL, Pass::COLOR,
		jobs);

	VisibilityTestResults& vi =
		m_r->getSceneGraph().getActiveCamera().getVisibilityTestResults();

	Camera& cam = m_r->getSceneGraph().getActiveCamera();
	for(auto& it : vi.m_renderables)
	{
		m_r->getSceneDrawer().render(cam, it);
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
	m_smallDepthFb.blit(jobs, m_planes[1].m_fb, 
		{{0, 0, m_planes[1].m_depthRt.getWidth(), 
			m_planes[1].m_depthRt.getHeight()}},
		{{0, 0, m_smallDepthRt.getWidth(), m_smallDepthRt.getHeight()}},
		GL_DEPTH_BUFFER_BIT, false);

	jobs.enableDepthTest(false);
}

} // end namespace anki
