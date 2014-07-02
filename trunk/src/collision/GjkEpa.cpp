// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE
// Inspired by http://vec3.ca/gjk/implementation/

#include "anki/collision/GjkEpa.h"
#include "anki/collision/ConvexShape.h"

namespace anki {

//==============================================================================
// Gjk                                                                         =
//==============================================================================

//==============================================================================
/// Helper of (axb)xa
static Vec4 crossAba(const Vec4& a, const Vec4& b)
{
	// We need to calculate the (axb)xa but we can use the triple product
	// property ax(bxc) = b(a.c) - c(a.b) to make it faster
	Vec4 out = b * (a.dot(a)) - a * (a.dot(b));

	return out;
}

//==============================================================================
void Gjk::support(const ConvexShape& shape0, const ConvexShape& shape1,
	const Vec4& dir, Support& support)
{
	support.m_v0 = shape0.computeSupport(dir);
	support.m_v1 = shape1.computeSupport(-dir);
	support.m_v = support.m_v0 - support.m_v1;
}

//==============================================================================
Bool Gjk::update(const Support& a)
{
	if(m_count == 2)
	{
		Vec4 ao = -a.m_v;
		 
		// Compute the vectors parallel to the edges we'll test
		Vec4 ab = m_simplex[1].m_v - a.m_v;
		Vec4 ac = m_simplex[2].m_v - a.m_v;
		 
		// Compute the triangle's normal
		Vec4 abc = ab.cross(ac);
		 
		// Compute a vector within the plane of the triangle,
		// Pointing away from the edge ab
		Vec4 abp = ab.cross(abc);
		 
		if(abp.dot(ao) > 0.0)
		{
			// The origin lies outside the triangle, near the edge ab
			m_simplex[2] = m_simplex[1];
			m_simplex[1] = a;
		 
			m_dir = crossAba(ab, ao);
		 
			return false;
		}

		// Perform a similar test for the edge ac
		Vec4 acp = abc.cross(ac);
		 
		if(acp.dot(ao) > 0.0)
		{
			m_simplex[1] = a;
			m_dir = crossAba(ac, ao);

			return false;
		}
		 
		// If we get here, then the origin must be within the triangle, 
		// but we care whether it is above or below it, so test
		if(abc.dot(ao) > 0.0)
		{
			m_simplex[3] = m_simplex[2];
			m_simplex[2] = m_simplex[1];
			m_simplex[1] = a;
		 
			m_dir = abc;
		}
		else
		{
			m_simplex[3] = m_simplex[1];
			m_simplex[1] = a;
		 
			m_dir = -abc;
		}
		 
		m_count = 3;
		 
		// Again, need a tetrahedron to enclose the origin
		return false;
	}
	else if(m_count == 3)
	{
		Vec4 ao = -a.m_v;
		 
		Vec4 ab = m_simplex[1].m_v - a.m_v;
		Vec4 ac = m_simplex[2].m_v - a.m_v;
		 
		Vec4 abc = ab.cross(ac);
		 
		Vec4 ad, acd, adb;
		 
		if(abc.dot(ao) > 0.0)
		{
			// In front of triangle ABC
			goto check_face;
		}
		 
		ad = m_simplex[3].m_v - a.m_v;
		acd = ac.cross(ad);
		 
		if(acd.dot(ao) > 0.0)
		{
			// In front of triangle ACD
			m_simplex[1] = m_simplex[2];
			m_simplex[2] = m_simplex[3];
		 
			ab = ac;
			ac = ad;
		 
			abc = acd;
			goto check_face;
		}
		 
		adb = ad.cross(ab);
		 
		if(adb.dot(ao) > 0.0)
		{
			// In front of triangle ADB
		 
			m_simplex[2] = m_simplex[1];
			m_simplex[1] = m_simplex[3];
		 
			ac = ab;
			ab = ad;
		 
			abc = adb;
			goto check_face;
		}
		 
		// Behind all three faces, the origin is in the tetrahedron, we're done
		m_simplex[0] = a;
		m_count = 4;
		return true;
		 
check_face:
		 
		// We have a CCW wound triangle ABC
		// the point is in front of this triangle
		// it is NOT "below" edge BC
		// it is NOT "above" the plane through A that's parallel to BC
		Vec4 abp = ab.cross(abc);
		 
		if(abp.dot(ao) > 0.0)
		{
			m_simplex[2] = m_simplex[1];
			m_simplex[1] = a;
		 
			m_dir = crossAba(ab, ao);
		 
			m_count = 2;
			return false;
		}
		 
		Vec4 acp = abc.cross(ac);	
		 
		if(acp.dot(ao) > 0.0)
		{
			m_simplex[1] = a;
		 
			m_dir = crossAba(ac, ao);
		 
			m_count = 2;
			return false;
		}
		 
		m_simplex[3] = m_simplex[2];
		m_simplex[2] = m_simplex[1];
		m_simplex[1] = a;
		 
		m_dir = abc;
		m_count = 3;
		 
		return false;		
	}

	ANKI_ASSERT(0);
	return true;
}

//==============================================================================
Bool Gjk::intersect(const ConvexShape& shape0, const ConvexShape& shape1)
{
	// Chose random direction
	m_dir = Vec4(1.0, 0.0, 0.0, 0.0);

	// Do cases 1, 2
	support(shape0, shape1, m_dir, m_simplex[2]);
	if(m_simplex[2].m_v.dot(m_dir) < 0.0)
	{
		return false;
	}
 
	m_dir = -m_simplex[2].m_v;
	support(shape0, shape1, m_dir, m_simplex[1]);

	if(m_simplex[1].m_v.dot(m_dir) < 0.0)
	{
		return false;
	}

	m_dir = crossAba(m_simplex[2].m_v - m_simplex[1].m_v, -m_simplex[1].m_v);
	m_count = 2;

	while(1)
	{
		Support a;
		support(shape0, shape1, m_dir, a);

		if(a.m_v.dot(m_dir) < 0.0)
		{
			return false;
		}

		if(update(a))
		{
			return true;
		}
	}
}

//==============================================================================
// GjkEpa                                                                      =
//==============================================================================

//==============================================================================
Bool GjkEpa::intersect(const ConvexShape& shape0, const ConvexShape& shape1,
	ContactPoint& contact)
{
	// Do the intersection test
	if(!Gjk::intersect(shape0, shape1))
	{
		return false;
	}

	U faceIndex = 0;
	Face* face;

	// Set the array simplex
	ANKI_ASSERT(m_count == 4);
	m_simplexArr[0] = m_simplex[0];
	m_simplexArr[1] = m_simplex[1];
	m_simplexArr[2] = m_simplex[2];
	m_simplexArr[3] = m_simplex[3];

	// Set the first 4 faces
	face = &m_faces[0];
	face->m_idx[0] = 0;
	face->m_idx[1] = 1;
	face->m_idx[2] = 2;
	face->init();
	computeFace(*face);

	face = &m_faces[1];
	face->m_idx[0] = 1;
	face->m_idx[1] = 2;
	face->m_idx[2] = 3;
	face->init();
	computeFace(*face);

	face = &m_faces[2];
	face->m_idx[0] = 2;
	face->m_idx[1] = 3;
	face->m_idx[2] = 0;
	face->init();
	computeFace(*face);

	face = &m_faces[3];
	face->m_idx[0] = 3;
	face->m_idx[1] = 0;
	face->m_idx[2] = 1;
	face->init();
	computeFace(*face);

	m_faceCount = 4;

	std::cout << "-----------------------" << std::endl;

	U iterations = 0;
	while(1) 
	{
		// Find the closest to the origin face
		findClosestFace(faceIndex);

		face = &m_faces[faceIndex];
		const Vec4& normal = face->m_normal;
		F32 distance = face->m_dist;

		// Get new support
		Support p;
		support(shape0, shape1, normal, p);
		F32 d = p.m_v.dot(normal);

		// Search if the simplex is there
		U pIdx;
		for(pIdx = 0; pIdx < m_count; pIdx++)
		{
			if(p.m_v == m_simplexArr[pIdx].m_v)
			{
				std::cout << "p found " << std::endl;
				break;
			}
		}

		// Check new distance
		if(d - distance < 0.001 
			|| m_faceCount == m_faces.size() - 2
			|| m_count == m_simplexArr.size() - 1
			|| pIdx != m_count
			/*|| iterations == 3*/)
		{
			/*if(pIdx != m_count)
			{
				contact.m_normal = m_faces[prevFaceIndex].m_normal;
				contact.m_depth = m_faces[prevFaceIndex].m_dist;
			}
			else*/
			{
				contact.m_normal = normal;
				contact.m_depth = d;
			}
			break;
		} 
		else 
		{
			// Create 3 new faces by adding 'p'

			//if(pIdx == m_count)
			{
				// Add p
				m_simplexArr[m_count++] = p;
			}

			Array<U32, 3> idx = face->m_idx;

			// Canibalize existing face
			face->m_idx[0] = idx[0];
			face->m_idx[1] = idx[1];
			face->m_idx[2] = pIdx;
			face->m_normal[0] = -100.0;

			// Next face
			face = &m_faces[m_faceCount++];
			face->m_idx[0] = idx[1];
			face->m_idx[1] = idx[2];
			face->m_idx[2] = pIdx;
			face->m_normal[0] = -100.0;

			// Next face
			face = &m_faces[m_faceCount++];
			face->m_idx[0] = idx[2];
			face->m_idx[1] = idx[0];
			face->m_idx[2] = pIdx;
			face->m_normal[0] = -100.0;
		}

		++iterations;
	}

	return true;
}

//==============================================================================
void GjkEpa::findClosestFace(U& index)
{
	F32 minDistance = MAX_F32;

	// Iterate the faces
	for(U i = 0; i < m_faceCount; i++) 
	{
		Face& face = m_faces[i];
		
		// Check if we calculated the normal before
		if(face.m_normal[0] == -100.0)
		{
			computeFace(face);
		}

		// Check the distance against the other distances
		if(face.m_dist < minDistance) 
		{
			// Check if the origin lies within the face
			if(face.m_originInside == 2)
			{
				const Vec4& a = m_simplexArr[face.m_idx[0]].m_v;
				const Vec4& b = m_simplexArr[face.m_idx[1]].m_v;
				const Vec4& c = m_simplexArr[face.m_idx[2]].m_v;
				const Vec4& n = face.m_normal;

				face.m_originInside = 1;

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
					face.m_originInside = 0;
				}

				// Check the 2nd edge
				if(face.m_originInside == 1)
				{
					adjacentNormal = e1.cross(n);
					d = adjacentNormal.dot(b);

					if(d <= 0.0)
					{
						face.m_originInside = 0;
					}
				}

				// Check the 3rd edge
				if(face.m_originInside == 1)
				{
					adjacentNormal = e2.cross(n);
					d = adjacentNormal.dot(c);

					if(d <= 0.0)
					{
						face.m_originInside = 0;
					}
				}
			}

			if(face.m_originInside)
			{
				// We have a candidate
				index = i;
				minDistance = face.m_dist;
			}
			else
			{
				std::cout << "origin not in face" << std::endl;
			}
		}
	}
}

