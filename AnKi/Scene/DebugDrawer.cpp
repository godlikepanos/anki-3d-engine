// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/DebugDrawer.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Collision.h>

namespace anki {

void allocateAndPopulateDebugBox(StagingGpuMemoryPool& stagingGpuAllocator, StagingGpuMemoryToken& vertsToken,
								 StagingGpuMemoryToken& indicesToken, U32& indexCount)
{
	Vec3* verts = static_cast<Vec3*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec3) * 8, StagingGpuMemoryType::kVertex, vertsToken));

	constexpr F32 kSize = 1.0f;
	verts[0] = Vec3(kSize, kSize, kSize); // front top right
	verts[1] = Vec3(-kSize, kSize, kSize); // front top left
	verts[2] = Vec3(-kSize, -kSize, kSize); // front bottom left
	verts[3] = Vec3(kSize, -kSize, kSize); // front bottom right
	verts[4] = Vec3(kSize, kSize, -kSize); // back top right
	verts[5] = Vec3(-kSize, kSize, -kSize); // back top left
	verts[6] = Vec3(-kSize, -kSize, -kSize); // back bottom left
	verts[7] = Vec3(kSize, -kSize, -kSize); // back bottom right

	constexpr U kIndexCount = 12 * 2;
	U16* indices = static_cast<U16*>(
		stagingGpuAllocator.allocateFrame(sizeof(U16) * kIndexCount, StagingGpuMemoryType::kVertex, indicesToken));

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

	ANKI_ASSERT(c == kIndexCount);
	indexCount = kIndexCount;
}

