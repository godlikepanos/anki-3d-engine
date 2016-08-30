// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	if(planes[Frustum::PlaneType::NEAR])
	{
		abcd = mvp.getRow(3) + mvp.getRow(2);
		setPlane(abcd, *planes[Frustum::PlaneType::NEAR]);
	}

	if(planes[Frustum::PlaneType::FAR])
	{
		abcd = mvp.getRow(3) - mvp.getRow(2);
		setPlane(abcd, *planes[Frustum::PlaneType::FAR]);
	}

	if(planes[Frustum::PlaneType::LEFT])
	{
		abcd = mvp.getRow(3) + mvp.getRow(0);
		setPlane(abcd, *planes[Frustum::PlaneType::LEFT]);
	}

	if(planes[Frustum::PlaneType::RIGHT])
	{
		abcd = mvp.getRow(3) - mvp.getRow(0);
		setPlane(abcd, *planes[Frustum::PlaneType::RIGHT]);
	}

	if(planes[Frustum::PlaneType::TOP])
	{
		abcd = mvp.getRow(3) - mvp.getRow(1);
		setPlane(abcd, *planes[Frustum::PlaneType::TOP]);
	}

	if(planes[Frustum::PlaneType::BOTTOM])
	{
		abcd = mvp.getRow(3) + mvp.getRow(1);
		setPlane(abcd, *planes[Frustum::PlaneType::BOTTOM]);
	}
}

} // end namespace anki
