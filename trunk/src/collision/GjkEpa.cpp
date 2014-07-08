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
	ContactPoint& contact, StackAllocator<U8>& alloc)
{
	// Do the intersection test
	Gjk gjk;
	if(!gjk.intersect(shape0, shape1))
	{
		return false;
	}

	// Init polytope
	detail::Polytope* poly = alloc.newInstance<detail::Polytope>(
		alloc, m_maxSimplexSize, m_maxFaceCount);
	m_poly = poly;
	poly->init(gjk.m_simplex);

	std::cout << "-----------------------" << std::endl;

	U iterations = 0;
	while(1) 
	{
		// Find the closest to the origin face
		detail::Face& cface = poly->findClosestFace();

		const Vec4& normal = cface.normal(*poly);
		F32 distance = cface.distance(*poly);

		// Get new support
		Support p;
		Gjk::support(shape0, shape1, normal, p);
		F32 d = p.m_v.dot(normal);

		// Check new distance
		if(d - distance < 0.001 
			//|| m_faceCount == m_faces.size() - 2
			//|| m_count == m_simplexArr.size() - 1
			//|| pIdx != m_count
			|| iterations == 5)
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
			if(!poly->addSupportAndExpand(p, cface))
			{
				contact.m_normal = normal;
				contact.m_depth = d;
				break;
			}
		}

		++iterations;
	}

	return true;
}

} // end namespace anki

