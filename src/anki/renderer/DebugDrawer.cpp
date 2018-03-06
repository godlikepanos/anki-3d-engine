// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DebugDrawer.h>
#include <anki/renderer/Renderer.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/GBuffer.h>
#include <anki/util/Logger.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/Collision.h>
#include <anki/Scene.h>

namespace anki
{

DebugDrawer::DebugDrawer()
{
}

DebugDrawer::~DebugDrawer()
{
}

Error DebugDrawer::init(Renderer* r)
{
	m_r = r;
	GrManager& gr = r->getGrManager();

	// Create the prog and shaders
	ANKI_CHECK(r->getResourceManager().loadResource("programs/Dbg.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	// Create the vert buffs
	for(BufferPtr& v : m_vertBuff)
	{
		v = gr.newBuffer(BufferInitInfo(
			sizeof(Vertex) * MAX_VERTS_PER_FRAME, BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE, "DbgDrawer"));
	}

	m_mMat.setIdentity();
	m_vpMat.setIdentity();
	m_mvpMat.setIdentity();

	return Error::NONE;
}

void DebugDrawer::prepareFrame(CommandBufferPtr& jobs)
{
	m_cmdb = jobs;

	U frame = m_r->getFrameCount() % MAX_FRAMES_IN_FLIGHT;
	void* mapped = m_vertBuff[frame]->map(0, MAX_VERTS_PER_FRAME * sizeof(Vertex), BufferMapAccessBit::WRITE);
	m_clientVerts = WeakArray<Vertex>(static_cast<Vertex*>(mapped), MAX_VERTS_PER_FRAME);

	m_cmdb->bindVertexBuffer(0, m_vertBuff[frame], 0, 2 * sizeof(Vec4));
	m_cmdb->setVertexAttribute(0, 0, Format::R32G32B32A32_SFLOAT, 0);
	m_cmdb->setVertexAttribute(1, 0, Format::R32G32B32A32_SFLOAT, sizeof(Vec4));

	m_cmdb->bindShaderProgram(m_grProg);

	m_frameVertCount = 0;
	m_crntDrawVertCount = 0;
}

void DebugDrawer::finishFrame()
{
	U frame = m_r->getFrameCount() % MAX_FRAMES_IN_FLIGHT;
	m_vertBuff[frame]->unmap();

	flush();

	// Restore state
	m_cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);

	m_cmdb = CommandBufferPtr(); // Release command buffer
}

void DebugDrawer::setModelMatrix(const Mat4& m)
{
	m_mMat = m;
	m_mvpMat = m_vpMat * m_mMat;
}

void DebugDrawer::setViewProjectionMatrix(const Mat4& m)
{
	m_vpMat = m;
	m_mvpMat = m_vpMat * m_mMat;
}

void DebugDrawer::begin(PrimitiveTopology topology)
{
	ANKI_ASSERT(topology == PrimitiveTopology::LINES || topology == PrimitiveTopology::TRIANGLES);

	if(topology != m_primitive)
	{
		flush();
	}

	m_primitive = topology;
}

void DebugDrawer::end()
{
}

void DebugDrawer::pushBackVertex(const Vec3& pos)
{
	if(m_frameVertCount < MAX_VERTS_PER_FRAME)
	{
		m_clientVerts[m_frameVertCount].m_position = m_mvpMat * Vec4(pos, 1.0);
		m_clientVerts[m_frameVertCount].m_color = Vec4(m_crntCol, 1.0);

		++m_frameVertCount;
		++m_crntDrawVertCount;
	}
	else
	{
		ANKI_R_LOGW("Increase DebugDrawer::MAX_VERTS_PER_FRAME");
	}
}

void DebugDrawer::flush()
{
	if(m_crntDrawVertCount > 0)
	{
		if(m_primitive == PrimitiveTopology::LINES)
		{
			ANKI_ASSERT((m_crntDrawVertCount % 2) == 0);
		}
		else
		{
			ANKI_ASSERT((m_crntDrawVertCount % 3) == 0);
		}

		m_cmdb->setDepthCompareOperation((m_depthTestEnabled) ? CompareOperation::LESS : CompareOperation::ALWAYS);

		U firstVert = m_frameVertCount - m_crntDrawVertCount;
		m_cmdb->drawArrays(m_primitive, m_crntDrawVertCount, 1, firstVert);

		m_crntDrawVertCount = 0;
	}
}

void DebugDrawer::drawLine(const Vec3& from, const Vec3& to, const Vec4& color)
{
	setColor(color);
	begin(PrimitiveTopology::LINES);
	pushBackVertex(from);
	pushBackVertex(to);
	end();
}

void DebugDrawer::drawGrid()
{
	Vec4 col0(0.5, 0.5, 0.5, 1.0);
	Vec4 col1(0.0, 0.0, 1.0, 1.0);
	Vec4 col2(1.0, 0.0, 0.0, 1.0);

	const F32 SPACE = 1.0; // space between lines
	const I NUM = 57; // lines number. must be odd

	const F32 GRID_HALF_SIZE = ((NUM - 1) * SPACE / 2);

	setColor(col0);

	begin(PrimitiveTopology::LINES);

	for(I x = -NUM / 2 * SPACE; x < NUM / 2 * SPACE; x += SPACE)
	{
		setColor(col0);

		// if the middle line then change color
		if(x == 0)
		{
			setColor(col0 * 0.5 + col1 * 0.5);
			pushBackVertex(Vec3(x, 0.0, -GRID_HALF_SIZE));
			pushBackVertex(Vec3(x, 0.0, 0.0));

			setColor(col1);
			pushBackVertex(Vec3(x, 0.0, 0.0));
			pushBackVertex(Vec3(x, 0.0, GRID_HALF_SIZE));
		}
		else
		{
			// line in z
			pushBackVertex(Vec3(x, 0.0, -GRID_HALF_SIZE));
			pushBackVertex(Vec3(x, 0.0, GRID_HALF_SIZE));
		}

		// if middle line change col so you can highlight the x-axis
		if(x == 0)
		{
			setColor(col0 * 0.5 + col2 * 0.5);
			pushBackVertex(Vec3(-GRID_HALF_SIZE, 0.0, x));
			pushBackVertex(Vec3(0.0, 0.0, x));

			setColor(col2);
			pushBackVertex(Vec3(0.0, 0.0, x));
			pushBackVertex(Vec3(GRID_HALF_SIZE, 0.0, x));
		}
		else
		{
			// line in the x
			pushBackVertex(Vec3(-GRID_HALF_SIZE, 0.0, x));
			pushBackVertex(Vec3(GRID_HALF_SIZE, 0.0, x));
		}
	}

	// render
	end();
}

void DebugDrawer::drawSphere(F32 radius, I complexity)
{
#if 1
	Mat4 oldMMat = m_mMat;
	Mat4 oldVpMat = m_vpMat;

	setModelMatrix(m_mMat * Mat4(Vec4(0.0, 0.0, 0.0, 1.0), Mat3::getIdentity(), radius));

	begin(PrimitiveTopology::LINES);

	// Pre-calculate the sphere points5
	F32 fi = PI / complexity;

	Vec3 prev(1.0, 0.0, 0.0);
	for(F32 th = fi; th < PI * 2.0 + fi; th += fi)
	{
		Vec3 p = Mat3(Euler(0.0, th, 0.0)) * Vec3(1.0, 0.0, 0.0);

		for(F32 th2 = 0.0; th2 < PI; th2 += fi)
		{
			Mat3 rot(Euler(th2, 0.0, 0.0));

			Vec3 rotPrev = rot * prev;
			Vec3 rotP = rot * p;

			pushBackVertex(rotPrev);
			pushBackVertex(rotP);

			Mat3 rot2(Euler(0.0, 0.0, PI / 2));

			pushBackVertex(rot2 * rotPrev);
			pushBackVertex(rot2 * rotP);
		}

		prev = p;
	}

	end();

	m_mMat = oldMMat;
	m_vpMat = oldVpMat;
#endif
}

void DebugDrawer::drawCube(F32 size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	Array<Vec3, 8> points = {{
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()), // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()), // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()), // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()), // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()), // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()), // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()), // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z()) // right bottom back
	}};

	static const Array<U32, 24> indeces = {{0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	begin(PrimitiveTopology::LINES);
	for(U32 id : indeces)
	{
		pushBackVertex(points[id]);
	}
	end();
}

void CollisionDebugDrawer::visit(const Sphere& sphere)
{
	m_dbg->setModelMatrix(Mat4(sphere.getCenter().xyz1(), Mat3::getIdentity(), 1.0));
	m_dbg->drawSphere(sphere.getRadius());
}

void CollisionDebugDrawer::visit(const Obb& obb)
{
	Mat4 scale(Mat4::getIdentity());
	scale(0, 0) = obb.getExtend().x();
	scale(1, 1) = obb.getExtend().y();
	scale(2, 2) = obb.getExtend().z();

	Mat4 tsl(Transform(obb.getCenter(), obb.getRotation(), 1.0));

	m_dbg->setModelMatrix(tsl * scale);
	m_dbg->drawCube(2.0);
}

void CollisionDebugDrawer::visit(const Plane& plane)
{
	Vec3 n = plane.getNormal().xyz();
	const F32& o = plane.getOffset();
	Quat q;
	q.setFrom2Vec3(Vec3(0.0, 0.0, 1.0), n);
	Mat3 rot(q);
	rot.rotateXAxis(PI / 2.0);
	Mat4 trf(Vec4(n * o, 1.0), rot);

	m_dbg->setModelMatrix(trf);
	m_dbg->drawGrid();
}

void CollisionDebugDrawer::visit(const LineSegment& ls)
{
	m_dbg->setModelMatrix(Mat4::getIdentity());
	m_dbg->begin(PrimitiveTopology::LINES);
	m_dbg->pushBackVertex(ls.getOrigin().xyz());
	m_dbg->pushBackVertex((ls.getOrigin() + ls.getDirection()).xyz());
	m_dbg->end();
}

void CollisionDebugDrawer::visit(const Aabb& aabb)
{
	Vec3 min = aabb.getMin().xyz();
	Vec3 max = aabb.getMax().xyz();

	Mat4 trf = Mat4::getIdentity();
	// Scale
	for(U32 i = 0; i < 3; ++i)
	{
		trf(i, i) = max[i] - min[i];
	}

	// Translation
	trf.setTranslationPart(Vec4((max + min) / 2.0, 1.0));

	m_dbg->setModelMatrix(trf);
	m_dbg->drawCube();
}

void CollisionDebugDrawer::visit(const Frustum& f)
{
	switch(f.getType())
	{
	case FrustumType::ORTHOGRAPHIC:
		visit(static_cast<const OrthographicFrustum&>(f).getObb());
		break;
	case FrustumType::PERSPECTIVE:
	{
		const PerspectiveFrustum& pf = static_cast<const PerspectiveFrustum&>(f);

		F32 camLen = pf.getFar();
		F32 tmp0 = camLen / tan((PI - pf.getFovX()) * 0.5) + 0.001;
		F32 tmp1 = camLen * tan(pf.getFovY() * 0.5) + 0.001;

		Vec3 points[] = {
			Vec3(0.0, 0.0, 0.0), // 0: eye point
			Vec3(-tmp0, tmp1, -camLen), // 1: top left
			Vec3(-tmp0, -tmp1, -camLen), // 2: bottom left
			Vec3(tmp0, -tmp1, -camLen), // 3: bottom right
			Vec3(tmp0, tmp1, -camLen) // 4: top right
		};

		const U32 indeces[] = {0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3, 3, 4, 4, 1};

		m_dbg->begin(PrimitiveTopology::LINES);
		for(U32 i = 0; i < sizeof(indeces) / sizeof(U32); i++)
		{
			m_dbg->pushBackVertex(points[indeces[i]]);
		}
		m_dbg->end();
		break;
	}
	}
}

void CollisionDebugDrawer::visit(const CompoundShape& cs)
{
	CollisionDebugDrawer* self = this;
	Error err = cs.iterateShapes([&](const CollisionShape& a) -> Error {
		a.accept(*self);
		return Error::NONE;
	});
	(void)err;
}

void CollisionDebugDrawer::visit(const ConvexHullShape& hull)
{
	m_dbg->setModelMatrix(Mat4(hull.getTransform()));
	m_dbg->begin(PrimitiveTopology::LINES);
	const Vec4* points = hull.getPoints() + 1;
	const Vec4* end = hull.getPoints() + hull.getPointsCount();
	for(; points != end; ++points)
	{
		m_dbg->pushBackVertex(hull.getPoints()->xyz());
		m_dbg->pushBackVertex(points->xyz());
	}
	m_dbg->end();
}

void PhysicsDebugDrawer::drawLines(const Vec3* lines, const U32 linesCount, const Vec4& color)
{
	m_dbg->begin(PrimitiveTopology::LINES);
	m_dbg->setColor(color);
	for(U i = 0; i < linesCount * 2; ++i)
	{
		m_dbg->pushBackVertex(lines[i]);
	}
	m_dbg->end();
}

void SceneDebugDrawer::draw(const RenderableQueueElement& r) const
{
	// TODO
}

void SceneDebugDrawer::draw(const PointLightQueueElement& light) const
{
	m_dbg->setColor(light.m_diffuseColor);
	CollisionDebugDrawer coldraw(m_dbg);
	Sphere sphere(light.m_worldPosition.xyz0(), light.m_radius);
	sphere.accept(coldraw);
}

void SceneDebugDrawer::draw(const SpotLightQueueElement& light) const
{
	// TODO
}

} // end namespace anki
