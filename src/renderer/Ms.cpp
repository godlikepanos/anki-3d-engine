// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
	Plane& plane = m_planes[index];

	ANKI_CHECK(m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		GL_DEPTH_COMPONENT24, samples, plane.m_depthRt));

	ANKI_CHECK(m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), 
		GL_RGBA8, samples, plane.m_rt0));

	ANKI_CHECK(m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), 
		GL_RGBA8, samples, plane.m_rt1));

	GrManager& gl = getGrManager();
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 2;
	fbInit.m_colorAttachments[0].m_texture = plane.m_rt0;
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[1].m_texture = plane.m_rt1;
	fbInit.m_colorAttachments[1].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_texture = plane.m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = 
		AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearColor.m_float[0] = 1.0;

	ANKI_CHECK(plane.m_fb.create(cmdb, fbInit));
	cmdb.finish();

	return ErrorCode::NONE;
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

	err = m_ez.init(initializer);
	return err;
}

//==============================================================================
Error Ms::run(CommandBufferHandle& cmdb)
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
		m_r->getSceneGraph().getActiveCamera().
		getComponent<FrustumComponent>().getVisibilityTestResults();

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

	cmdb.enableDepthTest(false);

	return err;
}

} // end namespace anki