//==============================================================================
void GjkEpa::computeFace(Face& face)
{
	ANKI_ASSERT(face.m_normal[0] == -100.0);

	const Vec4& a = m_simplexArr[face.m_idx[0]].m_v;
	const Vec4& b = m_simplexArr[face.m_idx[1]].m_v;
	const Vec4& c = m_simplexArr[face.m_idx[2]].m_v;

	// Compute the face edges
	Vec4 e0 = b - a;
	Vec4 e1 = c - b;

	// Compute the face normal
	Vec4 n = e0.cross(e1);
	n.normalize();

	// Calculate the distance from the origin to the edge
	F32 d = n.dot(a);

	// Check the winding
	if(d < 0.0)
	{
		// It's not facing the origin so fix some stuff

		d = -d;
		n = -n;

		// Swap some indices
		auto idx = face.m_idx[0];
		face.m_idx[0] = face.m_idx[1];
		face.m_idx[1] = idx;
	}

	face.m_normal = n;
	face.m_dist = d;
}

//==============================================================================
#if 0
static Bool commonEdge(const Face& f0, const Face& f1, U& edge0, U& edge1)
{
	// For all edges of f0
	for(U i0 = 0; i0 < 3; i0++)
	{
		Array<U, 2> e0 = {f0.m_idx[i0], f0.m_idx[(i0 == 2) ? 0 : i0 + 1]};

		// For all edges of f1
		for(U i1 = 0; i1 < 3; i1++)
		{
			Array<U, 2> e1 = {f1.m_idx[i1], f1.m_idx[(i1 == 2) ? 0 : i1 + 1]};

			if((e0[0] == e1[0] && e0[1] == e1[1])
				|| (e0[0] == e1[1] && e0[1] == e1[0]))
			{
				edge0 = i0;
				edge1 = i1;
				return true;
			}
		}	
	}

	return false;
}
#endif

