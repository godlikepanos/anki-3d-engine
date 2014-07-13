// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_GJK_EPA_H
#define ANKI_COLLISION_GJK_EPA_H

#include "anki/Math.h"
#include "anki/util/Allocator.h"
#include "anki/collision/ContactPoint.h"
#include "anki/collision/GjkEpaInternal.h"

namespace anki {

// Forward
class ConvexShape;

/// @addtogroup collision
/// @{

/// The implementation of the GJK algorithm. This algorithm is being used for
/// checking the intersection between convex shapes.
class Gjk
{
	friend class GjkEpa;

public:
	/// Return true if the two convex shapes intersect
	Bool intersect(const ConvexShape& shape0, const ConvexShape& shape1);

private:
	using Support = detail::Support;

	Array<Support, 4> m_simplex;
	U32 m_count; ///< Simplex count
	Vec4 m_dir;

	/// Compute the support
	static void support(const ConvexShape& shape0, const ConvexShape& shape1,
		const Vec4& dir, Support& support);

	/// Update simplex
	Bool update(const Support& a);
};

/// The implementation of EPA
class GjkEpa
{
public:
	//detail::Polytope* m_poly; // XXX

	GjkEpa(U32 maxSimplexSize, U32 maxFaceCount, U32 maxIterations)
	:	m_maxSimplexSize(maxSimplexSize),
		m_maxFaceCount(maxFaceCount),
		m_maxIterations(maxIterations)
	{}

	~GjkEpa()
	{}

	Bool intersect(const ConvexShape& shape0, const ConvexShape& shape1,
		ContactPoint& contact, CollisionTempAllocator<U8>& alloc);

private:
	using Support = detail::Support;

	U32 m_maxSimplexSize;
	U32 m_maxFaceCount;
	U32 m_maxIterations;
};

/// @}

} // end namespace anki

#endif

