#include "anki/collision/Frustum.h"
#include "anki/collision/LineSegment.h"


namespace anki {


//==============================================================================
Frustum& Frustum::operator=(const Frustum& b)
{
	for(int i = 0; i < planes.size(); ++i)
	{
		planes[i] = b.planes[i];
	}

	eye = b.eye;

	for(int i = 0; i < dirs.size(); ++i)
	{
		dirs[i] = b.dirs[i];
	}
}


//==============================================================================
float Frustum::testPlane(const Plane& p) const
{
	const Frustum& f = *this;
	float o = 0.0;

	for(uint i = 0; i < 4; i++)
	{
		LineSegment ls(f.getEye(), f.getDirections()[i]);
		float t = ls.testPlane(p);

		if(t == 0)
		{
			return 0.0;
		}
		else if(t < 0.0)
		{
			o = std::max(o, t);
		}
		else
		{
			o = std::min(o, t);
		}
	}

	return o;
}


} // end namespace
