// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/collision/CollisionShape.h>
#include <anki/physics/PhysicsDrawer.h>
#include <anki/util/Array.h>
#include <anki/resource/ShaderProgramResource.h>

namespace anki
{

// Forward
class RenderQueueDrawContext;

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

/// Contains methods to render the collision shapes
class CollisionDebugDrawer : public CollisionShape::ConstVisitor
{
public:
	/// Constructor
	CollisionDebugDrawer(DebugDrawer* dbg)
		: m_dbg(dbg)
	{
	}

	void visit(const LineSegment&);

	void visit(const Obb&);

	void visit(const Frustum&);

	void visit(const Plane&);

	void visit(const Sphere&);

	void visit(const Aabb&);

	void visit(const CompoundShape&);

	void visit(const ConvexHullShape&);

private:
	DebugDrawer* m_dbg; ///< The debug drawer
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
/// @}

} // end namespace anki
