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

public: // XXX
	class Support
	{
	public:
		Vec4 m_v;
		Vec4 m_v0;
		Vec4 m_v1;
	};

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
class GjkEpa: public Gjk
{
public:
	Bool intersect(const ConvexShape& shape0, const ConvexShape& shape1,
		ContactPoint& contact);

public: // XXX
	static const U MAX_SIMPLEX_COUNT = 50;
	static const U MAX_FACE_COUNT = 100;

	class Face
	{
	public:
		Array<U32, 3> m_idx;
		Vec4 m_normal;
		F32 m_dist; ///< Distance from the origin
		U8 m_originInside;

		void init()
		{
			m_normal[0] = -100.0;
			m_originInside = 2;
		}
	};

	Array<Support, MAX_SIMPLEX_COUNT> m_simplexArr;

	Array<Face, MAX_FACE_COUNT> m_faces;
	U32 m_faceCount;

	void findClosestFace(U& index);

	void expandPolytope(Face& closestFace, const Vec4& point);

	void computeFace(Face& face);
};

/// @}

} // end namespace anki

#endif

