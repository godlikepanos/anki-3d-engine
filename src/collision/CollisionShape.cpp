#include "anki/collision/CollisionShape.h"
#include "anki/Collision.h"

namespace anki {

//==============================================================================
void CollisionShape::toAabb(Aabb& out) const
{
	switch(cid)
	{
	case CST_LINE_SEG:
		static_cast<const LineSegment*>(this)->toAabb(out);
		break;
	case CST_RAY:
		static_cast<const Ray*>(this)->toAabb(out);
		break;
	case CST_PLANE:
		static_cast<const Plane*>(this)->toAabb(out);
		break;
	case CST_SPHERE:
		static_cast<const Sphere*>(this)->toAabb(out);
		break;
	case CST_AABB:
		static_cast<const Aabb*>(this)->toAabb(out);
		break;
	case CST_OBB:
		static_cast<const Obb*>(this)->toAabb(out);
		break;
	case CST_FRUSTUM:
		static_cast<const Frustum*>(this)->toAabb(out);
		break;
	default:
		ANKI_ASSERT(0);
	};
}

} // end namespace anki
