// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/collision/Functions.h>
#include <anki/collision/Plane.h>

namespace anki
{

static void setPlane(const Vec4& abcd, Plane& p)
{
	Vec4 n = abcd.xyz0();
	F32 len = n.getLength();
	p = Plane(n / len, -abcd.w() / len);
}

void extractClipPlanes(const Mat4& mvp, Array<Plane*, 6>& planes)
{
	// Plane equation coefficients
	Vec4 abcd;

	if(planes[FrustumPlaneType::NEAR])
	{
		abcd = mvp.getRow(3) + mvp.getRow(2);
		setPlane(abcd, *planes[FrustumPlaneType::NEAR]);
	}

	if(planes[FrustumPlaneType::FAR])
	{
		abcd = mvp.getRow(3) - mvp.getRow(2);
		setPlane(abcd, *planes[FrustumPlaneType::FAR]);
	}

	if(planes[FrustumPlaneType::LEFT])
	{
		abcd = mvp.getRow(3) + mvp.getRow(0);
		setPlane(abcd, *planes[FrustumPlaneType::LEFT]);
	}

	if(planes[FrustumPlaneType::RIGHT])
	{
		abcd = mvp.getRow(3) - mvp.getRow(0);
		setPlane(abcd, *planes[FrustumPlaneType::RIGHT]);
	}

	if(planes[FrustumPlaneType::TOP])
	{
		abcd = mvp.getRow(3) - mvp.getRow(1);
		setPlane(abcd, *planes[FrustumPlaneType::TOP]);
	}

	if(planes[FrustumPlaneType::BOTTOM])
	{
		abcd = mvp.getRow(3) + mvp.getRow(1);
		setPlane(abcd, *planes[FrustumPlaneType::BOTTOM]);
	}
}

} // end namespace anki
