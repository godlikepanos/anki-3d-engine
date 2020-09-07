// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	ANKI_CHECK(rsrcManager->loadResource("shaders/SceneDebug.ankiprog", m_prog));

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
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addMutation("COLOR_TEXTURE", 0);
		variantInitInfo.addMutation("DITHERED_DEPTH_TEST",
									m_ctx->m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON));
		variantInitInfo.addConstant("INSTANCE_COUNT", 1u);
		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
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
	cmdb->setLineWidth(1.0f);
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

	const F32 SPACE = 1.0f; // space between lines
	const F32 NUM = 57.0f; // lines number. must be odd

	const F32 GRID_HALF_SIZE = ((NUM - 1.0f) * SPACE / 2.0f);

	setColor(col0);
	setTopology(PrimitiveTopology::LINES);

	for(F32 x = -NUM / 2.0f * SPACE; x < NUM / 2 * SPACE; x += SPACE)
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
	F32 fi = PI / F32(complexity);

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
	const Vec3 maxPos = Vec3(0.5f * size);
	const Vec3 minPos = Vec3(-0.5f * size);

	Array<Vec3, 8> points = {
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()), // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()), // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()), // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()), // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()), // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()), // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()), // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z()) // right bottom back
	};

	static const Array<U32, 24> indeces = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

	setTopology(PrimitiveTopology::LINES);
	for(U32 id : indeces)
	{
		pushBackVertex(points[id]);
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

void allocateAndPopulateDebugBox(StagingGpuMemoryManager& stagingGpuAllocator, StagingGpuMemoryToken& vertsToken,
								 StagingGpuMemoryToken& indicesToken, U32& indexCount)
{
	Vec3* verts = static_cast<Vec3*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec3) * 8, StagingGpuMemoryType::VERTEX, vertsToken));

	const F32 SIZE = 1.0f;
	verts[0] = Vec3(SIZE, SIZE, SIZE); // front top right
	verts[1] = Vec3(-SIZE, SIZE, SIZE); // front top left
	verts[2] = Vec3(-SIZE, -SIZE, SIZE); // front bottom left
	verts[3] = Vec3(SIZE, -SIZE, SIZE); // front bottom right
	verts[4] = Vec3(SIZE, SIZE, -SIZE); // back top right
	verts[5] = Vec3(-SIZE, SIZE, -SIZE); // back top left
	verts[6] = Vec3(-SIZE, -SIZE, -SIZE); // back bottom left
	verts[7] = Vec3(SIZE, -SIZE, -SIZE); // back bottom right

	const U INDEX_COUNT = 12 * 2;
	U16* indices = static_cast<U16*>(
		stagingGpuAllocator.allocateFrame(sizeof(U16) * INDEX_COUNT, StagingGpuMemoryType::VERTEX, indicesToken));

	U c = 0;
	indices[c++] = 0;
	indices[c++] = 1;
	indices[c++] = 1;
	indices[c++] = 2;
	indices[c++] = 2;
	indices[c++] = 3;
	indices[c++] = 3;
	indices[c++] = 0;

	indices[c++] = 4;
	indices[c++] = 5;
	indices[c++] = 5;
	indices[c++] = 6;
	indices[c++] = 6;
	indices[c++] = 7;
	indices[c++] = 7;
	indices[c++] = 4;

	indices[c++] = 0;
	indices[c++] = 4;
	indices[c++] = 1;
	indices[c++] = 5;
	indices[c++] = 2;
	indices[c++] = 6;
	indices[c++] = 3;
	indices[c++] = 7;

	ANKI_ASSERT(c == INDEX_COUNT);
	indexCount = INDEX_COUNT;
}

Error DebugDrawer2::init(ResourceManager* rsrcManager)
{
	return rsrcManager->loadResource("shaders/SceneDebug.ankiprog", m_prog);
}

