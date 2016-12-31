// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Common.h>
#include <anki/collision/Frustum.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Extract the clip planes using an MVP matrix
///
/// @param[in] mvp The MVP matrix.
/// @param[out] planes Pointers to the planes. Elements can be nullptr
///
/// @note plane_count * 8 muls, plane_count sqrt
void extractClipPlanes(const Mat4& mvp, Array<Plane*, 6>& planes);

/// See extractClipPlanes.
inline void extractClipPlanes(const Mat4& mvp, Array<Plane, 6>& planes)
{
	Array<Plane*, 6> ptrs = {{&planes[0], &planes[1], &planes[2], &planes[3], &planes[4], &planes[5]}};
	extractClipPlanes(mvp, ptrs);
}
/// @}

} // end namespace anki
