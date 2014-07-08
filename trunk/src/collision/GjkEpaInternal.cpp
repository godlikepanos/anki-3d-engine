#include "anki/collision/GjkEpaInternal.h"
#include <unordered_map>
#include <algorithm>

namespace anki {
namespace detail {

//==============================================================================
// Edge                                                                        =
//==============================================================================

//==============================================================================
static inline std::ostream& operator<<(std::ostream& os, const Edge& e) 
{
	os << e.m_idx[0] << " " << e.m_idx[1];
	return os;
}

//==============================================================================
// Face                                                                        =
//==============================================================================

//==============================================================================
const Vec4& Face::normal(Polytope& poly, Bool fixWinding) const
{
	if(m_normal[0] == -100.0)
	{
		const Vec4& a = poly.m_simplex[m_idx[0]].m_v;
		const Vec4& b = poly.m_simplex[m_idx[1]].m_v;
		const Vec4& c = poly.m_simplex[m_idx[2]].m_v;

		// Compute the face edges
		Vec4 e0 = b - a;
		Vec4 e1 = c - b;

		// Compute the face normal
		m_normal = e0.cross(e1);
		m_normal.normalize();

		// Compute the distance as well for the winding
		
		// Calculate the distance from the origin to the edge
		m_dist = m_normal.dot(a);

		// Check the winding
		if(fixWinding && m_dist < 0.0)
		{
			// It's not facing the origin so fix some stuff

			m_dist = -m_dist;
			m_normal = -m_normal;

			// Swap some indices
			auto idx = m_idx[0];
			m_idx[0] = m_idx[1];
			m_idx[1] = idx;
		}
	}

	return m_normal;
}

//==============================================================================
Bool Face::originInside(Polytope& poly) const
{
	if(m_originInside == 2)
	{
		const Vec4& a = poly.m_simplex[m_idx[0]].m_v;
		const Vec4& b = poly.m_simplex[m_idx[1]].m_v;
		const Vec4& c = poly.m_simplex[m_idx[2]].m_v;
		const Vec4& n = normal(poly);

		m_originInside = 1;

		// Compute the face edges
		Vec4 e0 = b - a;
		Vec4 e1 = c - b;
		Vec4 e2 = a - c;

		Vec4 adjacentNormal;
		F32 d;

		// Check the 1st edge
		adjacentNormal = e0.cross(n);
		d = adjacentNormal.dot(a);
		if(d <= 0.0)
		{
			m_originInside = 0;
		}

		// Check the 2nd edge
		if(m_originInside == 1)
		{
			adjacentNormal = e1.cross(n);
			d = adjacentNormal.dot(b);

			if(d <= 0.0)
			{
				m_originInside = 0;
			}
		}

		// Check the 3rd edge
		if(m_originInside == 1)
		{
			adjacentNormal = e2.cross(n);
			d = adjacentNormal.dot(c);

			if(d <= 0.0)
			{
				m_originInside = 0;
			}
		}
	}

	return m_originInside;
}

//==============================================================================
// Polytope                                                                    =
//==============================================================================

//==============================================================================
void Polytope::init(const Array<Support, 4>& gjkSupport)
{
	// Allocate memory up front
	m_simplex.reserve(m_maxSimplexSize);
	m_faces.reserve(m_maxFaceCount);

	// Set the simplex
	m_simplex.push_back(gjkSupport[0]);
	m_simplex.push_back(gjkSupport[1]);
	m_simplex.push_back(gjkSupport[2]);
	m_simplex.push_back(gjkSupport[3]);

	// Set the first 4 faces
	m_faces.emplace_back(0, 1, 2);
	m_faces.emplace_back(1, 2, 3);
	m_faces.emplace_back(2, 3, 0);
	m_faces.emplace_back(3, 0, 1);

	// Pre-calc the normals and fix winding
	for(Face& face : m_faces)
	{
		face.normal(*this, true);
	}
}

//==============================================================================
Face& Polytope::findClosestFace()
{
	F32 minDistance = MAX_F32;
	U index = MAX_U32;

	// Iterate the faces
	for(U i = 0; i < m_faces.size(); i++) 
	{
		Face& face = m_faces[i];

		// Check the distance against the other distances
		if(!face.dead() && face.distance(*this) < minDistance)
		{
			if(face.originInside(*this))
			{
				// We have a candidate
				index = i;
				minDistance = face.distance(*this);
			}
			else
			{
				std::cout << "origin not in face" << std::endl;
			}
		}
	}

	return m_faces[index];
}

//==============================================================================
void Polytope::addNewSupport(const Support& s, Bool& found, U& idx)
{
	// Search if support is already there
	U i;
	for(i = 0; i < m_simplex.size(); i++)
	{
		if(m_simplex[i] == s)
		{
			break;
		}
	}

	idx = i;

	if(i == m_simplex.size())
	{
		// Not found
		m_simplex.push_back(s);
		found = false;
	}
	else
	{
		found = true;
	}
}

//==============================================================================
Bool Polytope::expand(Face& cface, U supportIdx)
{
	if(m_faces.size() + 2 > m_maxFaceCount)
	{
		// Reached a limit, cannot expand 
		return false;
	}

	//
	// First add the point to the polytope by spliting the cface.
	// Don't fix winding to the new faces because it should be correct.
	//
	Array<U32, 3> idx = cface.idx();

	// Canibalize existing face
	cface = Face(idx[0], idx[1], supportIdx);
	cface.normal(*this);

	// Next face
	m_faces.emplace_back(idx[1], idx[2], supportIdx);
	m_faces.back().normal(*this);

	// Next face
	m_faces.emplace_back(idx[2], idx[0], supportIdx);
	m_faces.back().normal(*this);

	Array<Face*, 3> newFaces = {
		&cface, &m_faces[m_faces.size() - 2], &m_faces[m_faces.size() - 1]};

	//return true;

	//
	// Find other faces that hide the new support
	//
	Vector<Face*, StackAllocator<Face*>> badFaces(getAllocator());
	Bool badFacesVectorInitialized = false; // Opt

	const Vec4& support = m_simplex[supportIdx].m_v;

	for(Face& face : m_faces)
	{
		if(face.dead() 
			|| &face == newFaces[0]
			|| &face == newFaces[1]
			|| &face == newFaces[2])
		{
			// Either dead or one of the new faces
			continue;
		}

		F32 dot = face.normal(*this).dot(support);
		if(dot > getEpsilon<F32>())
		{
			if(!badFacesVectorInitialized)
			{
				badFaces.reserve(20);
				badFacesVectorInitialized = true;
			}

			badFaces.push_back(&face);
			face.kill();
		}
	}

	if(badFaces.size() == 0)
	{
		// Early exit
		return true;
	}

	std::cout << "found bad faces " << badFaces.size() << std::endl;

	//
	// Out of the new 3 faces find those that share edges with the bad faces
	//
	Array<Face*, 3> newFacesBad = {nullptr, nullptr, nullptr};
	U newFacesBadCount = 0;

	for(Face* face : badFaces)
	{
		for(Face* nface : newFaces)
		{
			for(U i = 0; i < 3; i++)
			{
				for(U j = 0; j < 3; j++)
				{
					if(nface->edge(*this, i) == face->edge(*this, j))
					{
						newFacesBad[newFacesBadCount++] = nface;
						nface->kill();
					}
				}
			}
		}
	}

	ANKI_ASSERT(newFacesBadCount != 0);

	for(U i = 0; i < newFacesBadCount; i++)
	{
		badFaces.push_back(newFacesBad[i]);
	}

	//
	// Get the edges of the deleted faces that belong to the surface of the
	// deleted faces
	//
	std::unordered_map<Edge, U, EdgeHasher, EdgeCompare, 
		StackAllocator<std::pair<Edge, U>>> edgeMap(
		10, EdgeHasher(), EdgeCompare(), getAllocator());

	for(Face* face : badFaces)
	{
		for(U i = 0; i < 3; i ++)
		{
			Edge e = face->edge(*this, i);
			auto it = edgeMap.find(e);

			if(it == edgeMap.end())
			{
				// Not found
				edgeMap[e] = 1;
			}
			else
			{
				++(it->second);
			}
		}
	}

	//
	// Get the edges that are in a loop
	//
	Vector<Edge, StackAllocator<Edge>> edgeLoopTmp(getAllocator());
	edgeLoopTmp.reserve(edgeMap.size());
	U startEdge = MAX_U32;

	for(auto& it : edgeMap)
	{
		if(it.second == 1)
		{
			edgeLoopTmp.push_back(it.first);

			if(it.first.m_idx[0] == supportIdx)
			{
				startEdge = edgeLoopTmp.size() - 1;
			}
		}
	}

	ANKI_ASSERT(startEdge != MAX_U32);
	ANKI_ASSERT(edgeLoopTmp.size() > 2);

	//
	// Sort those edges to a continues loop starting from the edge with the 
	// support
	// 
	Vector<Edge, StackAllocator<Edge>> edgeLoop(getAllocator());
	edgeLoop.reserve(edgeMap.size());

	edgeLoop.push_back(edgeLoopTmp[startEdge]); // First edge

	while(edgeLoop.size() != edgeLoopTmp.size())
	{
		for(Edge& e : edgeLoopTmp)
		{
			if(edgeLoop.back().m_idx[1] == e.m_idx[0])
			{
				edgeLoop.push_back(e);
				break;
			}
		}
	}

	ANKI_ASSERT(edgeLoop[0].m_idx[0] == supportIdx);

	ANKI_ASSERT(edgeLoop[0].m_idx[0] == edgeLoop.back().m_idx[1] 
		&& "Edge loop is not closed");

	//
	// Spawn faces from the edge loop
	//
	Edge edge = edgeLoop[0];
	for(U i = 1; i < edgeLoop.size() - 1; i++)
	{
		ANKI_ASSERT(edge.m_idx[1] == edgeLoop[i].m_idx[0]);

		m_faces.emplace_back(
			edge.m_idx[0],
			edge.m_idx[1],
			edgeLoop[i].m_idx[1]);

		edge = m_faces.back().edge(*this, 2);
		std::swap(edge.m_idx[0], edge.m_idx[1]);
	}

	return true;
}

} // end namesapce detail
} // end namesapce anki