void DebugDrawer2::drawCubes(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth,
							 F32 cubeSideSize, StagingGpuMemoryManager& stagingGpuAllocator,
							 CommandBufferPtr& cmdb) const
{
	StagingGpuMemoryToken vertsToken;
	StagingGpuMemoryToken indicesToken;

	Vec3* verts = static_cast<Vec3*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec3) * 8, StagingGpuMemoryType::VERTEX, vertsToken));

	const F32 size = cubeSideSize / 2.0f;
	verts[0] = Vec3(size, size, size); // front top right
	verts[1] = Vec3(-size, size, size); // front top left
	verts[2] = Vec3(-size, -size, size); // front bottom left
	verts[3] = Vec3(size, -size, size); // front bottom right
	verts[4] = Vec3(size, size, -size); // back top right
	verts[5] = Vec3(-size, size, -size); // back top left
	verts[6] = Vec3(-size, -size, -size); // back bottom left
	verts[7] = Vec3(size, -size, -size); // back bottom right

	const U INDEX_COUNT = 12 * 2;
	U16* indices = static_cast<U16*>(
		stagingGpuAllocator.allocateFrame(sizeof(U16) * INDEX_COUNT, StagingGpuMemoryType::VERTEX, indicesToken));

	U32 indexCount = 0;
	indices[indexCount++] = 0;
	indices[indexCount++] = 1;
	indices[indexCount++] = 1;
	indices[indexCount++] = 2;
	indices[indexCount++] = 2;
	indices[indexCount++] = 3;
	indices[indexCount++] = 3;
	indices[indexCount++] = 0;

	indices[indexCount++] = 4;
	indices[indexCount++] = 5;
	indices[indexCount++] = 5;
	indices[indexCount++] = 6;
	indices[indexCount++] = 6;
	indices[indexCount++] = 7;
	indices[indexCount++] = 7;
	indices[indexCount++] = 4;

	indices[indexCount++] = 0;
	indices[indexCount++] = 4;
	indices[indexCount++] = 1;
	indices[indexCount++] = 5;
	indices[indexCount++] = 2;
	indices[indexCount++] = 6;
	indices[indexCount++] = 3;
	indices[indexCount++] = 7;

	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* pmvps = static_cast<Mat4*>(stagingGpuAllocator.allocateFrame(sizeof(Mat4) * mvps.getSize() + sizeof(Vec4),
																	   StagingGpuMemoryType::UNIFORM, unisToken));

	memcpy(pmvps, &mvps[0], mvps.getSizeInBytes());

	Vec4* pcolor = reinterpret_cast<Vec4*>(pmvps + mvps.getSize());
	*pcolor = color;

	// Setup state
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("COLOR_TEXTURE", 0);
	variantInitInfo.addMutation("DITHERED_DEPTH_TEST", U32(ditherFailedDepth != 0));
	variantInitInfo.addConstant("INSTANCE_COUNT", mvps.getSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	cmdb->bindShaderProgram(variant->getProgram());

	cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
	cmdb->bindVertexBuffer(0, vertsToken.m_buffer, vertsToken.m_offset, sizeof(Vec3));
	cmdb->bindIndexBuffer(indicesToken.m_buffer, indicesToken.m_offset, IndexType::U16);

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);

	cmdb->setLineWidth(lineSize);
	cmdb->drawElements(PrimitiveTopology::LINES, indexCount, mvps.getSize());
}

void DebugDrawer2::drawLines(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth,
							 ConstWeakArray<Vec3> lines, StagingGpuMemoryManager& stagingGpuAllocator,
							 CommandBufferPtr& cmdb) const
{
	ANKI_ASSERT(mvps.getSize() > 0);
	ANKI_ASSERT(lines.getSize() > 0 && (lines.getSize() % 2) == 0);

	// Verts
	StagingGpuMemoryToken vertsToken;
	Vec3* verts = static_cast<Vec3*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec3) * lines.getSize(), StagingGpuMemoryType::VERTEX, vertsToken));
	memcpy(verts, lines.getBegin(), lines.getSizeInBytes());

	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* pmvps = static_cast<Mat4*>(stagingGpuAllocator.allocateFrame(sizeof(Mat4) * mvps.getSize() + sizeof(Vec4),
																	   StagingGpuMemoryType::UNIFORM, unisToken));

	memcpy(pmvps, &mvps[0], mvps.getSizeInBytes());
	Vec4* pcolor = reinterpret_cast<Vec4*>(pmvps + mvps.getSize());
	*pcolor = color;

	// Setup state
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("COLOR_TEXTURE", 0);
	variantInitInfo.addMutation("DITHERED_DEPTH_TEST", U32(ditherFailedDepth != 0));
	variantInitInfo.addConstant("INSTANCE_COUNT", mvps.getSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	cmdb->bindShaderProgram(variant->getProgram());

	cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
	cmdb->bindVertexBuffer(0, vertsToken.m_buffer, vertsToken.m_offset, sizeof(Vec3));

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);

	cmdb->setLineWidth(lineSize);
	cmdb->drawArrays(PrimitiveTopology::LINES, lines.getSize(), mvps.getSize());
}

