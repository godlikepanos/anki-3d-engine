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
		SpatialComponent* sp = node.tryGetComponent<SpatialComponent>();
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

	// XXX
	if(0)
	{
		Vec3 tri[3] = {
			Vec3{10.0 , 7.0, -2.0}, 
			Vec3{-10.0 , 7.0, -2.0}, 
			Vec3{-10.0 , 0.0, -2.0}};

		Mat4 mvp = scene.getActiveCamera().getViewProjectionMatrix();

		Array<Vec4, 3> projtri = {{
			mvp * Vec4(tri[0], 1.0), 
			mvp * Vec4(tri[1], 1.0), 
			mvp * Vec4(tri[2], 1.0)}};

		Array<Vec4, 3> cliptri;

		for(int i = 0; i < 3; i++)
		{
			Vec4 proj = projtri[i];
			cliptri[i] = proj / fabs(proj.w());
		}

		Vec3 a = cliptri[1].xyz() - cliptri[0].xyz();
		Vec3 b = cliptri[2].xyz() - cliptri[1].xyz();
		//Vec2 c = a.xy() * b.yx();
		//Bool frontfacing = (c.x() - c.y()) > 0.0;
		Vec3 cross = a.cross(b);
		Bool frontfacing = cross.z() >= 0.0;

		if(frontfacing)
		{
			drawer->setColor(Vec4(1.0));
			drawer->setViewProjectionMatrix(
				scene.getActiveCamera().getViewProjectionMatrix());
			drawer->setModelMatrix(Mat4::getIdentity());

			drawer->pushBackVertex(tri[0]);
			drawer->pushBackVertex(tri[1]);
			drawer->pushBackVertex(tri[1]);
			drawer->pushBackVertex(tri[2]);
			drawer->pushBackVertex(tri[2]);
			drawer->pushBackVertex(tri[0]);
		}

		{
			for(Vec4& v : cliptri)
			{
				std::cout << v.toString() << "\n";
			}

			std::cout << std::endl;

			std::cout << a.toString() << ", " << b.toString() 
				//<< ", " << c.toString() << ", " 
				//<< (c.x() - c.y()) 
				<< ", ff:" << (U)frontfacing
				<< std::endl;

			std::cout << std::endl;
		}

		{
			drawer->setColor(Vec4(0.0, 1.0, 0.0, 1.0));
			drawer->setViewProjectionMatrix(Mat4::getIdentity());
			drawer->setModelMatrix(Mat4::getIdentity());

			F32 z = 0.0;

			drawer->pushBackVertex(Vec3(cliptri[0].xy(), z));
			drawer->pushBackVertex(Vec3(cliptri[1].xy(), z));
			drawer->pushBackVertex(Vec3(cliptri[1].xy(), z));
			drawer->pushBackVertex(Vec3(cliptri[2].xy(), z));
		}
	}
	// XXX

	drawer->flush();
}

} // end namespace anki
