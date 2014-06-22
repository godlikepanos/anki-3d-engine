// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_DEBUG_DRAWER_H
#define ANKI_RENDERER_DEBUG_DRAWER_H

#include "anki/Math.h"
#include "anki/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/collision/CollisionShape.h"
#include "anki/scene/Forward.h"
#include "anki/util/Array.h"
#include <unordered_map>

namespace anki {

/// @addtogroup renderer
/// @{

/// Draws simple primitives
class DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	void drawGrid();
	void drawSphere(F32 radius, I complexity = 4);
	void drawCube(F32 size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

	void prepareDraw(GlJobChainHandle& jobs)
	{
		m_jobs = jobs;
	}

	void finishDraw()
	{
		m_jobs = GlJobChainHandle(); // Release job chain
	}

	/// @name Render functions. Imitate the GL 1.1 immediate mode
	/// @{
	void begin(); ///< Initiates the draw
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

	/// This is the function that actualy draws
	void flush();

private:
	class Vertex
	{
	public:
		Vec4 m_position;
		Vec4 m_color;
	};

	ProgramResourcePointer m_frag;
	ProgramResourcePointer m_vert;
	GlProgramPipelineHandle m_ppline;
	GlJobChainHandle m_jobs;

	static const U MAX_POINTS_PER_DRAW = 256;
	Mat4 m_mMat;
	Mat4 m_vpMat;
	Mat4 m_mvpMat; ///< Optimization
	U32 m_vertexPointer;
	Vec3 m_crntCol;

	Array<Vertex, MAX_POINTS_PER_DRAW> m_clientVerts;

	GlBufferHandle m_vertBuff;

	/// This is a container of some precalculated spheres. Its a map that
	/// from sphere complexity it returns a vector of lines (Vec3s in
	/// pairs)
	std::unordered_map<U32, Vector<Vec3>> m_complexityToPreCalculatedSphere;
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

	void visit(const CompoundShape&)
	{}

private:
	DebugDrawer* m_dbg; ///< The debug drawer
};

// Forward
class Renderer;

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

	void draw(const Sector& sector);

	void setViewProjectionMatrix(const Mat4& m)
	{
		m_dbg->setViewProjectionMatrix(m);
	}

private:
	DebugDrawer* m_dbg;

	void draw(FrustumComponent& fr) const;

	void draw(SpatialComponent& sp) const;

	void drawPath(const Path& path) const;
};

/// @}

} // end namespace anki

#endif
