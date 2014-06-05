// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Functions.h"

namespace anki {

//==============================================================================
void extractClipPlanes(const Mat4& mvp, 
	Plane* planes[(U)Frustum::PlaneType::COUNT])
{
	// Plane equation coefficients
	F32 a, b, c, d;

	if(planes[(U)Frustum::PlaneType::NEAR])
	{
		a = mvp(3, 0) + mvp(2, 0);
		b = mvp(3, 1) + mvp(2, 1);
		c = mvp(3, 2) + mvp(2, 2);
		d = mvp(3, 3) + mvp(2, 3);

		*planes[(U)Frustum::PlaneType::NEAR] = Plane(a, b, c, d);
	}

	if(planes[(U)Frustum::PlaneType::FAR])
	{
		a = mvp(3, 0) - mvp(2, 0);
		b = mvp(3, 1) - mvp(2, 1);
		c = mvp(3, 2) - mvp(2, 2);
		d = mvp(3, 3) - mvp(2, 3);

		*planes[(U)Frustum::PlaneType::FAR] = Plane(a, b, c, d);
	}

	if(planes[(U)Frustum::PlaneType::LEFT])
	{
		a = mvp(3, 0) + mvp(0, 0);
		b = mvp(3, 1) + mvp(0, 1);
		c = mvp(3, 2) + mvp(0, 2);
		d = mvp(3, 3) + mvp(0, 3);

		*planes[(U)Frustum::PlaneType::LEFT] = Plane(a, b, c, d);
	}

	if(planes[(U)Frustum::PlaneType::RIGHT])
	{
		a = mvp(3, 0) - mvp(0, 0);
		b = mvp(3, 1) - mvp(0, 1);
		c = mvp(3, 2) - mvp(0, 2);
		d = mvp(3, 3) - mvp(0, 3);

		*planes[(U)Frustum::PlaneType::RIGHT] = Plane(a, b, c, d);
	}

	if(planes[(U)Frustum::PlaneType::TOP])
	{
		a = mvp(3, 0) - mvp(1, 0);
		b = mvp(3, 1) - mvp(1, 1);
		c = mvp(3, 2) - mvp(1, 2);
		d = mvp(3, 3) - mvp(1, 3);

		*planes[(U)Frustum::PlaneType::TOP] = Plane(a, b, c, d);
	}

	if(planes[(U)Frustum::PlaneType::BOTTOM])
	{
		a = mvp(3, 0) + mvp(1, 0);
		b = mvp(3, 1) + mvp(1, 1);
		c = mvp(3, 2) + mvp(1, 2);
		d = mvp(3, 3) + mvp(1, 3);

		*planes[(U)Frustum::PlaneType::BOTTOM] = Plane(a, b, c, d);
	}	
}

} // end namespace anki
