#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/Scene.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Dbg::~Dbg()
{}

//==============================================================================
void Dbg::init(const Renderer::Initializer& initializer)
{
	enabled = initializer.dbg.enabled;

	try
	{
		fbo.create();
		//fbo.setColorAttachments({&r->getPps().getPostPassFai()});
		fbo.setColorAttachments({&r->getIs().getFai()});
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
	if(!enabled)
	{
		return;
	}

	Scene& scene = r->getScene();

	fbo.bind();

	drawer->setViewProjectionMatrix(r->getViewProjectionMatrix());
	drawer->setModelMatrix(Mat4::getIdentity());
	drawer->drawGrid();

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
	{
		SceneNode* node = *it;
		if(!node->getSpatial())
		{
			continue;
		}

		sceneDrawer->draw(*node);
	}
}

} // end namespace
