#include "anki/collision/Functions.h"

namespace anki {

//==============================================================================
void extractClipPlanes(const Mat4& mvp, Plane* planes[Frustum::FP_COUNT])
{
	// Plane equation coefficients
	F32 a, b, c, d;

	if(planes[Frustum::FP_NEAR])
	{
		a = mvp(3, 0) + mvp(2, 0);
		b = mvp(3, 1) + mvp(2, 1);
		c = mvp(3, 2) + mvp(2, 2);
		d = mvp(3, 3) + mvp(2, 3);

		*planes[Frustum::FP_NEAR] = Plane(a, b, c, d);
	}

	if(planes[Frustum::FP_FAR])
	{
		a = mvp(3, 0) - mvp(2, 0);
		b = mvp(3, 1) - mvp(2, 1);
		c = mvp(3, 2) - mvp(2, 2);
		d = mvp(3, 3) - mvp(2, 3);

		*planes[Frustum::FP_FAR] = Plane(a, b, c, d);
	}

	if(planes[Frustum::FP_LEFT])
	{
		a = mvp(3, 0) + mvp(0, 0);
		b = mvp(3, 1) + mvp(0, 1);
		c = mvp(3, 2) + mvp(0, 2);
		d = mvp(3, 3) + mvp(0, 3);

		*planes[Frustum::FP_LEFT] = Plane(a, b, c, d);
	}

	if(planes[Frustum::FP_RIGHT])
	{
		a = mvp(3, 0) - mvp(0, 0);
		b = mvp(3, 1) - mvp(0, 1);
		c = mvp(3, 2) - mvp(0, 2);
		d = mvp(3, 3) - mvp(0, 3);

		*planes[Frustum::FP_RIGHT] = Plane(a, b, c, d);
	}

	if(planes[Frustum::FP_TOP])
	{
		a = mvp(3, 0) - mvp(1, 0);
		b = mvp(3, 1) - mvp(1, 1);
		c = mvp(3, 2) - mvp(1, 2);
		d = mvp(3, 3) - mvp(1, 3);

		*planes[Frustum::FP_TOP] = Plane(a, b, c, d);
	}

	if(planes[Frustum::FP_BOTTOM])
	{
		a = mvp(3, 0) + mvp(1, 0);
		b = mvp(3, 1) + mvp(1, 1);
		c = mvp(3, 2) + mvp(1, 2);
		d = mvp(3, 3) + mvp(1, 3);

		*planes[Frustum::FP_BOTTOM] = Plane(a, b, c, d);
	}	
}

} // end namespace anki
