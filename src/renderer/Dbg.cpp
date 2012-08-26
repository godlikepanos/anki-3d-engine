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

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().enable(GL_DEPTH_TEST);

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

	for(const Sector* sector : scene.sectors)
	{
		sceneDrawer->draw(sector->getOctree());
	}

	// XXX Remove these
	CollisionDebugDrawer cdd(drawer.get());
	Sphere s(Vec3(90.0, 4.0, 0.0), 2.0);
	s.accept(cdd);

	Camera& cam = scene.getActiveCamera();

	Vec4 p0(s.getCenter(), 1.0);
	Vec4 p1(s.getCenter() + Vec3(0.0, 2.0, 0.0), 1.0);
	p0 = cam.getViewProjectionMatrix() * p0;
	F32 w = p0.w();
	p0 /= p0.w();
	p1 = cam.getViewProjectionMatrix() * p1;
	F32 p1w = p1.w();
	p1 /= p1.w();

	Vec4 l = cam.getViewProjectionMatrix() * Vec4(0.0, 0.0, 0.0, 1.0);
	l /= l.w();

	std::cout << (p1 - p0).getLength() << ", " << l << std::endl;

	drawer->setViewProjectionMatrix(Mat4::getIdentity());
	drawer->setModelMatrix(Mat4::getIdentity());

	drawer->drawLine(Vec3(p0.x(), p0.y(), 0.0),
		Vec3(p0.x(), p0.y(), 0.0) + Vec3(2.0 / w, 0.0, 0.0),
		Vec4(0.0, 1.0, 0.0, 0.0));

	drawer->drawLine(Vec3(p0.x(), p0.y(), 0.0),
		Vec3(p1.x(), p1.y(), 0.0), Vec4(0.0, 1.0, 1.0, 0.0));
}

} // end namespace anki
