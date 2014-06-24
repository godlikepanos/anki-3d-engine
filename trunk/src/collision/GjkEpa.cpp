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
Vec4 Gjk::support(const ConvexShape& shape0, const ConvexShape& shape1,
	const Vec4& dir)
{
	Vec4 s0 = shape0.computeSupport(dir);
	Vec4 s1 = shape1.computeSupport(-dir);
	return s0 - s1;
}

//==============================================================================
Bool Gjk::update(const Vec4& a)
{
	if(m_count == 2)
	{
		Vec4 ao = -a;
		 
		// Compute the vectors parallel to the edges we'll test
		Vec4 ab = m_b - a;
		Vec4 ac = m_c - a;
		 
		// Compute the triangle's normal
		Vec4 abc = ab.cross(ac);
		 
		// Compute a vector within the plane of the triangle,
		// Pointing away from the edge ab
		Vec4 abp = ab.cross(abc);
		 
		if(abp.dot(ao) > 0.0)
		{
			// The origin lies outside the triangle, near the edge ab
			m_c = m_b;
			m_b = a;
		 
			m_dir = crossAba(ab, ao);
		 
			return false;
		}

		// Perform a similar test for the edge ac
		Vec4 acp = abc.cross(ac);
		 
		if(acp.dot(ao) > 0.0)
		{
			m_b = a;
			m_dir = crossAba(ac, ao);

			return false;
		}
		 
		// If we get here, then the origin must be within the triangle, 
		// but we care whether it is above or below it, so test
		if(abc.dot(ao) > 0.0)
		{
			m_d = m_c;
			m_c = m_b;
			m_b = a;
		 
			m_dir = abc;
		}
		else
		{
			m_d = m_b;
			m_b = a;
		 
			m_dir = -abc;
		}
		 
		m_count = 3;
		 
		// Again, need a tetrahedron to enclose the origin
		return false;
	}
	else if(m_count == 3)
	{
		Vec4 ao = -a;
		 
		Vec4 ab = m_b - a;
		Vec4 ac = m_c - a;
		 
		Vec4 abc = ab.cross(ac);
		 
		Vec4 ad, acd, adb;
		 
		if(abc.dot(ao) > 0.0)
		{
			// In front of triangle ABC
			goto check_face;
		}
		 
		ad = m_d - a;
		acd = ac.cross(ad);
		 
		if(acd.dot(ao) > 0.0)
		{
			// In front of triangle ACD
			m_b = m_c;
			m_c = m_d;
		 
			ab = ac;
			ac = ad;
		 
			abc = acd;
			goto check_face;
		}
		 
		adb = ad.cross(ab);
		 
		if(adb.dot(ao) > 0.0)
		{
			// In front of triangle ADB
		 
			m_c = m_b;
			m_b = m_d;
		 
			ac = ab;
			ab = ad;
		 
			abc = adb;
			goto check_face;
		}
		 
		// Behind all three faces, the origin is in the tetrahedron, we're done
		m_a = a;
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
			m_c = m_b;
			m_b = a;
		 
			m_dir = crossAba(ab, ao);
		 
			m_count = 2;
			return false;
		}
		 
		Vec4 acp = abc.cross(ac);	
		 
		if(acp.dot(ao) > 0.0)
		{
			m_b = a;
		 
			m_dir = crossAba(ac, ao);
		 
			m_count = 2;
			return false;
		}
		 
		m_d = m_c;
		m_c = m_b;
		m_b = a;
		 
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
	m_c = support(shape0, shape1, m_dir);
	if(m_c.dot(m_dir) < 0.0)
	{
		return false;
	}
 
	m_dir = -m_c;
	m_b = support(shape0, shape1, m_dir);

	if(m_b.dot(m_dir) < 0.0)
	{
		return false;
	}

	m_dir = crossAba(m_c - m_b, -m_b);
	m_count = 2;

	while(1)
	{
		Vec4 a = support(shape0, shape1, m_dir);

		if(a.dot(m_dir) < 0.0)
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

	// Set the array simplex
	ANKI_ASSERT(m_count == 4);
	m_simplex[0] = m_a;
	m_simplex[1] = m_b;
	m_simplex[2] = m_c;
	m_simplex[3] = m_d;

	// Set the first 4 faces
	Face* face;

	face = &m_faces[0];
	face->m_idx[0] = 0;
	face->m_idx[1] = 1;
	face->m_idx[2] = 2;
	face->m_normal[0] = -100.0;

	face = &m_faces[1];
	face->m_idx[0] = 1;
	face->m_idx[1] = 2;
	face->m_idx[2] = 3;
	face->m_normal[0] = -100.0;

	face = &m_faces[2];
	face->m_idx[0] = 2;
	face->m_idx[1] = 3;
	face->m_idx[2] = 0;
	face->m_normal[0] = -100.0;

	face = &m_faces[3];
	face->m_idx[0] = 3;
	face->m_idx[1] = 0;
	face->m_idx[2] = 1;
	face->m_normal[0] = -100.0;

	m_faceCount = 4;

	while(1) 
	{
		U faceIndex;
		findClosestFace(faceIndex);
		
		face = &m_faces[faceIndex];
		Vec4& normal = face->m_normal;
		F32 distance = face->m_dist;

		// Get new support
		Vec4 p = support(shape0, shape1, normal);
		F32 d = p.dot(normal);

		// Check new distance
		if(d - distance < 0.00001)
		{
			contact.m_normal = normal;
			contact.m_depth = d;
			break;
		} 
		else 
		{
			// Create 3 new faces by adding 'p'

			Array<U32, 3> idx = face->m_idx;

			// Canibalize existing face
			face->m_idx[0] = idx[0];
			face->m_idx[1] = idx[1];
			face->m_idx[2] = m_count;
			face->m_normal[0] = -100.0;

			// Next face
			face = &m_faces[m_faceCount++];
			face->m_idx[0] = idx[1];
			face->m_idx[1] = idx[2];
			face->m_idx[2] = m_count;
			face->m_normal[0] = -100.0;

			// Next face
			face = &m_faces[m_faceCount++];
			face->m_idx[0] = idx[2];
			face->m_idx[1] = idx[0];
			face->m_idx[2] = m_count;
			face->m_normal[0] = -100.0;

			// Add p
			m_simplex[m_count++] = p;
		}
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
			// First time we encounter that face

			Vec4 a = m_simplex[face.m_idx[0]];
			Vec4 b = m_simplex[face.m_idx[1]];
			Vec4 c = m_simplex[face.m_idx[2]];

			// Compute the face edges
			Vec4 e0 = b - a;
			Vec4 e1 = c - b;

			// Compute the face normal
			Vec4 n = e0.cross(e1);
			n.normalize();

			// Calculate the distance from the origin to the edge
			F32 d = n.dot(a);

			// Check where the face is facing
			if(d < 0.0)
			{
				// It's not facing the origin so fix the normal and 'd'
				d = -d;
				n = -n;
			}

			face.m_normal = n;
			face.m_dist = d;
		}

		// Check the distance against the other distances
		if(face.m_dist < minDistance) 
		{
			// if this edge is closer then use it
			index = i;
			minDistance = face.m_dist;
		}
	}
}

} // end namespace anki

