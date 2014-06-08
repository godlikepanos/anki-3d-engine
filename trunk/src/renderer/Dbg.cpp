// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ProgramResource.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Dbg::~Dbg()
{}

//==============================================================================
void Dbg::init(const ConfigSet& initializer)
{
	m_enabled = initializer.get("dbg.enabled");
	enableBits(DF_ALL);

	try
	{
		GlManager& gl = GlManagerSingleton::get();
		GlJobChainHandle jobs(&gl);

		// Chose the correct color FAI
		if(m_r->getPps().getEnabled())
		{
			m_fb = GlFramebufferHandle(jobs, 
				{{m_r->getPps()._getRt(), GL_COLOR_ATTACHMENT0},
				{m_r->getMs()._getDepthRt(), GL_DEPTH_ATTACHMENT}});
		}
		else
		{
			m_fb = GlFramebufferHandle(jobs, 
				{{m_r->getIs()._getRt(), GL_COLOR_ATTACHMENT0},
				{m_r->getMs()._getDepthRt(), GL_DEPTH_ATTACHMENT}});
		}

		m_drawer.reset(new DebugDrawer);
		m_sceneDrawer.reset(new SceneDebugDrawer(m_drawer.get()));

		jobs.flush();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}
}

//==============================================================================
void Dbg::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	SceneGraph& scene = m_r->getSceneGraph();

	m_fb.bind(jobs, false);
	jobs.enableBlend(true);
	jobs.enableDepthTest(m_depthTest);

	m_drawer->prepareDraw(jobs);
	m_drawer->setViewProjectionMatrix(
		scene.getActiveCamera().getViewProjectionMatrix());
	m_drawer->setModelMatrix(Mat4::getIdentity());
	//drawer->drawGrid();

	scene.iterateSceneNodes([&](SceneNode& node)
	{
		SpatialComponent* sp = node.tryGetComponent<SpatialComponent>();
		if(bitsEnabled(DF_SPATIAL) && sp)
		{
			m_sceneDrawer->draw(node);
		}
	});

	// Draw sectors
	for(const Sector* sector : scene.getSectorGroup().getSectors())
	{
		//if(sector->isVisible())
		{
			if(bitsEnabled(DF_SECTOR))
			{
				m_sceneDrawer->draw(*sector);
			}
		}
	}

	// Physics
	if(bitsEnabled(DF_PHYSICS))
	{
		scene.getPhysics().debugDraw();
	}

	// XXX
#if 0
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
#endif

	jobs.enableBlend(false);
	jobs.enableDepthTest(false);

	m_drawer->flush();
	m_drawer->finishDraw();
}

} // end namespace anki
