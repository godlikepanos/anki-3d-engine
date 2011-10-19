#include "anki/collision/CollisionShapeTransform.h"
#include "anki/collision/Collision.h"
#include "anki/math/Transform.h"


namespace anki {


//==============================================================================
void CollisionShapeTransform::transform(const Transform& trf,
	CollisionShape& cs)
{
	switch(cs.getCollisionShapeType())
	{
		case CollisionShape::CST_LINE_SEG:
			static_cast<LineSegment&>(cs).transform(trf);
			break;
		case CollisionShape::CST_RAY:
			static_cast<Ray&>(cs).transform(trf);
			break;
		case CollisionShape::CST_PLANE:
			static_cast<Plane&>(cs).transform(trf);
			break;
		case CollisionShape::CST_SPHERE:
			static_cast<Sphere&>(cs).transform(trf);
			break;
		case CollisionShape::CST_AABB:
			static_cast<Aabb&>(cs).transform(trf);
			break;
		case CollisionShape::CST_OBB:
			static_cast<Obb&>(cs).transform(trf);
			break;
		case CollisionShape::CST_PERSPECTIVE_CAMERA_FRUSTRUM:
			static_cast<PerspectiveCameraShape&>(cs).transform(trf);
			break;
		default:
			ASSERT(0 && "Forgot something");
	}
}


} // end namespace
