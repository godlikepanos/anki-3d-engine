#include "anki/collision/LineSegment.h"
#include "anki/collision/Plane.h"
#include <algorithm>


namespace anki {


//==============================================================================
LineSegment LineSegment::getTransformed(const Transform& transform) const
{
	LineSegment out;
	out.origin = origin.getTransformed(transform);
	out.dir = transform.getRotation() * (dir * transform.getScale());
	return out;
}


} // end namespace
