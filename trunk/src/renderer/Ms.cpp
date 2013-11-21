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
const Texture& Ms::getFai1() const
{
	ANKI_ASSERT(r->getUseMrt());
	return fai1[1];
}

//==============================================================================
void Ms::createFbo(U index, U samples)
{
	if(r->getUseMrt())
	{
		fai0[index].create2dFai(r->getWidth(), r->getHeight(), GL_RGBA8,
			GL_RGBA, GL_UNSIGNED_BYTE, samples);
		fai1[index].create2dFai(r->getWidth(), r->getHeight(), GL_RGBA8,
			GL_RGBA, GL_UNSIGNED_BYTE, samples);
	}
	else
	{
		fai0[index].create2dFai(r->getWidth(), r->getHeight(), GL_RG32UI,
			GL_RG_INTEGER, GL_UNSIGNED_INT, samples);
	}

	depthFai[index].create2dFai(r->getWidth(), r->getHeight(),
		GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT,
		GL_UNSIGNED_INT, samples);

	fbo[index].create();

	if(r->getUseMrt())
	{
		fbo[index].setColorAttachments({&fai0[index], &fai1[index]});
	}
	else
	{
		fbo[index].setColorAttachments({&fai0[index]});
	}
	fbo[index].setOtherAttachment(GL_DEPTH_ATTACHMENT, depthFai[index]);

	if(!fbo[index].isComplete())
	{
		throw ANKI_EXCEPTION("FBO is incomplete");
	}
}

//==============================================================================
void Ms::init(const RendererInitializer& initializer)
{
	try
	{
		if(initializer.get("samples") > 1)
		{
			createFbo(0, initializer.get("samples"));
		}
		createFbo(1, 1);

		ez.init(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to initialize material stage") << e;
	}
}

//==============================================================================
void Ms::run()
{
	GlState& gl = GlStateSingleton::get();

	// Chose the multisampled or the singlesampled FBO
	if(r->getSamples() > 1)
	{
		fbo[0].bind();
	}
	else
	{
		fbo[1].bind();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	gl.setViewport(0, 0, r->getWidth(), r->getHeight());
	gl.disable(GL_BLEND);
	gl.enable(GL_DEPTH_TEST);
	gl.setDepthFunc(GL_LESS);
	gl.setDepthMaskEnabled(true);

	if(ez.getEnabled())
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		ez.run();
		gl.setDepthFunc(GL_LEQUAL);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//gl.setDepthMaskEnabled(false);
	}

	// render all
	r->getSceneDrawer().prepareDraw();
	VisibilityTestResults& vi =
		r->getSceneGraph().getActiveCamera().getVisibilityTestResults();

	for(auto it : vi.renderables)
	{
		r->getSceneDrawer().render(r->getSceneGraph().getActiveCamera(),
			RenderableDrawer::RS_MATERIAL, COLOR_PASS, *it.node, 
			&it.spatialIndices[0], it.spatialsCount);
	}

	// If there is multisampling then resolve to singlesampled
	if(r->getSamples() > 1)
	{
		fbo[0].bind(Fbo::FT_READ);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		fbo[1].bind(Fbo::FT_DRAW);
		static const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT1};
		glDrawBuffers(1, drawBuffers);

		glBlitFramebuffer(
			0, 0, r->getWidth(), r->getHeight(), 
			0, 0, r->getWidth(), r->getHeight(),
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		static const GLenum drawBuffers2[] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, drawBuffers2);

		glBlitFramebuffer(
			0, 0, r->getWidth(), r->getHeight(), 
			0, 0, r->getWidth(), r->getHeight(),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST);
	}

	// Gen mips
	//fai0.generateMipmaps();

	ANKI_CHECK_GL_ERROR();
}

} // end namespace anki
