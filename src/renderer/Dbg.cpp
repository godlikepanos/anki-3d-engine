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
	//drawer->drawGrid();

	PointLight* deleteme;

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
	{
		SceneNode* node = *it;
		if(!node->getSpatial())
		{
			continue;
		}

		sceneDrawer->draw(*node);

		// XXX
		if(node->getLight())
		{
			CollisionDebugDrawer cdd(drawer.get());
			Light& l = *node->getLight();
			l.getLightType();
			PointLight& pl = static_cast<PointLight&>(l);
			deleteme = &pl;
			pl.getSphere().accept(cdd);
		}
	}

	for(const Sector* sector : scene.sectors)
	{
		sceneDrawer->draw(sector->getOctree());
	}

	// XXX
	drawer->setViewProjectionMatrix(r->getViewProjectionMatrix());
	drawer->setModelMatrix(Mat4::getIdentity());

	Camera* camera1 = static_cast<Camera*>(scene.findSceneNode("camera1"));

	F32 fx = static_cast<PerspectiveCamera*>(camera1)->getFovX();
	F32 fy = static_cast<PerspectiveCamera*>(camera1)->getFovY();

	//std::cout << fx << " " << fy << std::endl;

	Vec3 a(0.0);
	F32 s, c;
	Math::sinCos(Math::PI / 2 - fx / 2, s, c);
	Vec3 b(c, 0.0, -s);

	a.transform(camera1->getWorldTransform());
	b.transform(camera1->getWorldTransform());

	drawer->drawLine(a, b, Vec4(1));

	Math::sinCos(fy / 2, s, c);
	b = Vec3(0.0, s, -c);
	b.transform(camera1->getWorldTransform());

	drawer->drawLine(a, b, Vec4(1));

#if 0
	{
	PerspectiveFrustum fr =
		static_cast<const PerspectiveFrustum&>(scene.getActiveCamera().getFrustumable()->getFrustum());

	fr.setTransform(Transform::getIdentity());

	CollisionDebugDrawer cdd(drawer.get());
	fr.accept(cdd);
	}

	for(U j = 0; j < 16; j++)
	{
		for(U i = 0; i < 1; i++)
		{
			Is::Tile& tile = r->getIs().tiles[j][i];

			Mat4 vmat = scene.getActiveCamera().getViewMatrix();

			CollisionDebugDrawer cdd(drawer.get());
			//tile.planes[Frustum::FP_LEFT].accept(cdd);
			//tile.planes[Frustum::FP_RIGHT].accept(cdd);
			//tile.planes[Frustum::FP_BOTTOM].accept(cdd);
			//tile.planes[Frustum::FP_TOP].accept(cdd);
		}
	}
#endif

#if 0
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

	//std::cout << (p1 - p0).getLength() << ", " << l << std::endl;

	//
	F32 r = s.getRadius();
	Vec3 e = cam.getWorldTransform().getOrigin();
	Vec4 c(s.getCenter(), 1.0);
	Vec4 a = Vec4(c.xyz() + cam.getWorldTransform().getRotation() * Vec3(r, 0.0, 0.0), 1.0);

	c = cam.getViewProjectionMatrix() * c;
	c /= c.w();

	a = cam.getViewProjectionMatrix() * a;
	a /= a.w();

	Vec2 circleCenter = c.xy();
	F32 circleRadius = (c.xy() - a.xy()).getLength();
	//

	drawer->setViewProjectionMatrix(Mat4::getIdentity());
	drawer->setModelMatrix(Mat4::getIdentity());

	/*drawer->drawLine(Vec3(p0.x(), p0.y(), 0.0),
		Vec3(p0.x(), p0.y(), 0.0) + Vec3(2.0 / w, 0.0, 0.0),
		Vec4(0.0, 1.0, 0.0, 0.0));

	drawer->drawLine(Vec3(p0.x(), p0.y(), 0.0),
		Vec3(p1.x(), p1.y(), 0.0), Vec4(0.0, 1.0, 1.0, 0.0));*/

	drawer->drawLine(Vec3(circleCenter, 0.0),
		Vec3(circleCenter, 0.0) + Vec3(circleRadius, 0.0, 0.0), Vec4(0.0, 1.0, 1.0, 0.0));
#endif
}

} // end namespace anki
