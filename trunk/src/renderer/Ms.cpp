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
void Ms::init(const RendererInitializer& initializer)
{
	try
	{
#if ANKI_RENDERER_USE_MRT
		fai0.create2dFai(r->getWidth(), r->getHeight(), GL_SRGB8_ALPHA8,
			GL_RGBA, GL_UNSIGNED_BYTE, 16);
		fai1.create2dFai(r->getWidth(), r->getHeight(), GL_RG16F,
			GL_RG, GL_FLOAT);
#else
		fai0.create2dFai(r->getWidth(), r->getHeight(), GL_RG32UI,
			GL_RG_INTEGER, GL_UNSIGNED_INT);
#endif
		depthFai.create2dFai(r->getWidth(), r->getHeight(),
			GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
			GL_UNSIGNED_INT_24_8);

		fbo.create();
#if ANKI_RENDERER_USE_MRT
		fbo.setColorAttachments({&fai0, &fai1});
#else
		fbo.setColorAttachments({&fai0});
#endif
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
	GlState& gl = GlStateSingleton::get();

	fbo.bind();
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
		*r->getSceneGraph().getActiveCamera().getVisibilityTestResults();

	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(r->getSceneGraph().getActiveCamera(),
			RenderableDrawer::RS_MATERIAL, COLOR_PASS, *(*it).node, 
			(*it).subSpatialIndices, (*it).subSpatialIndicesCount);
	}

	// Gen mips
	//fai0.generateMipmaps();

	ANKI_CHECK_GL_ERROR();
}

} // end namespace anki
