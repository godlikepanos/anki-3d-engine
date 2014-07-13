// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/collision/Tests.h"
#include "anki/collision/ConvexShape.h"
#include "anki/collision/Plane.h"
#include "anki/collision/CompoundShape.h"
#include "anki/collision/Aabb.h"
#include "anki/collision/Sphere.h"
#include "anki/collision/LineSegment.h"
#include "anki/collision/Obb.h"
#include "anki/collision/GjkEpa.h"

namespace anki {

// Forward
static U todo(CollisionTester& self,
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points);

static U gjkEpa(CollisionTester& self,
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points);

static U comp(CollisionTester& self,
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points);

//==============================================================================
static Array2d<
	CollisionTester::TestCallback, 
	static_cast<U>(CollisionShape::Type::COUNT),
	static_cast<U>(CollisionShape::Type::COUNT)> 
	cdMatrix = {{
	{todo, todo, todo, todo, todo, todo},
	{comp, comp, comp, comp, comp, comp},
	{todo, todo, todo, gjkEpa, todo, gjkEpa},
	{todo, todo, todo, gjkEpa, todo, gjkEpa},
	{todo, todo, todo, todo, todo, todo},
	{todo, todo, todo, gjkEpa, todo, gjkEpa}}};

//==============================================================================
static U todo(CollisionTester& self,
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points)
{
	ANKI_ASSERT(0 && "TODO");
	return 0;
}

//==============================================================================
static U gjkEpa(CollisionTester& self,
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points)
{
	GjkEpa epa(20, 20, 20);

	ContactPoint cp;
	Bool intersect = epa.intersect(
		static_cast<const ConvexShape&>(a), 
		static_cast<const ConvexShape&>(b),
		cp,
		self._getAllocator());

	if(!intersect)
	{
		return 0;
	}
	else
	{
		points.push_back(cp);
		return 1;
	}
}

//==============================================================================
static U comp(CollisionTester& self,
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points)
{
	class Vis: public CollisionShape::ConstVisitor
	{
	public:
		CollisionTester* m_self;
		CollisionTempVector<ContactPoint>* m_points;
		U32 m_out;
		const CollisionShape* m_b;

		void visit(const LineSegment& a)
		{
			m_out += m_self->test(a, *m_b, *m_points);
		}

		void visit(const Obb& a)
		{
			m_out += m_self->test(a, *m_b, *m_points);
		}

		void visit(const Plane& a)
		{
			m_out += m_self->test(a, *m_b, *m_points);
		}

		void visit(const Sphere& a)
		{
			m_out += m_self->test(a, *m_b, *m_points);
		}

		void visit(const Aabb& a)
		{
			m_out += m_self->test(a, *m_b, *m_points);
		}

		void visit(const CompoundShape& a)
		{
			m_out += m_self->test(a, *m_b, *m_points);
		}
	};

	Vis vis;
	vis.m_self = &self;
	vis.m_points = &points;
	vis.m_out = 0;
	vis.m_b = &b;
	a.accept(vis);

	return vis.m_out;
}

//==============================================================================
U CollisionTester::test(
	const CollisionShape& a, const CollisionShape& b, 
	CollisionTempVector<ContactPoint>& points)
{
	ANKI_ASSERT(&a != &b);

	U i = static_cast<U>(a.getType());
	U j = static_cast<U>(b.getType());

	return cdMatrix[i][j](*this, a, b, points);
}

} // end namespace anki

