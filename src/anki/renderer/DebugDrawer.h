// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/collision/CollisionShape.h>
#include <anki/physics/PhysicsDrawer.h>
#include <anki/scene/Forward.h>
#include <anki/resource/ShaderResource.h>
#include <anki/util/Array.h>

namespace anki
{

// Forward
class Renderer;

/// @addtogroup renderer
/// @{

/// Draws simple primitives
class DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	ANKI_USE_RESULT Error init(Renderer* r);

	void drawGrid();
	void drawSphere(F32 radius, I complexity = 8);
	void drawCube(F32 size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

	void prepareFrame(CommandBufferPtr& jobs);

	void finishFrame();

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

private:
	class Vertex
	{
	public:
		Vec4 m_position;
		Vec4 m_color;
	};

	static const U MAX_VERTS_PER_FRAME = 1024 * 1024;

	Renderer* m_r;
	ShaderResourcePtr m_frag;
	ShaderResourcePtr m_vert;
	ShaderProgramPtr m_prog;
	Array<BufferPtr, MAX_FRAMES_IN_FLIGHT> m_vertBuff;

	CommandBufferPtr m_cmdb;
	WeakArray<Vertex> m_clientVerts;

	Mat4 m_mMat;
	Mat4 m_vpMat;
	Mat4 m_mvpMat; ///< Optimization.
	Vec3 m_crntCol = Vec3(1.0, 0.0, 0.0);
	PrimitiveTopology m_primitive = PrimitiveTopology::LINES;
	U32 m_frameVertCount = 0;
	U32 m_crntDrawVertCount = 0;

	DynamicArray<Vec3> m_sphereVerts;

	Bool8 m_depthTestEnabled = true;

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

	void drawLines(const Vec3* lines, const U32 linesCount, const Vec4& color) final;

private:
	DebugDrawer* m_dbg; ///< The debug drawer
};

/// This is a drawer for some scene nodes that need debug
class SceneDebugDrawer
{
public:
	SceneDebugDrawer(DebugDrawer* d)
		: m_dbg(d)
	{
	}

	void draw(const FrustumComponent& fr) const;

	void draw(const SpatialComponent& sp) const;

	void draw(const PortalComponent& c) const;

	void draw(const SectorComponent& c) const;

	void draw(const ReflectionProxyComponent& proxy) const;

	void draw(const DecalComponent& decalc) const;

private:
	DebugDrawer* m_dbg;
};
/// @}

} // end namespace anki
