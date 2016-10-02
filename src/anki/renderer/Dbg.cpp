// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Dbg.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/DebugDrawer.h>
#include <anki/resource/ShaderResource.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/Sector.h>
#include <anki/scene/Light.h>
#include <anki/util/Logger.h>
#include <anki/util/Enum.h>
#include <anki/misc/ConfigSet.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/Ui.h> /// XXX
#include <anki/scene/SoftwareRasterizer.h> /// XXX

namespace anki
{

Dbg::Dbg(Renderer* r)
	: RenderingPass(r)
{
}

Dbg::~Dbg()
{
	if(m_drawer != nullptr)
	{
		getAllocator().deleteInstance(m_drawer);
	}
}

Error Dbg::init(const ConfigSet& initializer)
{
	m_enabled = initializer.getNumber("dbg.enabled");
	m_flags.set(DbgFlag::ALL);
	return ErrorCode::NONE;
}

Error Dbg::lazyInit()
{
	ANKI_ASSERT(!m_initialized);

	// RT
	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		DBG_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	m_drawer = getAllocator().newInstance<DebugDrawer>();
	ANKI_CHECK(m_drawer->init(m_r));

	return ErrorCode::NONE;
}

Error Dbg::run(RenderingContext& ctx)
{
	ANKI_ASSERT(m_enabled);

	if(!m_initialized)
	{
		ANKI_CHECK(lazyInit());
		m_initialized = true;
	}

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	FrustumComponent& camFrc = *ctx.m_frustumComponent;
	SceneNode& cam = camFrc.getSceneNode();
	m_drawer->prepareFrame(cmdb);
	m_drawer->setViewProjectionMatrix(camFrc.getViewProjectionMatrix());
	m_drawer->setModelMatrix(Mat4::getIdentity());
	// m_drawer->drawGrid();

	SceneGraph& scene = cam.getSceneGraph();

	SceneDebugDrawer sceneDrawer(m_drawer);
	camFrc.getVisibilityTestResults().iterateAll([&](SceneNode& node) {
		if(&node == &cam)
		{
			return;
		}

		// Set position
		MoveComponent* mv = node.tryGetComponent<MoveComponent>();
		if(mv)
		{
			m_drawer->setModelMatrix(Mat4(mv->getWorldTransform()));
		}
		else
		{
			m_drawer->setModelMatrix(Mat4::getIdentity());
		}

		// Spatial
		if(m_flags.get(DbgFlag::SPATIAL_COMPONENT))
		{
			Error err = node.iterateComponentsOfType<SpatialComponent>([&](SpatialComponent& sp) -> Error {
				sceneDrawer.draw(sp);
				return ErrorCode::NONE;
			});
			(void)err;
		}

		// Frustum
		if(m_flags.get(DbgFlag::FRUSTUM_COMPONENT))
		{
			Error err = node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
				if(&frc != &camFrc)
				{
					sceneDrawer.draw(frc);
				}
				return ErrorCode::NONE;
			});
			(void)err;
		}

		// Sector/portal
		if(m_flags.get(DbgFlag::SECTOR_COMPONENT))
		{
			Error err = node.iterateComponentsOfType<SectorComponent>([&](SectorComponent& psc) -> Error {
				sceneDrawer.draw(psc);
				return ErrorCode::NONE;
			});

			err = node.iterateComponentsOfType<PortalComponent>([&](PortalComponent& psc) -> Error {
				sceneDrawer.draw(psc);
				return ErrorCode::NONE;
			});
			(void)err;
		}
	});

	if(m_flags.get(DbgFlag::PHYSICS))
	{
		PhysicsDebugDrawer phyd(m_drawer);

		m_drawer->setModelMatrix(Mat4::getIdentity());
		phyd.drawWorld(scene.getPhysicsWorld());
	}