void DebugDrawer2::drawBillboardTextures(const Mat4& projMat, const Mat4& viewMat, ConstWeakArray<Vec3> positions,
										 const Vec4& color, Bool ditherFailedDepth, TextureViewPtr tex,
										 SamplerPtr sampler, Vec2 billboardSize,
										 StagingGpuMemoryManager& stagingGpuAllocator, CommandBufferPtr& cmdb) const
{
	StagingGpuMemoryToken positionsToken;
	Vec3* verts = static_cast<Vec3*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec3) * 4, StagingGpuMemoryType::VERTEX, positionsToken));

	verts[0] = Vec3(-0.5f, -0.5f, 0.0f);
	verts[1] = Vec3(+0.5f, -0.5f, 0.0f);
	verts[2] = Vec3(-0.5f, +0.5f, 0.0f);
	verts[3] = Vec3(+0.5f, +0.5f, 0.0f);

	StagingGpuMemoryToken uvsToken;
	Vec2* uvs =
		static_cast<Vec2*>(stagingGpuAllocator.allocateFrame(sizeof(Vec2) * 4, StagingGpuMemoryType::VERTEX, uvsToken));

	uvs[0] = Vec2(0.0f, 0.0f);
	uvs[1] = Vec2(1.0f, 0.0f);
	uvs[2] = Vec2(0.0f, 1.0f);
	uvs[3] = Vec2(1.0f, 1.0f);

	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* pmvps = static_cast<Mat4*>(stagingGpuAllocator.allocateFrame(
		sizeof(Mat4) * positions.getSize() + sizeof(Vec4), StagingGpuMemoryType::UNIFORM, unisToken));

	const Mat4 camTrf = viewMat.getInverse();
	const Vec3 zAxis = camTrf.getZAxis().xyz().getNormalized();
	Vec3 yAxis = Vec3(0.0f, 1.0f, 0.0f);
	const Vec3 xAxis = yAxis.cross(zAxis).getNormalized();
	yAxis = zAxis.cross(xAxis).getNormalized();
	Mat3 rot;
	rot.setColumns(xAxis, yAxis, zAxis);

	for(const Vec3& pos : positions)
	{
		Mat3 scale = Mat3::getIdentity();
		scale(0, 0) *= billboardSize.x();
		scale(1, 1) *= billboardSize.y();

		*pmvps = projMat * viewMat * Mat4(pos.xyz1(), rot * scale, 1.0f);
		++pmvps;
	}

	Vec4* pcolor = reinterpret_cast<Vec4*>(pmvps);
	*pcolor = color;

	// Setup state
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("COLOR_TEXTURE", 1);
	variantInitInfo.addMutation("DITHERED_DEPTH_TEST", U32(ditherFailedDepth != 0));
	variantInitInfo.addConstant("INSTANCE_COUNT", positions.getSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	cmdb->bindShaderProgram(variant->getProgram());

	cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
	cmdb->setVertexAttribute(1, 1, Format::R32G32_SFLOAT, 0);
	cmdb->bindVertexBuffer(0, positionsToken.m_buffer, positionsToken.m_offset, sizeof(Vec3));
	cmdb->bindVertexBuffer(1, uvsToken.m_buffer, uvsToken.m_offset, sizeof(Vec2));

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);
	cmdb->bindSampler(1, 1, sampler);
	cmdb->bindTexture(1, 2, tex, TextureUsageBit::SAMPLED_FRAGMENT);

	cmdb->drawArrays(PrimitiveTopology::TRIANGLE_STRIP, 4, positions.getSize());
}

} // end namespace anki
