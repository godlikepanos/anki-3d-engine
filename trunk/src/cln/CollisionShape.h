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
		enum ClassId
		{
			CID_LINE_SEG,
			CID_RAY,
			CID_PLANE,
			CID_SPHERE,
			CID_AABB,
			CID_OBB
		};

		CollisionShape(ClassId cid_): cid(cid_) {}

		ClassId getClassId() const {return cid;}

		/// If the bounding volume intersects with the plane then the func
		/// returns 0, else it returns the distance. If the distance is < 0
		/// then the b.v. lies behind the plane and if > 0 then in front of it
		virtual float testPlane(const Plane&) const = 0;

	private:
		ClassId cid;
};
/// @}


#endif
