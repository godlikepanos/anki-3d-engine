#ifndef COLLISION_SHAPE
#define COLLISION_SHAPE


class Plane;


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
				virtual void visit(class LineSegment&) = 0;
		};

		CollisionShape(CollisionShapeType cid_)
		:	cid(cid_)
		{}

		CollisionShapeType getCollisionShapeType() const
		{
			return cid;
		}

		/// If the bounding volume intersects with the plane then the func
		/// returns 0, else it returns the distance. If the distance is < 0
		/// then the collision shape lies behind the plane and if > 0 then in
		/// front of it
		virtual float testPlane(const Plane& p) const = 0;

	private:
		CollisionShapeType cid;
};
/// @}


#endif
