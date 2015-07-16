// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Ms.h"
#include "anki/renderer/Renderer.h"

#include "anki/util/Logger.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
const Array<PixelFormat, Ms::ATTACHMENT_COUNT> Ms::RT_PIXEL_FORMATS = {
	PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM),
	PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM),
	PixelFormat(ComponentFormat::R10G10B10A2, TransformFormat::UNORM)};

const PixelFormat Ms::DEPTH_RT_PIXEL_FORMAT(
	ComponentFormat::D24, TransformFormat::FLOAT);

//==============================================================================
Ms::~Ms()
{}

//==============================================================================
Error Ms::createRt(U32 index, U32 samples)
{
	Plane& plane = m_planes[index];

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		DEPTH_RT_PIXEL_FORMAT, samples, SamplingFilter::NEAREST, 4,
		plane.m_depthRt);

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		RT_PIXEL_FORMATS[0], samples, SamplingFilter::NEAREST, 1, plane.m_rt0);

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		RT_PIXEL_FORMATS[1], samples, SamplingFilter::NEAREST, 1, plane.m_rt1);

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(),
		RT_PIXEL_FORMATS[2], samples, SamplingFilter::NEAREST, 1, plane.m_rt2);

	AttachmentLoadOperation loadop = AttachmentLoadOperation::DONT_CARE;
#if ANKI_DEBUG
	loadop = AttachmentLoadOperation::CLEAR;
#endif

	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = ATTACHMENT_COUNT;
	fbInit.m_colorAttachments[0].m_texture = plane.m_rt0;
	fbInit.m_colorAttachments[0].m_loadOperation = loadop;
	fbInit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[1].m_texture = plane.m_rt1;
	fbInit.m_colorAttachments[1].m_loadOperation = loadop;
	fbInit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0, 1.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[2].m_texture = plane.m_rt2;
	fbInit.m_colorAttachments[2].m_loadOperation = loadop;
	fbInit.m_colorAttachments[2].m_clearValue.m_colorf = {{0.0, 0.0, 1.0, 0.0}};
	fbInit.m_depthStencilAttachment.m_texture = plane.m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	plane.m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

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
	if(initializer.getNumber("samples") > 1)
	{
		ANKI_CHECK(createRt(0, initializer.getNumber("samples")));
	}

	ANKI_CHECK(createRt(1, 1));

	return ErrorCode::NONE;
}

//==============================================================================
Error Ms::run(CommandBufferPtr& cmdb)
{
	// Chose the multisampled or the singlesampled framebuffer
	U planeId = 0;
	if(m_r->getSamples() == 1)
	{
		planeId = 1;
	}

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	cmdb->bindFramebuffer(m_planes[planeId].m_fb);

	// render all
	m_r->getSceneDrawer().prepareDraw(
		RenderingStage::MATERIAL, Pass::MS_FS, cmdb);

	SceneNode& cam = m_r->getActiveCamera();

	VisibilityTestResults& vi =
		cam.getComponent<FrustumComponent>().getVisibilityTestResults();

	auto it = vi.getRenderablesBegin();
	auto end = vi.getRenderablesEnd();
	for(; it != end; ++it)
	{
		ANKI_CHECK(m_r->getSceneDrawer().render(cam, *it));
	}

	m_r->getSceneDrawer().finishDraw();

	// If there is multisampling then resolve to singlesampled
	if(m_r->getSamples() > 1)
	{
#if 0
		fbo[1].blitFrom(fbo[0], UVec2(0U), UVec2(r->getWidth(), r->getHeight()),
			UVec2(0U), UVec2(r->getWidth(), r->getHeight()),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST_BASE);
#endif
		ANKI_ASSERT(0 && "TODO");
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Ms::generateMipmaps(CommandBufferPtr& cmdb)
{
	U planeId = (m_r->getSamples() == 1) ? 1 : 0;
	cmdb->generateMipmaps(m_planes[planeId].m_depthRt);
}

} // end namespace anki
