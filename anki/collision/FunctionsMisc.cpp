// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Functions.h>

namespace anki
{

void extractClipPlane(const Mat4& mvp, FrustumPlaneType id, Plane& plane)
{
#define ANKI_CASE(i, a, op, b) \
	case i: \
	{ \
		const Vec4 planeEqationCoefs = mvp.getRow(a) op mvp.getRow(b); \
		const Vec4 n = planeEqationCoefs.xyz0(); \
		const F32 len = n.getLength(); \
		plane = Plane(n / len, -planeEqationCoefs.w() / len); \
		break; \
	}

	switch(id)
	{
		ANKI_CASE(FrustumPlaneType::NEAR, 3, +, 2)
		ANKI_CASE(FrustumPlaneType::FAR, 3, -, 2)
		ANKI_CASE(FrustumPlaneType::LEFT, 3, +, 0)
		ANKI_CASE(FrustumPlaneType::RIGHT, 3, -, 0)
		ANKI_CASE(FrustumPlaneType::TOP, 3, -, 1)
		ANKI_CASE(FrustumPlaneType::BOTTOM, 3, +, 1)
	default:
		ANKI_ASSERT(0);
	}

#undef ANKI_CASE
}

void extractClipPlanes(const Mat4& mvp, Array<Plane, 6>& planes)
{
	for(U i = 0; i < 6; ++i)
	{
		extractClipPlane(mvp, static_cast<FrustumPlaneType>(i), planes[i]);
	}
}

void computeEdgesOfFrustum(F32 far, F32 fovX, F32 fovY, Vec4 points[4])
{
	// This came from unprojecting. It works, don't touch it
	const F32 x = far * tan(fovY / 2.0f) * fovX / fovY;
	const F32 y = tan(fovY / 2.0f) * far;
	const F32 z = -far;

	points[0] = Vec4(x, y, z, 0.0f); // top right
	points[1] = Vec4(-x, y, z, 0.0f); // top left
	points[2] = Vec4(-x, -y, z, 0.0f); // bot left
	points[3] = Vec4(x, -y, z, 0.0f); // bot right
}

} // end namespace anki