#if 0
	{
		m_drawer->setViewProjectionMatrix(camFrc.getViewProjectionMatrix());
		m_drawer->setModelMatrix(Mat4::getIdentity());
		CollisionDebugDrawer cd(m_drawer);
		Mat4 proj = camFrc.getProjectionMatrix();

		Array<Plane, 6> planes;
		Array<Plane*, 6> pplanes = {&planes[0],
			&planes[1],
			&planes[2],
			&planes[3],
			&planes[4],
			&planes[5]};
		extractClipPlanes(proj, pplanes);

		planes[5].accept(cd);

		m_drawer->setViewProjectionMatrix(camFrc.getViewProjectionMatrix());
		m_drawer->setModelMatrix(Mat4::getIdentity());

		m_drawer->setColor(Vec4(0.0, 1.0, 1.0, 1.0));
		PerspectiveFrustum frc;
		const PerspectiveFrustum& cfrc =
			(const PerspectiveFrustum&)camFrc.getFrustum();
		frc.setAll(
			cfrc.getFovX(), cfrc.getFovY(), cfrc.getNear(), cfrc.getFar());
		cd.visit(frc);

		m_drawer->drawLine(Vec3(0.0), planes[5].getNormal().xyz() * 100.0, 
			Vec4(1.0));
	}
#endif

#if 0
	{
		m_drawer->setViewProjectionMatrix(Mat4::getIdentity());
		m_drawer->setModelMatrix(Mat4::getIdentity());
		Mat4 proj = camFrc.getProjectionMatrix();
		Mat4 view = camFrc.getViewMatrix();

		Array<Vec4, 12> ltriangle = {Vec4(0.0, 2.0, 2.0, 1.0),
			Vec4(4.0, 2.0, 2.0, 1.0),
			Vec4(0.0, 8.0, 2.0, 1.0),

			Vec4(0.0, 8.0, 2.0, 1.0),
			Vec4(4.0, 2.0, 2.0, 1.0),
			Vec4(4.0, 8.0, 2.0, 1.0),

			Vec4(0.9, 2.0, 1.9, 1.0),
			Vec4(4.9, 2.0, 1.9, 1.0),
			Vec4(0.9, 8.0, 1.9, 1.0),

			Vec4(0.9, 8.0, 1.9, 1.0),
			Vec4(4.9, 2.0, 1.9, 1.0),
			Vec4(4.9, 8.0, 1.9, 1.0)};

		SoftwareRasterizer r;
		r.init(getAllocator());
		r.prepare(
			view, proj, m_r->getTileCountXY().x(), m_r->getTileCountXY().y());
		r.draw(&ltriangle[0][0], 12, sizeof(Vec4));

		/*m_drawer->begin(PrimitiveTopology::TRIANGLES);
		U count = 0;
		for(U y = 0; y < m_r->getTileCountXY().y(); ++y)
		{
			for(U x = 0; x < m_r->getTileCountXY().x(); ++x)
			{
				F32 d = r.m_zbuffer[y * m_r->getTileCountXY().x() + x].get()
					/ F32(MAX_U32);

				if(d < 1.0)
				{
					F32 zNear = camFrc.getFrustum().getNear();
					F32 zFar = camFrc.getFrustum().getFar();
					F32 ld =
						(2.0 * zNear) / (zFar + zNear - d * (zFar - zNear));
					m_drawer->setColor(Vec4(ld));

					++count;
					Vec2 min(F32(x) / m_r->getTileCountXY().x(),
						F32(y) / m_r->getTileCountXY().y());

					Vec2 max(F32(x + 1) / m_r->getTileCountXY().x(),
						F32(y + 1) / m_r->getTileCountXY().y());

					min = min * 2.0 - 1.0;
					max = max * 2.0 - 1.0;

					m_drawer->pushBackVertex(Vec3(min.x(), min.y(), 0.0));
					m_drawer->pushBackVertex(Vec3(max.x(), min.y(), 0.0));
					m_drawer->pushBackVertex(Vec3(min.x(), max.y(), 0.0));

					m_drawer->pushBackVertex(Vec3(min.x(), max.y(), 0.0));
					m_drawer->pushBackVertex(Vec3(max.x(), min.y(), 0.0));
					m_drawer->pushBackVertex(Vec3(max.x(), max.y(), 0.0));
				}
			}
		}
		m_drawer->end();*/

		m_drawer->setViewProjectionMatrix(camFrc.getViewProjectionMatrix());
		Vec3 offset(0.0, 0.0, 0.0);
		m_drawer->setColor(Vec4(0.5));
		m_drawer->begin(PrimitiveTopology::TRIANGLES);
		for(U i = 0; i < ltriangle.getSize() / 3; ++i)
		{
			m_drawer->pushBackVertex(ltriangle[i * 3 + 0].xyz());
			m_drawer->pushBackVertex(ltriangle[i * 3 + 1].xyz());
			m_drawer->pushBackVertex(ltriangle[i * 3 + 2].xyz());
		}
		m_drawer->end();

		SceneNode& node = scene.findSceneNode("Lamp");
		SpatialComponent& spc = node.getComponent<SpatialComponent>();
		Aabb nodeAabb = spc.getAabb();

		Bool inside =
			r.visibilityTest(spc.getSpatialCollisionShape(), nodeAabb);

		if(inside)
		{
			m_drawer->setColor(Vec4(1.0, 0.0, 0.0, 1.0));
			CollisionDebugDrawer cd(m_drawer);
			nodeAabb.accept(cd);
		}
	}
