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

	while(1) 
	{
		F32 distance;
		Vec4 normal;
		U index;
		findClosestEdge(distance, normal, index);	
		
		// Get new support
		Vec4 p = support(shape0, shape1, normal);
		F32 d = p.dot(normal);
		if(d - distance < 0.00001)
		{
			contact.m_normal = normal;
			contact.m_depth = d;
			break;
		} 
		else 
		{
			m_simplex[index] = p;
		}
	}
}

//==============================================================================
void GjkEpa::findClosestEdge(F32& distance, Vec4& normal, U& index)
{
	distance = MAX_F32;

	// Iterate the simplex
	for(U i = 0; i < m_count; i++) 
	{
		// Compute the next points index
		U j = i + 1;
		if(j == m_count)
		{
			j = 0;
		}

		Vec4 a = m_simplex[i];
		Vec4 b = m_simplex[j];
		
		// Create the edge vector
		Vec4 e = b - a;

		// Get the vector from the origin to a
		Vec4 oa = a;

		// Get the vector from the edge towards the origin
		Vec4 n = crossAba(e, oa);
		n.normalize();

		// Calculate the distance from the origin to the edge
		F32 d = n.dot(a);

		// Check the distance against the other distances
		if(d < distance) 
		{
			// if this edge is closer then use it
			distance = d;
			normal = n;
			index = j;
		}
	}
}

} // end namespace anki

