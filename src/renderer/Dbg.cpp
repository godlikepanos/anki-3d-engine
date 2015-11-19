// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Dbg.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/resource/ShaderResource.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Sector.h>
#include <anki/scene/Camera.h>
#include <anki/scene/Light.h>
#include <anki/util/Logger.h>
#include <anki/util/Enum.h>
#include <anki/misc/ConfigSet.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/util/Rtti.h>
#include <anki/Ui.h> /// XXX

namespace anki {

//==============================================================================
Dbg::Dbg(Renderer* r)
	: RenderingPass(r)
{}

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
	m_enabled = initializer.getNumber("dbg.enabled");
	enableBits(Flag::ALL);

	// Chose the correct color FAI
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().getDepthRt();
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::LOAD;
	if(m_r->getPps().getEnabled())
	{
		fbInit.m_colorAttachments[0].m_texture = m_r->getPps().getRt();
	}
	else
	{
		fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	}
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::LOAD;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	m_drawer = getAllocator().newInstance<DebugDrawer>();
	ANKI_CHECK(m_drawer->create(m_r));

	m_sceneDrawer = getAllocator().newInstance<SceneDebugDrawer>(m_drawer);

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Dbg::run(CommandBufferPtr& cmdb)
{
	Error err = ErrorCode::NONE;

	ANKI_ASSERT(m_enabled);

	cmdb->bindFramebuffer(m_fb);

	FrustumComponent& camFr = m_r->getActiveFrustumComponent();
	SceneNode& cam = camFr.getSceneNode();
	m_drawer->prepareDraw(cmdb);
	m_drawer->setViewProjectionMatrix(camFr.getViewProjectionMatrix());
	m_drawer->setModelMatrix(Mat4::getIdentity());
	//m_drawer->drawGrid();

	SceneGraph& scene = cam.getSceneGraph();

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

		if(bitsEnabled(Flag::SECTOR)
			&& node.tryGetComponent<PortalSectorComponent>())
		{
			m_sceneDrawer->draw(node);
		}

		return ErrorCode::NONE;
	});

	(void)err;

	if(0)
	{
		PhysicsDebugDrawer phyd(m_drawer);

		m_drawer->setModelMatrix(Mat4::getIdentity());
		phyd.drawWorld(scene._getPhysicsWorld());
	}

#if 0
	{
		static Bool firstTime = true;
		static UiInterfaceImpl* interface;
		static Canvas* canvas;

		if(firstTime)
		{
			firstTime = false;

			auto alloc = getAllocator();
			interface = alloc.newInstance<UiInterfaceImpl>(alloc);
			ANKI_CHECK(interface->init(&getGrManager(), &getResourceManager()));

			canvas = alloc.newInstance<Canvas>(interface);
			canvas->setDebugDrawEnabled();
		}

		cmdb->setViewport(10, 10, 1024, 1024);
		canvas->update(0.1);
		interface->beginRendering(cmdb);
		canvas->paint();
		interface->endRendering();
		cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}
#endif

#if 1
	{
		CollisionDebugDrawer cd(m_drawer);

		Array<Vec3, 4> poly;
		poly[0] = Vec3(0.0, 0.0, 0.0);
		poly[1] = Vec3(2.5, 0.0, 0.0);
		poly[2] = Vec3(2.0, 4.9, 0.0);
		poly[3] = Vec3(0.5, 3.9, 0.0);

		Mat4 trf(Vec4(1.2, 14.0, 1.1, 1.0), Mat3(Euler(toRad(-120.0), toRad(35.0),
			toRad(85.0))), 1.0);

		Array<Vec3, 4> polyw;
		polyw[0] = trf.transform(poly[0]);
		polyw[1] = trf.transform(poly[1]);
		polyw[2] = trf.transform(poly[2]);
		polyw[3] = trf.transform(poly[3]);

		m_drawer->setModelMatrix(Mat4::getIdentity());
		m_drawer->drawLine(polyw[0], polyw[1], Vec4(1.0));
		m_drawer->drawLine(polyw[1], polyw[2], Vec4(0.8));
		m_drawer->drawLine(polyw[2], polyw[3], Vec4(0.6));
		m_drawer->drawLine(polyw[3], polyw[0], Vec4(0.4));



		Vec3 edge0 = polyw[2] - polyw[1];
		Vec3 edge1 = polyw[3] - polyw[2];

		Vec3 xAxis = edge0;
		xAxis.normalize();
		Vec3 zAxis = edge0.cross(edge1);
		zAxis.normalize();
		Vec3 yAxis = zAxis.cross(xAxis);

		Mat3 rot;
		rot.setColumns(xAxis.xyz(), yAxis.xyz(), zAxis.xyz());

		Mat3 invRot = rot.getInverse();

		Array<Vec3, 8> polyl;
		for(U i = 0; i < 4; ++i)
		{
			polyl[i] = invRot * polyw[i];
		}

		m_drawer->drawLine(polyl[0], polyl[1], Vec4(1.0));
		m_drawer->drawLine(polyl[1], polyl[2], Vec4(0.8));
		m_drawer->drawLine(polyl[2], polyl[3], Vec4(0.6));
		m_drawer->drawLine(polyl[3], polyl[0], Vec4(0.4));


		for(U i = 4; i < 8; ++i)
		{
			polyl[i] = polyl[i - 4] + Vec3(0.0, 0.0, 10.0);
		}

		Obb obb;
		obb.setFromPointCloud(&polyl[0], 8, sizeof(Vec3), sizeof(polyl));

		obb.transform(Transform(Vec4(0.0), Mat3x4(rot), 1.0));

		obb.accept(cd);
	}
#endif

	return m_drawer->flush();
}

//==============================================================================
Bool Dbg::getDepthTestEnabled() const
{
	return m_drawer->getDepthTestEnabled();
}

//==============================================================================
void Dbg::setDepthTestEnabled(Bool enable)
{
	m_drawer->setDepthTestEnabled(enable);
}

//==============================================================================
void Dbg::switchDepthTestEnabled()
{
	Bool enabled = m_drawer->getDepthTestEnabled();
	m_drawer->setDepthTestEnabled(!enabled);
}

} // end namespace anki