//==============================================================================
void GjkEpa::expandPolytope(Face& cface, const Vec4& point)
{
#if 0
	static const Array2d<U, 3, 2> edges = {
		{0,  1},
		{1,  2},
		{2,  0}};

	// Find other faces that share the same edge
	for(U f = 0; f < m_faceCount; f++)
	{
		Face& face = m_face[f];

		// Skip the same face
		if(&face == &cface)
		{
			continue;
		}

		for(U i = 0, i < 3; i++)
		{
			for(U j = 0, j < 3; j++)
			{
				if(cface.m_idx[edges[i][0]] == face.m_idx[edges[j][1]]
					&& cface.m_idx[edges[i][1]] == face.m_idx[edges[j][0]])
				{
					// Found common edge

					// Check if the point will create concave polytope
					if(face.m_normal.dot(point) > 0.0)
					{
						// Concave


					}
				}
			}
		}

		ANKI_ASSER(e0 < 3 && e1 < 3);

		// Check if the point will create concave polytope
		if(face.m_normal.dot(point) < 0.0)
		{
			// No concave
			continue;
		}

		// XXX
		Array<U32, 3> idx = face.m_idx;

		for(U e = 0; e < 3; e++)
		{
			if(e != e1)
			{
			}
		}

	}
#endif
}

} // end namespace anki

