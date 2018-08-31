// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Common.h>
#include <anki/math/Vec4.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// https://bartwronski.com/2017/04/13/cull-that-cone/
inline Bool testConeSphere(const Vec4& coneOrigin,
	const Vec4& coneDir,
	F32 coneLength,
	F32 coneAngle,
	const Vec4& sphereCenter,
	F32 sphereRadius)
{
	ANKI_ASSERT(coneOrigin.w() == 0.0f && sphereCenter.w() == 0.0f && coneDir.w() == 0.0f);
	coneAngle /= 2.0f;
	const Vec4 V = sphereCenter - coneOrigin;
	const F32 VlenSq = V.dot(V);
	const F32 V1len = V.dot(coneDir);
	const F32 distanceClosestPoint = cos(coneAngle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(coneAngle);

	const Bool angleCull = distanceClosestPoint > sphereRadius;
	const Bool frontCull = V1len > sphereRadius + coneLength;
	const Bool backCull = V1len < -sphereRadius;
	return !(angleCull || frontCull || backCull);
}

/// Test if two collision shapes collide.
Bool testCollisionShapes(const CollisionShape& a, const CollisionShape& b);
/// @}

} // end namespace anki
