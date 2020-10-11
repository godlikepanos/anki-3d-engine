// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/physics/PhysicsDrawer.h>
#include <anki/resource/ShaderProgramResource.h>
#include <anki/util/Array.h>

namespace anki
{

// Forward
class RenderQueueDrawContext;
class StagingGpuMemoryManager;
class StagingGpuMemoryToken;

/// @addtogroup renderer
/// @{

/// Draws simple primitives
class DebugDrawer
{
public:
	ANKI_USE_RESULT Error init(ResourceManager* rsrcManager);

	void prepareFrame(RenderQueueDrawContext* ctx);

	void finishFrame()
	{
		flush();
		m_ctx = nullptr;
	}

	void drawGrid();
	void drawSphere(F32 radius, I complexity = 8);
	void drawCube(F32 size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

	void setTopology(PrimitiveTopology topology)
	{
		if(topology != m_topology)
		{
			flush();
		}
		m_topology = topology;
	}

	void pushBackVertex(const Vec3& pos)
	{
		if((m_cachedPositionCount + 3) >= m_cachedPositions.getSize())
		{
			flush();
			ANKI_ASSERT(m_cachedPositionCount == 0);
		}
		m_cachedPositions[m_cachedPositionCount++] = pos;
	}

	/// Something like glColor
	void setColor(const Vec4& col)
	{
		if(m_crntCol != col)
		{
			flush();
		}
		m_crntCol = col;
	}

	void setModelMatrix(const Mat4& m)
	{
		flush();
		m_mMat = m;
		m_mvpMat = m_vpMat * m_mMat;
	}

	void setViewProjectionMatrix(const Mat4& m)
	{
		flush();
		m_vpMat = m;
		m_mvpMat = m_vpMat * m_mMat;
	}

private:
	ShaderProgramResourcePtr m_prog;

	RenderQueueDrawContext* m_ctx = nullptr;

	// State
	Mat4 m_mMat = Mat4::getIdentity();
	Mat4 m_vpMat = Mat4::getIdentity();
	Mat4 m_mvpMat = Mat4::getIdentity(); ///< Optimization.
	Vec4 m_crntCol = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
	PrimitiveTopology m_topology = PrimitiveTopology::LINES;

	static const U MAX_VERTS_BEFORE_FLUSH = 128;
	Array<Vec3, MAX_VERTS_BEFORE_FLUSH> m_cachedPositions;
	U32 m_cachedPositionCount = 0;

	DynamicArray<Vec3> m_sphereVerts;

	void flush();
};

/// Implement physics debug drawer.
class PhysicsDebugDrawer : public PhysicsDrawer
{
public:
	PhysicsDebugDrawer(DebugDrawer* dbg)
		: m_dbg(dbg)
	{
	}

	void drawLines(const Vec3* lines, const U32 vertCount, const Vec4& color) final;

private:
	DebugDrawer* m_dbg; ///< The debug drawer
};

/// Allocate memory for a line cube and populate it.
void allocateAndPopulateDebugBox(StagingGpuMemoryManager& stagingGpuAllocator, StagingGpuMemoryToken& vertsToken,
								 StagingGpuMemoryToken& indicesToken, U32& indexCount);

/// Debug drawer.
class DebugDrawer2
{
public:
	ANKI_USE_RESULT Error init(ResourceManager* rsrcManager);

	void drawCubes(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth, F32 cubeSideSize,
				   StagingGpuMemoryManager& stagingGpuAllocator, CommandBufferPtr& cmdb) const;

	void drawCube(const Mat4& mvp, const Vec4& color, F32 lineSize, Bool ditherFailedDepth, F32 cubeSideSize,
				  StagingGpuMemoryManager& stagingGpuAllocator, CommandBufferPtr& cmdb) const
	{
		drawCubes(ConstWeakArray<Mat4>(&mvp, 1), color, lineSize, ditherFailedDepth, cubeSideSize, stagingGpuAllocator,
				  cmdb);
	}

	void drawLines(ConstWeakArray<Mat4> mvps, const Vec4& color, F32 lineSize, Bool ditherFailedDepth,
				   ConstWeakArray<Vec3> lines, StagingGpuMemoryManager& stagingGpuAllocator,
				   CommandBufferPtr& cmdb) const;

	void drawLine(const Mat4& mvp, const Vec4& color, F32 lineSize, Bool ditherFailedDepth, const Vec3& a,
				  const Vec3& b, StagingGpuMemoryManager& stagingGpuAllocator, CommandBufferPtr& cmdb) const
	{
		Array<Vec3, 2> points = {a, b};
		drawLines(ConstWeakArray<Mat4>(&mvp, 1), color, lineSize, ditherFailedDepth, points, stagingGpuAllocator, cmdb);
	}

	void drawBillboardTextures(const Mat4& projMat, const Mat4& viewMat, ConstWeakArray<Vec3> positions,
							   const Vec4& color, Bool ditherFailedDepth, TextureViewPtr tex, SamplerPtr sampler,
							   Vec2 billboardSize, StagingGpuMemoryManager& stagingGpuAllocator,
							   CommandBufferPtr& cmdb) const;

	void drawBillboardTexture(const Mat4& projMat, const Mat4& viewMat, Vec3 position, const Vec4& color,
							  Bool ditherFailedDepth, TextureViewPtr tex, SamplerPtr sampler, Vec2 billboardSize,
							  StagingGpuMemoryManager& stagingGpuAllocator, CommandBufferPtr& cmdb) const
	{
		drawBillboardTextures(projMat, viewMat, ConstWeakArray<Vec3>(&position, 1), color, ditherFailedDepth, tex,
							  sampler, billboardSize, stagingGpuAllocator, cmdb);
	}

private:
	ShaderProgramResourcePtr m_prog;
};
/// @}

} // end namespace anki
