// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/Math.h"
#include "anki/Gr.h"
#include "anki/collision/CollisionShape.h"
#include "anki/physics/PhysicsDrawer.h"
#include "anki/scene/Forward.h"
#include "anki/resource/ShaderResource.h"
#include "anki/util/Array.h"

namespace anki {

// Forward
class Renderer;
class PortalSectorComponent;

/// @addtogroup renderer
/// @{

/// Draws simple primitives
class DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	ANKI_USE_RESULT Error create(Renderer* r);

	void drawGrid();
	void drawSphere(F32 radius, I complexity = 8);
	void drawCube(F32 size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

	void prepareDraw(CommandBufferPtr& jobs)
	{
		m_cmdb = jobs;
	}

	void finishDraw()
	{
		m_cmdb = CommandBufferPtr(); // Release job chain
	}

	/// @name Render functions. Imitate the GL 1.1 immediate mode
	/// @{
	void begin(PrimitiveTopology topology); ///< Initiates the draw

	void end(); ///< Draws

	void pushBackVertex(const Vec3& pos); ///< Something like glVertex

	/// Something like glColor
	void setColor(const Vec3& col)
	{
		m_crntCol = col;
	}

	/// Something like glColor
	void setColor(const Vec4& col)
	{
		m_crntCol = col.xyz();
	}

	void setModelMatrix(const Mat4& m);

	void setViewProjectionMatrix(const Mat4& m);
	/// @}

	void setDepthTestEnabled(Bool enabled)
	{
		m_depthTestEnabled = enabled;
	}

	Bool getDepthTestEnabled() const
	{
		return m_depthTestEnabled;
	}

	/// This is the function that actualy draws
	ANKI_USE_RESULT Error flush();

private:
	class Vertex
	{
	public:
		Vec4 m_position;
		Vec4 m_color;
	};

	ShaderResourcePtr m_frag;
	ShaderResourcePtr m_vert;
	PipelinePtr m_pplineLinesDepth;
	PipelinePtr m_pplineLinesNoDepth;
	CommandBufferPtr m_cmdb;
	ResourceGroupPtr m_rcGroup;

	static const U MAX_POINTS_PER_DRAW = 256;
	Mat4 m_mMat;
	Mat4 m_vpMat;
	Mat4 m_mvpMat; ///< Optimization
	U32 m_lineVertCount;
	U32 m_triVertCount;
	Vec3 m_crntCol;
	PrimitiveTopology m_primitive;

	BufferPtr m_vertBuff;

	Array<Vertex, MAX_POINTS_PER_DRAW> m_clientLineVerts;
	Array<Vertex, MAX_POINTS_PER_DRAW> m_clientTriVerts;

	DArray<Vec3> m_sphereVerts;

	Bool8 m_depthTestEnabled = true;

	ANKI_USE_RESULT Error flushInternal(PrimitiveTopology topology);
};

/// Contains methods to render the collision shapes
class CollisionDebugDrawer: public CollisionShape::ConstVisitor
{
public:
	/// Constructor
	CollisionDebugDrawer(DebugDrawer* dbg)
		: m_dbg(dbg)
	{}

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
class PhysicsDebugDrawer: public PhysicsDrawer
{
public:
	PhysicsDebugDrawer(DebugDrawer* dbg)
		: m_dbg(dbg)
	{}

	void drawLines(
		const Vec3* lines,
		const U32 linesCount,
		const Vec4& color) final;

private:
	DebugDrawer* m_dbg; ///< The debug drawer
};

/// This is a drawer for some scene nodes that need debug
class SceneDebugDrawer
{
public:
	SceneDebugDrawer(DebugDrawer* d)
		: m_dbg(d)
	{}

	~SceneDebugDrawer()
	{}

	void draw(SceneNode& node);

	//void draw(const Sector& sector);

	void setViewProjectionMatrix(const Mat4& m)
	{
		m_dbg->setViewProjectionMatrix(m);
	}

private:
	DebugDrawer* m_dbg;

	void draw(FrustumComponent& fr) const;

	void draw(SpatialComponent& sp) const;

	void draw(const PortalSectorComponent& c) const;

	void drawPath(const Path& path) const;
};
/// @}

} // end namespace anki

