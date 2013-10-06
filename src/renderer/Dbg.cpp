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
void Dbg::init(const RendererInitializer& initializer)
{
	enabled = initializer.get("dbg.enabled");
	enableBits(DF_ALL);

	try
	{
		fbo.create();

		// Chose the correct color FAI
		if(r->getPps().getEnabled())
		{
			fbo.setColorAttachments({&r->getPps().getFai()});
		}
		else
		{
			fbo.setColorAttachments({&r->getIs().getFai()});
		}

		fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());

		if(!fbo.isComplete())
		{
			throw ANKI_EXCEPTION("FBO is incomplete");
		}

		drawer.reset(new DebugDrawer);
		sceneDrawer.reset(new SceneDebugDrawer(drawer.get()));
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}
}

//==============================================================================
void Dbg::run()
{
	ANKI_ASSERT(enabled);

	fbo.bind();
	SceneGraph& scene = r->getSceneGraph();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().enable(GL_DEPTH_TEST, depthTest);

	drawer->setViewProjectionMatrix(
		scene.getActiveCamera().getViewProjectionMatrix());
	drawer->setModelMatrix(Mat4::getIdentity());
	//drawer->drawGrid();

	scene.iterateSceneNodes([&](SceneNode& node)
	{
		SpatialComponent* sp = node.getSpatialComponent();
		if(bitsEnabled(DF_SPATIAL) && sp)
		{
			sceneDrawer->draw(node);
		}
	});

	// Draw sectors
	for(const Sector* sector : scene.getSectorGroup().getSectors())
	{
		//if(sector->isVisible())
		{
			if(bitsEnabled(DF_SECTOR))
			{
				sceneDrawer->draw(*sector);
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
