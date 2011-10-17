#include "anki/collision/Ray.h"
#include "anki/collision/Plane.h"


namespace anki {


//==============================================================================
// getTransformed                                                              =
//==============================================================================
Ray Ray::getTransformed(const Transform& transform) const
{
	Ray out;
	out.origin = origin.getTransformed(transform);
	out.dir = transform.getRotation() * dir;
	return out;
}


} // end namespace
