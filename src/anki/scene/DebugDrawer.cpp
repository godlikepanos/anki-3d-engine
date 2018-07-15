// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/DebugDrawer.h>
#include <anki/resource/ResourceManager.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/Collision.h>

namespace anki
{

Error DebugDrawer::init(ResourceManager* rsrcManager)
{
	ANKI_ASSERT(rsrcManager);

	// Create the prog and shaders
	ANKI_CHECK(rsrcManager->loadResource("shaders/SceneDebug.glslp", m_prog));

	return Error::NONE;
}

void DebugDrawer::prepareFrame(RenderQueueDrawContext* ctx)
{
	m_ctx = ctx;
}

void DebugDrawer::flush()
{
	if(m_cachedPositionCount == 0)
	{
		return;
	}

	CommandBufferPtr& cmdb = m_ctx->m_commandBuffer;

	// Bind program
	{
		ShaderProgramResourceMutationInitList<2> mutators(m_prog);
		mutators.add("COLOR_TEXTURE", 0);
		mutators.add(
			"DITHERED_DEPTH_TEST", m_ctx->m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON));
		ShaderProgramResourceConstantValueInitList<1> consts(m_prog);
		consts.add("INSTANCE_COUNT", 1u);
		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
		cmdb->bindShaderProgram(variant->getProgram());
	}

	// Set vertex state
	{
		const U32 size = m_cachedPositionCount * sizeof(Vec3);

		StagingGpuMemoryToken token;
		void* mem = m_ctx->m_stagingGpuAllocator->allocateFrame(size, StagingGpuMemoryType::VERTEX, token);
		memcpy(mem, &m_cachedPositions[0], size);

		cmdb->bindVertexBuffer(0, token.m_buffer, token.m_offset, sizeof(Vec3));
		cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
	}

	// Set uniform state
	{
		struct Uniforms
		{
			Mat4 m_mvp;
			Vec4 m_color;
		};

		StagingGpuMemoryToken token;
		Uniforms* uniforms = static_cast<Uniforms*>(
			m_ctx->m_stagingGpuAllocator->allocateFrame(sizeof(Uniforms), StagingGpuMemoryType::UNIFORM, token));
		uniforms->m_mvp = m_mvpMat;
		uniforms->m_color = m_crntCol;

		cmdb->bindUniformBuffer(1, 0, token.m_buffer, token.m_offset, token.m_range);
	}

	const Bool enableDepthTest = m_ctx->m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
	if(enableDepthTest)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}
	else
	{
		cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	}

	// Draw
	cmdb->drawArrays(m_topology, m_cachedPositionCount);

	// Restore state
	if(!enableDepthTest)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}

	// Other
	m_cachedPositionCount = 0;
}

void DebugDrawer::drawLine(const Vec3& from, const Vec3& to, const Vec4& color)
{
	setColor(color);
	setTopology(PrimitiveTopology::LINES);
	pushBackVertex(from);
	pushBackVertex(to);
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
	setTopology(PrimitiveTopology::LINES);

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
}

void DebugDrawer::drawSphere(F32 radius, I complexity)
{
	Mat4 oldMMat = m_mMat;

	setModelMatrix(m_mMat * Mat4(Vec4(0.0, 0.0, 0.0, 1.0), Mat3::getIdentity(), radius));
	setTopology(PrimitiveTopology::LINES);

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

	setModelMatrix(oldMMat);
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

	setTopology(PrimitiveTopology::LINES);
	for(U32 id : indeces)
	{
		pushBackVertex(points[id]);
	}
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
	m_dbg->setTopology(PrimitiveTopology::LINES);
	m_dbg->pushBackVertex(ls.getOrigin().xyz());
	m_dbg->pushBackVertex((ls.getOrigin() + ls.getDirection()).xyz());
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

		m_dbg->setTopology(PrimitiveTopology::LINES);
		for(U32 i = 0; i < sizeof(indeces) / sizeof(U32); i++)
		{
			m_dbg->pushBackVertex(points[indeces[i]]);
		}
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
	m_dbg->setTopology(PrimitiveTopology::LINES);
	const Vec4* points = hull.getPoints() + 1;
	const Vec4* end = hull.getPoints() + hull.getPointsCount();
	for(; points != end; ++points)
	{
		m_dbg->pushBackVertex(hull.getPoints()->xyz());
		m_dbg->pushBackVertex(points->xyz());
	}
}

void PhysicsDebugDrawer::drawLines(const Vec3* lines, const U32 vertCount, const Vec4& color)
{
	m_dbg->setTopology(PrimitiveTopology::LINES);
	m_dbg->setColor(color);
	for(U i = 0; i < vertCount; ++i)
	{
		m_dbg->pushBackVertex(lines[i]);
	}
}

} // end namespace anki