Error DebugDrawer2::init(ResourceManager* rsrcManager)
{
	ANKI_CHECK(rsrcManager->loadResource("ShaderBinaries/SceneDebug.ankiprogbin", m_prog));

	{
		BufferInitInfo bufferInit("DebugCube");
		bufferInit.m_usage = BufferUsageBit::kVertex;
		bufferInit.m_size = sizeof(Vec3) * 8;
		bufferInit.m_mapAccess = BufferMapAccessBit::kWrite;
		m_cubePositionsBuffer = rsrcManager->getGrManager().newBuffer(bufferInit);

		Vec3* verts = static_cast<Vec3*>(m_cubePositionsBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		const F32 size = 1.0f;
		verts[0] = Vec3(size, size, size); // front top right
		verts[1] = Vec3(-size, size, size); // front top left
		verts[2] = Vec3(-size, -size, size); // front bottom left
		verts[3] = Vec3(size, -size, size); // front bottom right
		verts[4] = Vec3(size, size, -size); // back top right
		verts[5] = Vec3(-size, size, -size); // back top left
		verts[6] = Vec3(-size, -size, -size); // back bottom left
		verts[7] = Vec3(size, -size, -size); // back bottom right

		m_cubePositionsBuffer->flush(0, kMaxPtrSize);
		m_cubePositionsBuffer->unmap();
	}

	{
		constexpr U kIndexCount = 12 * 2;

		BufferInitInfo bufferInit("DebugCube");
		bufferInit.m_usage = BufferUsageBit::kVertex;
		bufferInit.m_size = sizeof(U16) * kIndexCount;
		bufferInit.m_mapAccess = BufferMapAccessBit::kWrite;
		m_cubeIndicesBuffer = rsrcManager->getGrManager().newBuffer(bufferInit);

		U16* indices = static_cast<U16*>(m_cubeIndicesBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

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

		m_cubeIndicesBuffer->flush(0, kMaxPtrSize);
		m_cubeIndicesBuffer->unmap();
	}

	return Error::kNone;
}

void DebugDrawer2::drawCubes(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth,
							 F32 cubeSideSize, StagingGpuMemoryPool& stagingGpuAllocator, CommandBufferPtr& cmdb) const
{
	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* pmvps = static_cast<Mat4*>(stagingGpuAllocator.allocateFrame(sizeof(Mat4) * mvps.getSize() + sizeof(Vec4),
																	   StagingGpuMemoryType::kUniform, unisToken));

	if(cubeSideSize == 2.0f)
	{
		memcpy(pmvps, &mvps[0], mvps.getSizeInBytes());
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	Vec4* pcolor = reinterpret_cast<Vec4*>(pmvps + mvps.getSize());
	*pcolor = color;

	// Setup state
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("COLOR_TEXTURE", 0);
	variantInitInfo.addMutation("DITHERED_DEPTH_TEST", U32(ditherFailedDepth != 0));
	variantInitInfo.addConstant("kInstanceCount", mvps.getSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	cmdb->bindShaderProgram(variant->getProgram());

	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);
	cmdb->bindVertexBuffer(0, m_cubePositionsBuffer, 0, sizeof(Vec3));
	cmdb->bindIndexBuffer(m_cubeIndicesBuffer, 0, IndexType::kU16);

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);

	cmdb->setLineWidth(lineSize);
	constexpr U kIndexCount = 12 * 2;
	cmdb->drawElements(PrimitiveTopology::kLines, kIndexCount, mvps.getSize());
}

void DebugDrawer2::drawLines(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth,
							 ConstWeakArray<Vec3> linePositions, StagingGpuMemoryPool& stagingGpuAllocator,
							 CommandBufferPtr& cmdb) const
{
	ANKI_ASSERT(mvps.getSize() > 0);
	ANKI_ASSERT(linePositions.getSize() > 0 && (linePositions.getSize() % 2) == 0);

	// Verts
	StagingGpuMemoryToken vertsToken;
	Vec3* verts = static_cast<Vec3*>(stagingGpuAllocator.allocateFrame(sizeof(Vec3) * linePositions.getSize(),
																	   StagingGpuMemoryType::kVertex, vertsToken));
	memcpy(verts, linePositions.getBegin(), linePositions.getSizeInBytes());

	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* pmvps = static_cast<Mat4*>(stagingGpuAllocator.allocateFrame(sizeof(Mat4) * mvps.getSize() + sizeof(Vec4),
																	   StagingGpuMemoryType::kUniform, unisToken));

	memcpy(pmvps, &mvps[0], mvps.getSizeInBytes());
	Vec4* pcolor = reinterpret_cast<Vec4*>(pmvps + mvps.getSize());
	*pcolor = color;

	// Setup state
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("COLOR_TEXTURE", 0);
	variantInitInfo.addMutation("DITHERED_DEPTH_TEST", U32(ditherFailedDepth != 0));
	variantInitInfo.addConstant("kInstanceCount", mvps.getSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	cmdb->bindShaderProgram(variant->getProgram());

	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);
	cmdb->bindVertexBuffer(0, vertsToken.m_buffer, vertsToken.m_offset, sizeof(Vec3));

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);

	cmdb->setLineWidth(lineSize);
	cmdb->drawArrays(PrimitiveTopology::kLines, linePositions.getSize(), mvps.getSize());
}

void DebugDrawer2::drawBillboardTextures(const Mat4& projMat, const Mat3x4& viewMat, ConstWeakArray<Vec3> positions,
										 const Vec4& color, Bool ditherFailedDepth, TextureViewPtr tex,
										 SamplerPtr sampler, Vec2 billboardSize,
										 StagingGpuMemoryPool& stagingGpuAllocator, CommandBufferPtr& cmdb) const
{
	StagingGpuMemoryToken positionsToken;
	Vec3* verts = static_cast<Vec3*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec3) * 4, StagingGpuMemoryType::kVertex, positionsToken));

	verts[0] = Vec3(-0.5f, -0.5f, 0.0f);
	verts[1] = Vec3(+0.5f, -0.5f, 0.0f);
	verts[2] = Vec3(-0.5f, +0.5f, 0.0f);
	verts[3] = Vec3(+0.5f, +0.5f, 0.0f);

	StagingGpuMemoryToken uvsToken;
	Vec2* uvs = static_cast<Vec2*>(
		stagingGpuAllocator.allocateFrame(sizeof(Vec2) * 4, StagingGpuMemoryType::kVertex, uvsToken));

	uvs[0] = Vec2(0.0f, 0.0f);
	uvs[1] = Vec2(1.0f, 0.0f);
	uvs[2] = Vec2(0.0f, 1.0f);
	uvs[3] = Vec2(1.0f, 1.0f);

	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* pmvps = static_cast<Mat4*>(stagingGpuAllocator.allocateFrame(
		sizeof(Mat4) * positions.getSize() + sizeof(Vec4), StagingGpuMemoryType::kUniform, unisToken));

	const Mat4 camTrf = Mat4(viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f)).getInverse();
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

		*pmvps = projMat * Mat4(viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f)) * Mat4(pos.xyz1(), rot * scale, 1.0f);
		++pmvps;
	}

	Vec4* pcolor = reinterpret_cast<Vec4*>(pmvps);
	*pcolor = color;

	// Setup state
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("COLOR_TEXTURE", 1);
	variantInitInfo.addMutation("DITHERED_DEPTH_TEST", U32(ditherFailedDepth != 0));
	variantInitInfo.addConstant("kInstanceCount", positions.getSize());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	cmdb->bindShaderProgram(variant->getProgram());

	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);
	cmdb->setVertexAttribute(1, 1, Format::kR32G32_Sfloat, 0);
	cmdb->bindVertexBuffer(0, positionsToken.m_buffer, positionsToken.m_offset, sizeof(Vec3));
	cmdb->bindVertexBuffer(1, uvsToken.m_buffer, uvsToken.m_offset, sizeof(Vec2));

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);
	cmdb->bindSampler(1, 1, sampler);
	cmdb->bindTexture(1, 2, tex);

	cmdb->drawArrays(PrimitiveTopology::kTriangleStrip, 4, positions.getSize());
}

void PhysicsDebugDrawer::drawLines(const Vec3* lines, const U32 vertCount, const Vec4& color)
{
	if(color != m_currentColor)
	{
		// Color have changed, flush and change the color
		flush();
		m_currentColor = color;
	}

	for(U32 i = 0; i < vertCount; ++i)
	{
		if(m_vertCount == m_vertCache.getSize())
		{
			flush();
		}

		m_vertCache[m_vertCount++] = lines[i];
	}
}

void PhysicsDebugDrawer::flush()
{
	if(m_vertCount > 0)
	{
		m_dbg->drawLines(ConstWeakArray<Mat4>(&m_mvp, 1), m_currentColor, 2.0f, false,
						 ConstWeakArray<Vec3>(&m_vertCache[0], m_vertCount), *m_stagingGpuAllocator, m_cmdb);

		m_vertCount = 0;
	}
}

} // end namespace anki
