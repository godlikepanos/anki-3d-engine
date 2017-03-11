// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/Scene.h>
#include <anki/util/Logger.h>
#include <anki/util/Enum.h>
#include <anki/misc/ConfigSet.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/Ui.h> /// XXX
#include <anki/renderer/Clusterer.h> /// XXX

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
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		DBG_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1));

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_texture = m_r->getMs().m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;

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

	m_drawer->prepareFrame(cmdb);
	m_drawer->setViewProjectionMatrix(ctx.m_viewProjMat);
	m_drawer->setModelMatrix(Mat4::getIdentity());
	// m_drawer->drawGrid();

	const SceneGraph* scene = nullptr;

	SceneDebugDrawer sceneDrawer(m_drawer);
	ctx.m_visResults->iterateAll([&](const SceneNode& node) {
		// Get the scenegraph
		if(scene == nullptr)
		{
			scene = &node.getSceneGraph();
		}

		/*if(&node == &cam)
		{
			return;
		}*/

		// Set position
		const MoveComponent* mv = node.tryGetComponent<MoveComponent>();
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
			Error err = node.iterateComponentsOfType<const SpatialComponent>([&](const SpatialComponent& sp) -> Error {
				sceneDrawer.draw(sp);
				return ErrorCode::NONE;
			});
			(void)err;
		}

		// Frustum
		if(m_flags.get(DbgFlag::FRUSTUM_COMPONENT))
		{
			Error err = node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
				/*if(&frc != &camFrc)
				{
					sceneDrawer.draw(frc);
				}*/
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

		// Decal
		if(m_flags.get(DbgFlag::DECAL_COMPONENT))
		{
			Error err = node.iterateComponentsOfType<DecalComponent>([&](DecalComponent& psc) -> Error {
				sceneDrawer.draw(psc);
				return ErrorCode::NONE;
			});
			(void)err;
		}
	});

	if(m_flags.get(DbgFlag::PHYSICS) && scene)
	{
		PhysicsDebugDrawer phyd(m_drawer);

		m_drawer->setModelMatrix(Mat4::getIdentity());
		phyd.drawWorld(scene->getPhysicsWorld());
	}

#if 0
	{
		m_drawer->setViewProjectionMatrix(camFrc.getViewProjectionMatrix());
		m_drawer->setModelMatrix(Mat4::getIdentity());
		CollisionDebugDrawer cd(m_drawer);
		Mat4 proj = camFrc.getProjectionMatrix();

		m_drawer->setViewProjectionMatrix(camFrc.getViewProjectionMatrix());

		Sphere s(Vec4(1.2, 2.0, -1.1, 0.0), 2.1);

		s.accept(cd);

		Transform trf = scene.findSceneNode("light0").getComponent<MoveComponent>().getWorldTransform();
		Vec4 rayOrigin = trf.getOrigin();
		Vec3 rayDir = -trf.getRotation().getZAxis().getNormalized();
		m_drawer->setModelMatrix(Mat4::getIdentity());
		m_drawer->drawLine(rayOrigin.xyz(), rayOrigin.xyz() + rayDir.xyz() * 10.0, Vec4(1.0, 1.0, 1.0, 1.0));

		Array<Vec4, 2> intersectionPoints;
		U intersectionPointCount;
		s.intersectsRay(rayDir.xyz0(), rayOrigin, intersectionPoints, intersectionPointCount);
		for(U i = 0; i < intersectionPointCount; ++i)
		{
			m_drawer->drawLine(Vec3(0.0), intersectionPoints[i].xyz(), Vec4(0.0, 1.0, 0.0, 1.0));
		}
	}
#endif

#if 0
	{
		Clusterer c;
		c.init(getAllocator(), 16, 12, 30);

		const FrustumComponent& frc = scene.findSceneNode("cam0").getComponent<FrustumComponent>();
		const MoveComponent& movc = scene.findSceneNode("cam0").getComponent<MoveComponent>();

		ClustererPrepareInfo pinf;
		pinf.m_viewMat = frc.getViewMatrix();
		pinf.m_projMat = frc.getProjectionMatrix();
		pinf.m_camTrf = frc.getFrustum().getTransform();
		c.prepare(m_r->getThreadPool(), pinf);

		class DD : public ClustererDebugDrawer
		{
		public:
			DebugDrawer* m_d;

			void operator()(const Vec3& lineA, const Vec3& lineB, const Vec3& color)
			{
				m_d->drawLine(lineA, lineB, color.xyz1());
			}
		};

		DD dd;
		dd.m_d = m_drawer;

		CollisionDebugDrawer cd(m_drawer);

		Sphere s(Vec4(1.0, 0.1, -1.2, 0.0), 1.2);
		PerspectiveFrustum fr(toRad(25.), toRad(35.), 0.1, 5.);
		fr.transform(Transform(Vec4(0., 1., 0., 0.), Mat3x4::getIdentity(), 1.0));

		m_drawer->setModelMatrix(Mat4(movc.getWorldTransform()));
		// c.debugDraw(dd);

		if(frc.getFrustum().insideFrustum(fr))
		{
			ClustererTestResult rez;
			c.initTestResults(getAllocator(), rez);
			Aabb sbox;
			fr.computeAabb(sbox);
			c.binPerspectiveFrustum(fr, sbox, rez);
			//c.bin(s, sbox, rez);

			c.debugDrawResult(rez, dd);
		}

		m_drawer->setColor(Vec4(1.0, 1.0, 0.0, 1.0));
		frc.getFrustum().accept(cd);
		fr.accept(cd);
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
