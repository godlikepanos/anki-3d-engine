// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderResource.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/util/Logger.h"
#include "anki/util/Enum.h"
#include "anki/misc/ConfigSet.h"
#include "anki/collision/ConvexHullShape.h"
#include "anki/util/Rtti.h"

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

	GrManager& gl = getGrManager();
	CommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err)
	{
		return err;
	}

	// Chose the correct color FAI
	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs()._getDepthRt();
	if(m_r->getPps().getEnabled())
	{
		fbInit.m_colorAttachments[0].m_texture = m_r->getPps()._getRt();
	}
	else
	{
		fbInit.m_colorAttachments[0].m_texture = m_r->getIs()._getRt();
	}
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::LOAD;

	err = m_fb.create(cmdb, fbInit);
	if(!err)
	{
		m_drawer = getAllocator().newInstance<DebugDrawer>();
	}

	if(!err)
	{
		err = m_drawer->create(m_r);
	}

	if(!err)
	{
		m_sceneDrawer = getAllocator().newInstance<SceneDebugDrawer>(m_drawer);
	}

	if(!err)
	{
		cmdb.finish();
	}

	return err;
}

//==============================================================================
Error Dbg::run(CommandBufferHandle& cmdb)
{
	Error err = ErrorCode::NONE;

	ANKI_ASSERT(m_enabled);

	SceneGraph& scene = m_r->getSceneGraph();

	m_fb.bind(cmdb);
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

#if 0
	{
		SceneNode& sn = scene.findSceneNode("plight0");
		SpatialComponent& sp = sn.getComponent<SpatialComponent>();
		const CollisionShape& cs = sp.getSpatialCollisionShape();
		const Sphere& sphere = dcast<const Sphere&>(cs);
		F32 r = sphere.getRadius();

		Mat4 v = cam.getComponent<FrustumComponent>().getViewMatrix();
		Mat4 p = cam.getComponent<FrustumComponent>().getProjectionMatrix();
		Mat4 vp = 
			cam.getComponent<FrustumComponent>().getViewProjectionMatrix();

		Transform t = cam.getComponent<MoveComponent>().getWorldTransform();
		Mat4 trf(t);
		Mat3x4 rot = cam.getComponent<MoveComponent>().getWorldTransform().getRotation();

		CollisionDebugDrawer cd(m_drawer);

		m_drawer->setModelMatrix(Mat4::getIdentity());

		cs.accept(cd);

		m_drawer->setViewProjectionMatrix(Mat4::getIdentity());
		m_drawer->setModelMatrix(Mat4::getIdentity());

		Vec4 a = vp * sphere.getCenter().xyz1();
		F32 w = a.w();
		a /= a.w();

		
		Vec2 rr;
		Vec4 n = t.getOrigin() - sphere.getCenter();
		Vec4 right = rot.getColumn(1).xyz0().cross(n);
		right.normalize();

		Vec4 b = sphere.getCenter() + right * r;
		b = vp * b.xyz1();
		b /= b.w();
		rr.x() = b.x() - a.x();

		Vec4 top = n.cross(rot.getColumn(0).xyz0());
		top.normalize();

		b = sphere.getCenter() + top * r;
		b = vp * b.xyz1();
		b /= b.w();
		rr.y() = b.y() - a.y();

		m_drawer->begin(GL_LINES);
		m_drawer->setColor(Vec4(1.0, 1.0, 1.0, 1.0));

		m_drawer->pushBackVertex(a.xyz());
		m_drawer->pushBackVertex(a.xyz() + Vec3(rr.x(), 0, 0));

		m_drawer->pushBackVertex(a.xyz());
		m_drawer->pushBackVertex(a.xyz() + Vec3(0, rr.y(), 0));

		//m_drawer->pushBackVertex(Vec3(0.0, 0.0, 0.5));
		//m_drawer->pushBackVertex(Vec3(1.0, 1.0, 0.5));
		m_drawer->end();
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
