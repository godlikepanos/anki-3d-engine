// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_GJK_EPA_H
#define ANKI_COLLISION_GJK_EPA_H

#include "anki/Math.h"
#include "anki/collision/ContactPoint.h"

namespace anki {

// Forward
class ConvexShape;

/// @addtogroup collision
/// @{

/// The implementation of the GJK algorithm. This algorithm is being used for
/// checking the intersection between convex shapes.
class Gjk
{
public:
	/// Return true if the two convex shapes intersect
	Bool intersect(const ConvexShape& shape0, const ConvexShape& shape1);

protected:
	U32 m_count; ///< Simplex count
	Vec4 m_a, m_b, m_c, m_d; ///< Simplex
	Vec4 m_dir;

	/// Compute the support
	static Vec4 support(const ConvexShape& shape0, const ConvexShape& shape1,
		const Vec4& dir);

	/// Update simplex
	Bool update(const Vec4& a);
};

/// The implementation of EPA
class GjkEpa: public Gjk
{
public:
	Bool intersect(const ConvexShape& shape0, const ConvexShape& shape1,
		ContactPoint& contact);

private:
	static const U MAX_SIMPLEX_COUNT = 20;
	static const U MAX_FACE_COUNT = 50;

	class Face
	{
	public:
		Array<U32, 3> m_idx;
		Vec4 m_normal;
		F32 m_dist;
	};

	Array<Vec4, MAX_SIMPLEX_COUNT> m_simplex;

	Array<Face, MAX_FACE_COUNT> m_faces;
	U32 m_faceCount;

	void findClosestFace(U& index);
};

/// @}

} // end namespace anki

#endif