#endif

#if 0
	{
		CollisionDebugDrawer cd(m_drawer);

		Array<Vec3, 4> poly;
		poly[0] = Vec3(0.0, 0.0, 0.0);
		poly[1] = Vec3(2.5, 0.0, 0.0);

		Mat4 trf(Vec4(147.392776, -12.132728, 16.607138, 1.0),
			Mat3(Euler(toRad(45.0), toRad(0.0), toRad(120.0))),
			1.0);

		Array<Vec3, 4> polyw;
		polyw[0] = trf.transform(poly[0]);
		polyw[1] = trf.transform(poly[1]);

		m_drawer->setModelMatrix(Mat4::getIdentity());
		m_drawer->drawLine(polyw[0], polyw[1], Vec4(1.0));

		Vec4 p0 = camFrc.getViewMatrix() * polyw[0].xyz1();
		p0.w() = 0.0;
		Vec4 p1 = camFrc.getViewMatrix() * polyw[1].xyz1();
		p1.w() = 0.0;

		Vec4 r = p1 - p0;
		r.normalize();

		Vec4 a = camFrc.getProjectionMatrix() * p0.xyz1();
		a /= a.w();

		Vec4 i;
		if(r.z() > 0)
		{
			// Plane near(Vec4(0, 0, -1, 0), camFrc.getFrustum().getNear() +
			// 0.001);
			// Bool in = near.intersectRay(p0, r * 100000.0, i);
			i.z() = -camFrc.getFrustum().getNear();
			F32 t = (i.z() - p0.z()) / r.z();
			i.x() = p0.x() + t * r.x();
			i.y() = p0.y() + t * r.y();

			i = camFrc.getProjectionMatrix() * i.xyz1();
			i /= i.w();
		}
		else
		{
			i = camFrc.getProjectionMatrix() * (r * 100000.0).xyz1();
			i /= i.w();
		}

		/*r *= 0.01;
		Vec4 b = polyw[0].xyz0() + r;
		b = camFrc.getViewProjectionMatrix() * b.xyz1();
		Vec4 d = b / b.w();*/

		m_drawer->setViewProjectionMatrix(Mat4::getIdentity());
		m_drawer->drawLine(
			Vec3(a.xy(), 0.1), Vec3(i.xy(), 0.1), Vec4(1.0, 0, 0, 1));
	}
#endif

	m_drawer->finishFrame();
	cmdb->endRenderPass();
	return ErrorCode::NONE;
}

Bool Dbg::getDepthTestEnabled() const
{
	return m_drawer->getDepthTestEnabled();
}

void Dbg::setDepthTestEnabled(Bool enable)
{
	m_drawer->setDepthTestEnabled(enable);
}

void Dbg::switchDepthTestEnabled()
{
	Bool enabled = m_drawer->getDepthTestEnabled();
	m_drawer->setDepthTestEnabled(!enabled);
}

} // end namespace anki
