#ifndef ANKI_RENDERER_COLLISION_DBG_DRAWER_H
#define ANKI_RENDERER_COLLISION_DBG_DRAWER_H

#include "anki/collision/CollisionShape.h"
#include "anki/util/Assert.h"


namespace anki {


class Sphere;
class Obb;
class Plane;
class Dbg;


/// Contains methods to render the collision shapes
class CollisionDbgDrawer: public CollisionShape::ConstVisitor
{
	public:
		/// Constructor
		CollisionDbgDrawer(Dbg& dbg_)
		:	dbg(dbg_)
		{}

		void visit(const LineSegment&)
		{
			ASSERT(0 && "ToDo");
		}

		void visit(const Obb&);

		void visit(const PerspectiveCameraShape&)
		{
			ASSERT(0 && "ToDo");
		}

		void visit(const Plane&);

		void visit(const Ray&)
		{
			ASSERT(0 && "ToDo");
		}

		void visit(const Sphere&);

		void visit(const Aabb&);

	private:
		Dbg& dbg; ///< The debug stage
};


} // end namespace


#endif
