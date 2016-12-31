// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/CollisionShape.h>
#include <anki/math/Vec3.h>

namespace anki
{

/// @addtogroup Collision
/// @{

/// Line segment. Line from a point to a point. P0 = origin and P1 = direction + origin
class LineSegment : public CollisionShape
{
public:
	using Base = CollisionShape;

	LineSegment()
		: Base(CollisionShapeType::LINE_SEG)
		, m_origin(0.0)
		, m_dir(0.0)
	{
	}

	LineSegment(const Vec4& origin, const Vec4& direction)
		: Base(CollisionShapeType::LINE_SEG)
		, m_origin(origin)
		, m_dir(direction)
	{
	}

	LineSegment(const LineSegment& b)
		: Base(CollisionShapeType::LINE_SEG)
	{
		operator=(b);
	}

	const Vec4& getOrigin() const
	{
		return m_origin;
	}

	Vec4& getOrigin()
	{
		return m_origin;
	}

	void setOrigin(const Vec4& x)
	{
		m_origin = x;
	}

	const Vec4& getDirection() const
	{
		return m_dir;
	}

	Vec4& getDirection()
	{
		return m_dir;
	}

	void setDirection(const Vec4& x)
	{
		m_dir = x;
	}

	LineSegment& operator=(const LineSegment& b)
	{
		Base::operator=(b);
		m_origin = b.m_origin;
		m_dir = b.m_dir;
		return *this;
	}

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v)
	{
		v.visit(*this);
	}
	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& b) const;

	LineSegment getTransformed(const Transform& transform) const;

private:
	Vec4 m_origin; ///< P0
	Vec4 m_dir; ///< P1 = origin+dir so dir = P1-origin
};
/// @}

} // end namespace anki
