// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>
#include <anki/collision/Common.h>

namespace anki
{
namespace detail
{

// Forward
class Polytope;
class Face;

/// @addtogroup collision_internal
/// @{

/// Edge of a polytope
class Edge
{
public:
	Array<U32, 2> m_idx;
	Face* m_face;

	Edge(U32 i, U32 j, Face* face)
		: m_idx{{i, j}}
		, m_face(face)
	{
	}

	Edge(const Edge&) = default;

	Bool operator==(const Edge& b) const
	{
		return (m_idx[0] == b.m_idx[1] && m_idx[1] == b.m_idx[0]) || (m_idx[0] == b.m_idx[0] && m_idx[1] == b.m_idx[1]);
	}
};

class EdgeHasher
{
public:
	PtrSize operator()(const Edge& e) const
	{
		if(e.m_idx[0] < e.m_idx[1])
		{
			return (e.m_idx[1] << 16) | e.m_idx[0];
		}
		else
		{
			return (e.m_idx[0] << 16) | e.m_idx[1];
		}
	}
};

class EdgeCompare
{
public:
	Bool operator()(const Edge& a, const Edge& b) const
	{
		return a == b;
	}
};

/// The face of the polytope used for EPA
class Face
{
public:
	Face()
	{
		m_normal[0] = -100.0;
		m_dist = -1.0;
		m_originInside = 2;
		m_dead = false;
#if ANKI_EXTRA_CHECKS
		m_idx[0] = m_idx[1] = m_idx[2] = MAX_U32;
#endif
	}

	Face(U i, U j, U k)
		: Face()
	{
		m_idx[0] = i;
		m_idx[1] = j;
		m_idx[2] = k;
	}

	const Array<U32, 3>& idx() const
	{
		return m_idx;
	}

	const Vec4& normal(Polytope& poly, Bool fixWinding = false) const;

	F32 distance(Polytope& poly) const
	{
		normal(poly);
		return m_dist;
	}

	Bool originInside(Polytope& poly) const;

	Edge edge(Polytope& poly, U i)
	{
		normal(poly);
		return Edge(m_idx[i], (i == 2) ? m_idx[0] : m_idx[i + 1], this);
	}

	Bool dead() const
	{
		return m_dead;
	}

	void kill()
	{
		m_dead = true;
	}

	void revive()
	{
		m_dead = false;
	}

private:
	mutable Array<U32, 3> m_idx;
	mutable Vec4 m_normal;
	mutable F32 m_dist; ///< Distance from the origin
	mutable U8 m_originInside; ///< 0: Not use it, 1: Use it, 2: Initial
	Bool8 m_dead;
};

/// The polytope used for EPA
class Polytope
{
	friend class Face;

public:
	Polytope(CollisionTempAllocator<U8>& alloc, U32 maxSimplexSize, U32 maxFaceCount)
		: m_maxSimplexSize(maxSimplexSize)
		, m_maxFaceCount(maxFaceCount)
		, m_simplex(alloc)
		, m_faces(alloc)
	{
	}

	void init(const Array<Support, 4>& gjkSupport);

	Bool addSupportAndExpand(const Support& s, Face& cface)
	{
		Bool found;
		U idx;
		addNewSupport(s, found, idx);

		if(!found)
		{
			return expand(cface, idx);
		}
		else
		{
			return false;
		}
	}

	/// Find closest face to the origin
	Face& findClosestFace();

public: // XXX
	U32 m_maxSimplexSize;
	U32 m_maxFaceCount;

	Vector<Support, CollisionTempAllocator<Support>> m_simplex;
	Vector<Face, CollisionTempAllocator<Face>> m_faces;

	CollisionTempAllocator<U8> getAllocator() const
	{
		return m_simplex.get_allocator();
	}

	void addNewSupport(const Support& s, Bool& found, U& idx);

	Bool expand(Face& cface, U supportIdx);
};
/// @}

} // end namesapce detail
} // end namesapce anki
