#ifndef ANKI_COLLISION_COLLISION_SHAPE
#define ANKI_COLLISION_COLLISION_SHAPE

#include "anki/collision/Forward.h"


namespace anki {


/// @addtogroup Collision
/// @{

/// Abstract class for collision shapes
class CollisionShape
{
	public:
		/// Collision shape type
		enum CollisionShapeType
		{
			CST_LINE_SEG,
			CST_RAY,
			CST_PLANE,
			CST_SPHERE,
			CST_AABB,
			CST_OBB,
			CST_PERSPECTIVE_CAMERA_FRUSTRUM
		};

		/// Generic visitor
		class Visitor
		{
			public:
				virtual void visit(LineSegment&) = 0;
				virtual void visit(Obb&) = 0;
				virtual void visit(PerspectiveCameraShape&) = 0;
				virtual void visit(Plane&) = 0;
				virtual void visit(Ray&) = 0;
				virtual void visit(Sphere&) = 0;
		};

		CollisionShape(CollisionShapeType cid_)
		:	cid(cid_)
		{}

		CollisionShapeType getCollisionShapeType() const
		{
			return cid;
		}

		/// Visitor accept
		virtual void accept(Visitor& v) = 0;

		/// If the bounding volume intersects with the plane then the func
		/// returns 0, else it returns the distance. If the distance is < 0
		/// then the collision shape lies behind the plane and if > 0 then in
		/// front of it
		virtual float testPlane(const Plane& p) const = 0;

	private:
		CollisionShapeType cid;
};
/// @}


} // end namespace


#endif
