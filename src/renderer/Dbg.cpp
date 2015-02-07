// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ProgramResource.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/util/Logger.h"
#include "anki/util/Enum.h"
#include "anki/misc/ConfigSet.h"
#include "anki/collision/ConvexHullShape.h"

namespace anki {

//==============================================================================
Dbg::~Dbg()
{
	if(m_drawer != nullptr)
	{
		getAllocator().deleteInstance(m_drawer);
	}

	if(m_sceneDrawer != nullptr)
	{
		getAllocator().deleteInstance(m_sceneDrawer);
	}
}

//==============================================================================
Error Dbg::init(const ConfigSet& initializer)
{
	Error err = ErrorCode::NONE;

	m_enabled = initializer.get("dbg.enabled");
	enableBits(Flag::ALL);

	GlDevice& gl = getGlDevice();
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err)
	{
		return err;
	}

	// Chose the correct color FAI
	if(m_r->getPps().getEnabled())
	{
		err = m_fb.create(cmdb, 
			{{m_r->getPps()._getRt(), GL_COLOR_ATTACHMENT0},
			{m_r->getMs()._getDepthRt(), GL_DEPTH_ATTACHMENT}});
	}
	else
	{
		err = m_fb.create(cmdb, 
			{{m_r->getIs()._getRt(), GL_COLOR_ATTACHMENT0},
			{m_r->getMs()._getDepthRt(), GL_DEPTH_ATTACHMENT}});
	}

	if(!err)
	{
		m_drawer = getAllocator().newInstance<DebugDrawer>();
		if(m_drawer == nullptr)
		{
			err = ErrorCode::OUT_OF_MEMORY;
		}
	}

	if(!err)
	{
		err = m_drawer->create(m_r);
	}

	if(!err)
	{
		m_sceneDrawer = getAllocator().newInstance<SceneDebugDrawer>(m_drawer);
		if(m_sceneDrawer == nullptr)
		{
			err = ErrorCode::OUT_OF_MEMORY;
		}
	}

	if(!err)
	{
		cmdb.finish();
	}

	return err;
}

//==============================================================================
Error Dbg::run(GlCommandBufferHandle& cmdb)
{
	Error err = ErrorCode::NONE;

	ANKI_ASSERT(m_enabled);

	SceneGraph& scene = m_r->getSceneGraph();

	m_fb.bind(cmdb, false);
	cmdb.enableDepthTest(m_depthTest);

	Camera& cam = scene.getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
	m_drawer->prepareDraw(cmdb);
	m_drawer->setViewProjectionMatrix(camFr.getViewProjectionMatrix());
	m_drawer->setModelMatrix(Mat4::getIdentity());
	m_drawer->drawGrid();

	err = scene.iterateSceneNodes([&](SceneNode& node) -> Error
	{
		SpatialComponent* sp = node.tryGetComponent<SpatialComponent>();

		if(&cam.getComponent<SpatialComponent>() == sp)
		{
			return ErrorCode::NONE;
		}

		if(bitsEnabled(Flag::SPATIAL) && sp)
		{
			m_sceneDrawer->draw(node);
		}

		return ErrorCode::NONE;
	});

	(void)err;

	if(1)
	{
		PhysicsDebugDrawer phyd(m_drawer);

		m_drawer->setModelMatrix(Mat4::getIdentity());
		phyd.drawWorld(scene._getPhysicsWorld());
	}

#if 1
	{
		m_drawer->setModelMatrix(Mat4::getIdentity());
		Sphere s(Vec4(2.0, 1.4, 0.6, 0.0), 6.0);
		CollisionDebugDrawer cd(m_drawer);
		cd.visit(s);
	}
#endif

#if 0
	{
		ConvexHullShape hull;
		Vec4 storage[] = {
			Vec4(1.0, 1.0, 1.0, 0.0),
			Vec4(1.0, 1.0, -1.0, 0.0),
			Vec4(-1.0, 1.0, -1.0, 0.0),
			Vec4(-1.0, -1.0, 1.0, 0.0),
		};

		SceneNode& sn = scene.findSceneNode("horse");
		MoveComponent& move = sn.getComponent<MoveComponent>();

		hull.initStorage(storage, 4);

		Sphere s(Vec4(0.0), 1.0);

		hull.transform(move.getWorldTransform());

		Gjk gjk;
		Bool collide = gjk.intersect(s, hull);

		if(collide)
		{
			m_drawer->setColor(Vec4(1.0, 0.0, 0.0, 1.0));
		}
		else
		{
			m_drawer->setColor(Vec4(1.0));
		}

		m_drawer->setModelMatrix(Mat4::getIdentity());
		CollisionDebugDrawer cd(m_drawer);

		cd.visit(hull);
		cd.visit(s);

		Aabb aabb;
		hull.computeAabb(aabb);
		cd.visit(aabb);
	}
#endif

	err = m_drawer->flush();

	if(!err)
	{
		m_drawer->finishDraw();
		cmdb.enableDepthTest(false);
	}

	return err;
}

} // end namespace anki
