// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Pps.h"
#include "anki/resource/ShaderResource.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Sector.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/util/Logger.h"
#include "anki/util/Enum.h"
#include "anki/misc/ConfigSet.h"
#include "anki/collision/ConvexHullShape.h"
#include "anki/util/Rtti.h"
#include "anki/Ui.h" /// XXX
#include "anki/scene/Clusterer.h" /// XXX

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

	return ErrorCode::NONE;
}

//==============================================================================
Error Dbg::run(CommandBufferPtr& cmdb)
{
	Error err = ErrorCode::NONE;

	ANKI_ASSERT(m_enabled);

	cmdb->bindFramebuffer(m_fb);

	SceneNode& cam = m_r->getActiveCamera();
	FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
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

#if 0
	{
		Vec4 origin(0.0);

		PerspectiveFrustum fr;
		const F32 ang = 55.0;
		F32 far = 200.0;
		fr.setAll(toRad(ang) * m_r->getAspectRatio(), toRad(ang), 1.0, far);
		fr.resetTransform(Transform(origin, Mat3x4::getIdentity(), 1.0));

		Clusterer c(getAllocator());

		c.init(m_r->getWidth() / 64, m_r->getHeight() / 64, 20);
		//c.init(5, 3, 10);
		c.prepare(fr, SArray<Vec2>());

		CollisionDebugDrawer cd(m_drawer);
		m_drawer->setColor(Vec4(1.0, 0.0, 0.0, 1.0));
		fr.accept(cd);
		/*m_drawer->begin(PrimitiveTopology::LINES);
		m_drawer->pushBackVertex(Vec3(0.f));
		m_drawer->pushBackVertex(Vec3(0.f, 0.f, -10.f));
		m_drawer->end();*/


		U k = 0;
		while(0)
		{
			F32 neark = c.calcNear(k);
			if(neark >= c.m_far)
			{
				break;
			}
			Plane p;
			p.setNormal(Vec4(0.0, 0.0, -1.0, 0.0));
			p.setOffset(neark);
			p.accept(cd);
			++k;
		}

		SceneNode& lnode = scene.findSceneNode("spot0");
		SpatialComponent& sp = lnode.getComponent<SpatialComponent>();

		m_drawer->setColor(Vec4(0.0, 0.0, 1.0, 1.0));
		sp.getSpatialCollisionShape().accept(cd);

		ClustererTestResult rez;
		c.initTempTestResults(getAllocator(), rez);

		c.bin(sp.getSpatialCollisionShape(), rez);

		m_drawer->setColor(Vec4(1.0));
		for(U z = 0; z < c.m_counts[2]; ++z)
		{
			for(U y = 0; y < c.m_counts[1]; ++y)
			{
				for(U x = 0; x < c.m_counts[0]; ++x)
				{
					auto& cluster = c.cluster(x, y, z);
					Aabb box(cluster.m_min.xyz0(), cluster.m_max.xyz0());
					m_drawer->setColor(Vec4(1.0));

					Bool found = false;
					auto it = rez.getClustersBegin();
					auto end = rez.getClustersEnd();
					for(; it != end; ++it)
					{
						if((*it)[0] == x && (*it)[1] == y && (*it)[2] == z)
						{
							m_drawer->setColor(Vec4(1.0, 0.0, 0.0, 1.0));
							found = true;
							break;
						}
					}

					if(found)
					box.accept(cd);
				}
			}
		}
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
