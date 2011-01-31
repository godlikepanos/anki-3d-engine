#ifndef COLLISION_SHAPE
#define COLLISION_SHAPE


class Plane;


/// Abstract class for collision shapes
class CollisionShape
{
	public:
		enum CollisionShapeType
		{
			CST_LINE_SEG,
			CST_RAY,
			CST_PLANE,
			CST_SPHERE,
			CST_AABB,
			CST_OBB,
			CST_NUM
		};

	CollisionShapeType getType() const {return type;}

	public:
		CollisionShapeType type;

		CollisionShape(CollisionShapeType type_): type(type_) {}

		/// If the bounding volume intersects with the plane then the func returns 0, else it returns the distance. If the
		/// distance is < 0 then the b.v. lies behind the plane and if > 0 then in front of it
		virtual float testPlane(const Plane&) const = 0;
};


#endif
