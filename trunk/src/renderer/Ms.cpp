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
		Renderer::createFai(r->getWidth(), r->getHeight(), GL_RG32UI,
			GL_RG_INTEGER, GL_UNSIGNED_INT, fai0);
		Renderer::createFai(r->getWidth(), r->getHeight(),
			GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
			GL_UNSIGNED_INT_24_8, depthFai);

		fbo.create();
		fbo.setColorAttachments({&fai0});
		fbo.setOtherAttachment(GL_DEPTH_STENCIL_ATTACHMENT, depthFai);
		if(!fbo.isComplete())
		{
			throw ANKI_EXCEPTION("FBO is incomplete");
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create deferred "
			"shading material stage") << e;
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
	GlStateSingleton::get().disable(GL_BLEND);

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
	VisibilityInfo& vi =
		r->getScene().getActiveCamera().getFrustumable()->getVisibilityInfo();
	for(auto it = vi.getRenderablesBegin();
		it != vi.getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(r->getScene().getActiveCamera(), 0, *(*it));
	}

	// restore depth
	/*if(ez.getEnabled())
	{
		glDepthMask(true);
		glDepthFunc(GL_LESS);
	}*/
	ANKI_CHECK_GL_ERROR();
}

} // end namespace anki
