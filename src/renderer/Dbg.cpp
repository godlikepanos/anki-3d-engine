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
		Spatial* sp = node->getSpatial();
		if(bitsEnabled(DF_SPATIAL) && sp)
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

	// XXX
	{
		SceneNode& sn = scene.findSceneNode("horse");
		Vec3 pos = sn.getMovable()->getWorldTransform().getOrigin();

		Vec4 posclip = r->getViewProjectionMatrix() * Vec4(pos, 1.0);
		Vec2 posndc = (posclip.xyz() / posclip.w()).xy();

		drawer->setModelMatrix(Mat4::getIdentity());
		drawer->setViewProjectionMatrix(Mat4::getIdentity());

		//drawer->drawLine(Vec3(0.0), posndc, Vec4(1.0));

		Vec2 dist = -posndc;
		F32 len = dist.getLength();
		dist /= len;

		U count = 1;

		while(count-- != 0)
		{
			Vec2 foopos = posndc + dist * (len * 2.0);

			drawer->drawLine(Vec3(posndc, 0.0), Vec3(foopos, 0.0), 
				Vec4(1.0, 0.0, 1.0, 1.0));
		}

	}

	drawer->flush();
}

} // end namespace anki
