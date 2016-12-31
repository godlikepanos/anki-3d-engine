// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>

namespace anki
{

// Forward
class ConvexShape;

/// @addtogroup collision_internal
/// @{

/// GJK support
class GjkSupport
{
public:
	Vec4 m_v;
	Vec4 m_v0;
	Vec4 m_v1;

	Bool operator==(const GjkSupport& b) const
	{
		return m_v == b.m_v && m_v0 == b.m_v0 && m_v1 == b.m_v1;
	}
};
/// @}

/// @addtogroup collision
/// @{

/// The implementation of the GJK algorithm. This algorithm is being used for checking the intersection between convex
/// shapes.
class Gjk
{
	friend class GjkEpa;

public:
	/// Return true if the two convex shapes intersect
	Bool intersect(const ConvexShape& shape0, const ConvexShape& shape1);

private:
	using Support = GjkSupport;

	Array<Support, 4> m_simplex;
	U32 m_count; ///< Simplex count
	Vec4 m_dir;

	/// Compute the support
	static void support(const ConvexShape& shape0, const ConvexShape& shape1, const Vec4& dir, Support& support);

	/// Update simplex
	Bool update(const Support& a);
};
/// @}

} // end namespace anki
