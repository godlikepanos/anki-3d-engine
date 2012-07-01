#include "anki/renderer/Ms.h"
#include "anki/renderer/Ez.h"
#include "anki/renderer/Renderer.h"

#include "anki/core/Logger.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
Ms::~Ms()
{}

//==============================================================================
void Ms::init(const RendererInitializer& initializer)
{
	try
	{
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB16F,
			GL_RGB, GL_FLOAT, normalFai);
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGB8,
			GL_RGB, GL_FLOAT, diffuseFai);
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RGBA8,
			GL_RGBA, GL_FLOAT, specularFai);
		Renderer::createFai(r->getWidth(), r->getHeight(),
			GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
			GL_UNSIGNED_INT_24_8, depthFai);

		fbo.create();
		fbo.setColorAttachments({&normalFai, &diffuseFai, &specularFai});
		fbo.setOtherAttachment(GL_DEPTH_STENCIL_ATTACHMENT, depthFai);
		ANKI_ASSERT(fbo.isComplete());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create deferred "
			"shading material stage FBO") << e;
	}

	ez.init(initializer);
}

//==============================================================================
void Ms::run()
{
	/*if(ez.getEnabled())
	{
		ez.run();
	}*/

	fbo.bind();

	/*if(!ez.getEnabled())
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}*/

	GlStateSingleton::get().setViewport(0, 0, r->getWidth(), r->getHeight());

	//GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, true);
	//app->getScene().skybox.Render(cam.getViewMatrix().getRotationPart());
	//glDepthFunc(GL_LEQUAL);

	// if ez then change the default depth test and disable depth writing
	/*if(ez.getEnabled())
	{
		glDepthMask(false);
		glDepthFunc(GL_EQUAL);
	}*/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render all
	for(Renderable* node :
		r->getScene().getVisibilityInfo().getRenderables())
	{
		r->getSceneDrawer().render(r->getScene().getActiveCamera(),
			0, *node);
	}

	// restore depth
	/*if(ez.getEnabled())
	{
		glDepthMask(true);
		glDepthFunc(GL_LESS);
	}*/
	ANKI_CHECK_GL_ERROR();
}

} // end namespace
