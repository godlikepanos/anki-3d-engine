#ifndef ANKI_COLLISION_COLLISION_SHAPE
#define ANKI_COLLISION_COLLISION_SHAPE

#include "anki/collision/Forward.h"
#include "anki/math/Forward.h"
#include "anki/util/Visitor.h"


namespace anki {


/// @addtogroup Collision
/// @{

/// Abstract class for collision shapes
class CollisionShape: public Visitable<LineSegment, Obb,
	PerspectiveCameraShape, Plane, Ray, Sphere, Aabb>
{
public:
	typedef Visitable<LineSegment, Obb,
		PerspectiveCameraShape, Plane, Ray, Sphere, Aabb> BaseType;
	typedef BaseType::MutableVisitorType MutableVisitor;
	typedef BaseType::ConstVisitorType ConstVisitor;

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

	CollisionShape(CollisionShapeType cid_)
		: cid(cid_)
	{}

	CollisionShapeType getCollisionShapeType() const
	{
		return cid;
	}

	/// If the collision shape intersects with the plane then the method
	/// returns 0.0, else it returns the distance. If the distance is < 0.0
	/// then the collision shape lies behind the plane and if > 0.0 then
	/// in front of it
	virtual float testPlane(const Plane& p) const = 0;

	/// Transform
	virtual void transform(const Transform& trf) = 0;

private:
	CollisionShapeType cid;
};
/// @}


} // end namespace


#endif
