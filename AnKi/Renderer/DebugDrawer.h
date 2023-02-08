// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Math.h>
#include <AnKi/Gr.h>
#include <AnKi/Physics/PhysicsDrawer.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Util/Array.h>

namespace anki {

// Forward
class RebarStagingGpuMemoryPool;
class RebarGpuMemoryToken;

/// @addtogroup renderer
/// @{

/// Allocate memory for a line cube and populate it.
void allocateAndPopulateDebugBox(RebarStagingGpuMemoryPool& stagingGpuAllocator, RebarGpuMemoryToken& vertsToken,
								 RebarGpuMemoryToken& indicesToken, U32& indexCount);

/// Debug drawer.
class DebugDrawer2
{
public:
	Error init(ResourceManager* rsrcManager, GrManager* gr);

	Bool isInitialized() const
	{
		return m_prog.isCreated();
	}

	void drawCubes(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth, F32 cubeSideSize,
				   RebarStagingGpuMemoryPool& stagingGpuAllocator, CommandBufferPtr& cmdb) const;

	void drawCube(const Mat4& mvp, const Vec4& color, F32 lineSize, Bool ditherFailedDepth, F32 cubeSideSize,
				  RebarStagingGpuMemoryPool& stagingGpuAllocator, CommandBufferPtr& cmdb) const
	{
		drawCubes(ConstWeakArray<Mat4>(&mvp, 1), color, lineSize, ditherFailedDepth, cubeSideSize, stagingGpuAllocator,
				  cmdb);
	}

	void drawLines(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth,
				   ConstWeakArray<Vec3> linePositions, RebarStagingGpuMemoryPool& stagingGpuAllocator,
				   CommandBufferPtr& cmdb) const;

	void drawLine(const Mat4& mvp, const Vec4& color, F32 lineSize, Bool ditherFailedDepth, const Vec3& a,
				  const Vec3& b, RebarStagingGpuMemoryPool& stagingGpuAllocator, CommandBufferPtr& cmdb) const
	{
		Array<Vec3, 2> points = {a, b};
		drawLines(ConstWeakArray<Mat4>(&mvp, 1), color, lineSize, ditherFailedDepth, points, stagingGpuAllocator, cmdb);
	}

	void drawBillboardTextures(const Mat4& projMat, const Mat3x4& viewMat, ConstWeakArray<Vec3> positions,
							   const Vec4& color, Bool ditherFailedDepth, TextureViewPtr tex, SamplerPtr sampler,
							   Vec2 billboardSize, RebarStagingGpuMemoryPool& stagingGpuAllocator,
							   CommandBufferPtr& cmdb) const;

	void drawBillboardTexture(const Mat4& projMat, const Mat3x4& viewMat, Vec3 position, const Vec4& color,
							  Bool ditherFailedDepth, TextureViewPtr tex, SamplerPtr sampler, Vec2 billboardSize,
							  RebarStagingGpuMemoryPool& stagingGpuAllocator, CommandBufferPtr& cmdb) const
	{
		drawBillboardTextures(projMat, viewMat, ConstWeakArray<Vec3>(&position, 1), color, ditherFailedDepth, tex,
							  sampler, billboardSize, stagingGpuAllocator, cmdb);
	}

private:
	ShaderProgramResourcePtr m_prog;
	BufferPtr m_cubePositionsBuffer;
	BufferPtr m_cubeIndicesBuffer;
};

/// Implement physics debug drawer.
class PhysicsDebugDrawer : public PhysicsDrawer
{
public:
	PhysicsDebugDrawer(const DebugDrawer2* dbg)
		: m_dbg(dbg)
	{
	}

	void start(const Mat4& mvp, CommandBufferPtr& cmdb, RebarStagingGpuMemoryPool* stagingGpuAllocator)
	{
		ANKI_ASSERT(stagingGpuAllocator);
		ANKI_ASSERT(m_vertCount == 0);
		m_mvp = mvp;
		m_cmdb = cmdb;
		m_stagingGpuAllocator = stagingGpuAllocator;
	}

	void drawLines(const Vec3* lines, const U32 vertCount, const Vec4& color) final;

	void end()
	{
		flush();
		m_cmdb.reset(nullptr); // This is essential!!!
		m_stagingGpuAllocator = nullptr;
	}

private:
	const DebugDrawer2* m_dbg; ///< The debug drawer
	Mat4 m_mvp = Mat4::getIdentity();
	CommandBufferPtr m_cmdb;
	RebarStagingGpuMemoryPool* m_stagingGpuAllocator = nullptr;

	// Use a vertex cache because drawLines() is practically called for every line
	Array<Vec3, 32> m_vertCache;
	U32 m_vertCount = 0;
	Vec4 m_currentColor = Vec4(-1.0f);

	void flush();
};
/// @}

} // end namespace anki
