#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/math/Math.h"
#include "anki/gl/Vbo.h"
#include "anki/gl/Vao.h"
#include "anki/resource/Resource.h"
#include "anki/collision/CollisionShape.h"
#include "anki/scene/SceneNode.h"
#include "anki/util/Flags.h"
#include <array>
#include <map>
#include <LinearMath/btIDebugDraw.h>

namespace anki {

/// Draws simple primitives
class DebugDrawer
{
public:
	DebugDrawer();
	~DebugDrawer();

	void drawGrid();
	void drawSphere(float radius, int complexity = 4);
	void drawCube(float size = 1.0);
	void drawLine(const Vec3& from, const Vec3& to, const Vec4& color);

	/// @name Render functions. Imitate the GL 1.1 immediate mode
	/// @{
	void begin(); ///< Initiates the draw
	void end(); ///< Draws
	void pushBackVertex(const Vec3& pos); ///< Something like glVertex
	/// Something like glColor
	void setColor(const Vec3& col)
	{
		crntCol = col;
	}
	/// Something like glColor
	void setColor(const Vec4& col)
	{
		crntCol = Vec3(col);
	}
	void setModelMatrix(const Mat4& modelMat);
	void setViewProjectionMatrix(const Mat4& m)
	{
		vpMat = m;
	}
	/// @}

private:
	ShaderProgramResourcePointer sProg;
	static const uint MAX_POINTS_PER_DRAW = 256;
	std::array<Vec3, MAX_POINTS_PER_DRAW> positions;
	std::array<Vec3, MAX_POINTS_PER_DRAW> colors;
	Mat4 modelMat;
	Mat4 vpMat;
	uint pointIndex;
	Vec3 crntCol;
	Vbo positionsVbo;
	Vbo colorsVbo;
	Vao vao;

	/// This is a container of some precalculated spheres. Its a map that
	/// from sphere complexity it returns a vector of lines (Vec3s in
	/// pairs)
	std::map<uint, std::vector<Vec3>> complexityToPreCalculatedSphere;
};


/// Contains methods to render the collision shapes
class CollisionDebugDrawer: public CollisionShape::ConstVisitor
{
public:
	/// Constructor
	CollisionDebugDrawer(DebugDrawer* dbg_)
		: dbg(dbg_)
	{}

	void visit(const LineSegment&)
	{
		/// XXX
		ANKI_ASSERT(0 && "ToDo");
	}

	void visit(const Obb&);

	void visit(const Frustum&);

	void visit(const Plane&);

	void visit(const Ray&)
	{
		ANKI_ASSERT(0 && "ToDo");
	}

	void visit(const Sphere&);

	void visit(const Aabb&);

private:
	DebugDrawer* dbg; ///< The debug drawer
};

/// An implementation of btIDebugDraw used for debugging Bullet. See Bullet
/// docs for details
class PhysicsDebugDrawer: public btIDebugDraw
{
public:
	PhysicsDebugDrawer(DebugDrawer* dbg_)
		: dbg(dbg_)
	{}

	void drawLine(const btVector3& from, const btVector3& to,
		const btVector3& color);

	void drawContactPoint(const btVector3& pointOnB,
		const btVector3& normalOnB, btScalar distance, int lifeTime,
		const btVector3& color);

	void drawSphere(btScalar radius, const btTransform& transform,
		const btVector3& color);

	void drawBox(const btVector3& bbMin, const btVector3& bbMax,
		const btVector3& color);

	void drawBox(const btVector3& bbMin, const btVector3& bbMax,
		const btTransform& trans, const btVector3& color);

	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode_)
	{
		debugMode = debugMode_;
	}
	int getDebugMode() const
	{
		return debugMode;
	}

private:
	int debugMode;
	DebugDrawer* dbg;
};

class Octree;
class OctreeNode;
class Renderer;
class Camera;

/// This is a drawer for some scene nodes that need debug
class SceneDebugDrawer: public Flags<uint32_t>
{
public:
	enum DebugFlag
	{
		DF_NONE = 0,
		DF_SPATIAL = 1 << 0,
		DF_FRUSTUMABLE = 1 << 1
	};

	SceneDebugDrawer(DebugDrawer* d)
		: Flags<uint32_t>(DF_SPATIAL | DF_FRUSTUMABLE), dbg(d)
	{}

	virtual ~SceneDebugDrawer()
	{}

	void draw(SceneNode& node);

	virtual void draw(const Octree& octree) const;

	void setViewProjectionMatrix(const Mat4& m)
	{
		dbg->setViewProjectionMatrix(m);
	}

private:
	DebugDrawer* dbg;

	virtual void draw(Frustumable& fr) const;

	virtual void draw(Spatial& sp) const;

	virtual void draw(const OctreeNode& octnode,
		uint32_t depth, const Octree& octree) const;
};

class PassLevelKey;

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
public:
	/// The one and only constructor
	RenderableDrawer(Renderer* r_)
		: r(r_)
	{}

	void render(const Camera& cam,
		uint pass, Renderable& renderable);

private:
	Renderer* r;

	void setupShaderProg(
		const PassLevelKey& key,
		const Camera& cam,
		Renderable& renderable);
};

} // end namespace anki

#endif
