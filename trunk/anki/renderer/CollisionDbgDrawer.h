#ifndef ANKI_RENDERER_COLLISION_DBG_DRAWER_H
#define ANKI_RENDERER_COLLISION_DBG_DRAWER_H

#include "anki/collision/CollisionShape.h"
#include "anki/util/Assert.h"


namespace anki {


class Dbg;


/// Contains methods to render the collision shapes
class CollisionDbgDrawer: public CollisionShape::ConstVisitor
{
public:
	/// Constructor
	CollisionDbgDrawer(Dbg* dbg_)
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
	Dbg* dbg; ///< The debug stage
};


} // end namespace


#endif
