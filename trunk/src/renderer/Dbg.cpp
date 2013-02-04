#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Light.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Dbg::~Dbg()
{}

//==============================================================================
void Dbg::init(const Renderer::Initializer& initializer)
{
	enabled = initializer.dbg.enabled;

	if(!enabled)
	{
		return;
	}

	try
	{
		fbo.create();
		fbo.setColorAttachments({&r->getPps().getFai()});
		fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());

		if(!fbo.isComplete())
		{
			throw ANKI_EXCEPTION("FBO is incomplete");
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}

	drawer.reset(new DebugDrawer);
	sceneDrawer.reset(new SceneDebugDrawer(drawer.get()));
}

//==============================================================================
void Dbg::run()
{
	ANKI_ASSERT(enabled);

	Scene& scene = r->getScene();

	fbo.bind();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().enable(GL_DEPTH_TEST, depthTest);

	drawer->setViewProjectionMatrix(r->getViewProjectionMatrix());
	drawer->setModelMatrix(Mat4::getIdentity());
	//drawer->drawGrid();

	for(auto it = scene.getSceneNodesBegin();
		it != scene.getSceneNodesEnd(); it++)
	{
		SceneNode* node = *it;
		Spatial* sp = node->getSpatial();
		if(sp)
		{
			sceneDrawer->draw(*node);
		}
	}

	// Draw sectors
	for(const Sector* sector : scene.getSectorGroup().getSectors())
	{
		if(sector->isVisible())
		{
			sceneDrawer->draw(*sector);
		}
	}

	scene.getPhysics().debugDraw();

	drawer->flush();
}

} // end namespace anki
