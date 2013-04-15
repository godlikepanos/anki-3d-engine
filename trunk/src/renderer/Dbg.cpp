#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
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
	enableBits(DF_ALL);

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

	fbo.bind();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().enable(GL_DEPTH_TEST, depthTest);

	drawer->setViewProjectionMatrix(r->getViewProjectionMatrix());
	drawer->setModelMatrix(Mat4::getIdentity());
	//drawer->drawGrid();

	SceneGraph& scene = r->getSceneGraph();

	for(auto it = scene.getSceneNodesBegin();
		it != scene.getSceneNodesEnd(); it++)
	{
		SceneNode* node = *it;
		/*Spatial* sp = node->getSpatial();
		if(bitsEnabled(DF_SPATIAL) && sp)*/
		{
			sceneDrawer->draw(*node);
		}
	}

	// Draw sectors
	for(const Sector* sector : scene.getSectorGroup().getSectors())
	{
		//if(sector->isVisible())
		{
			if(bitsEnabled(DF_SECTOR))
			{
				sceneDrawer->draw(*sector);
			}

			if(bitsEnabled(DF_OCTREE))
			{
				sceneDrawer->draw(sector->getOctree());
			}
		}
	}

	// Physics
	if(bitsEnabled(DF_PHYSICS))
	{
		scene.getPhysics().debugDraw();
	}

	drawer->flush();
}

} // end namespace anki