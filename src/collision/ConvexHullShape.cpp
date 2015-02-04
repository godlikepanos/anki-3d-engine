// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/ConvexHullShape.h"
#include "anki/collision/Plane.h"

namespace anki {

//==============================================================================
ConvexHullShape::~ConvexHullShape()
{
	if(m_ownsTheStorage)
	{
		ANKI_ASSERT(m_pointsCount > 0);
		ANKI_ASSERT(m_points != nullptr);
		m_alloc.deallocate(static_cast<void*>(m_points), m_pointsCount);
	}

	m_trf = Transform::getIdentity();
	m_invTrf = Transform::getIdentity();
	m_points = nullptr;
	m_pointsCount = 0;
	m_alloc = CollisionAllocator<U8>();
	m_ownsTheStorage = false;
}

//==============================================================================
void ConvexHullShape::move(ConvexHullShape& b)
{
	this->~ConvexHullShape();
	Base::operator=(b);

	m_trf = b.m_trf;
	b.m_trf = Transform::getIdentity();

	m_invTrf = b.m_invTrf;
	b.m_invTrf = Transform::getIdentity();
	
	m_points = b.m_points;
	b.m_points = nullptr;

	m_pointsCount = b.m_pointsCount;
	b.m_pointsCount = 0;

	m_alloc = b.m_alloc;
	b.m_alloc = CollisionAllocator<U8>();

	m_ownsTheStorage = b.m_ownsTheStorage;
	b.m_ownsTheStorage = false;
}

//==============================================================================
Error ConvexHullShape::initStorage(CollisionAllocator<U8>& alloc, U pointCount)
{
	this->~ConvexHullShape();

	m_points = 
		reinterpret_cast<Vec4*>(alloc.allocate(pointCount * sizeof(Vec4)));
	if(m_points)
	{
		m_alloc = alloc;
		m_ownsTheStorage = true;
		m_pointsCount = pointCount;
	}
	else
	{
		return ErrorCode::OUT_OF_MEMORY;
	}

	return ErrorCode::NONE;
}

//==============================================================================
void ConvexHullShape::initStorage(void* buffer, U pointCount)
{
	ANKI_ASSERT(buffer);
	ANKI_ASSERT(pointCount > 0);
	
	this->~ConvexHullShape();
	m_points = static_cast<Vec4*>(buffer);
	m_pointsCount = pointCount;
	ANKI_ASSERT(m_ownsTheStorage == false);
}

//==============================================================================
F32 ConvexHullShape::testPlane(const Plane& p) const
{
	// Compute the invert transformation of the plane instead
	Plane pa = p.getTransformed(m_invTrf);

	F32 minDist = MAX_F32;

	const Vec4* point = m_points;
	const Vec4* end = m_points + m_pointsCount;
	for(; point != end; ++point)
	{
		F32 test = pa.test(*point);
		if(test == 0.0)
		{
			// Early exit
			return 0.0; 	
		}

		minDist = min(minDist, test);
	}

	return minDist;
}

//==============================================================================
void ConvexHullShape::transform(const Transform& trf)
{
	m_trf = m_trf.combineTransformations(trf);
	m_invTrf = m_trf.getInverse();
}

//==============================================================================
void ConvexHullShape::computeAabb(Aabb& aabb) const
{
}

} // end namespace anki
