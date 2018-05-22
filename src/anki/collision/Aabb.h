// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/ConvexShape.h>
#include <anki/math/Vec3.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Axis align bounding box collision shape
class Aabb : public ConvexShape
{
public:
	using Base = ConvexShape;

	Aabb()
		: Base(CollisionShapeType::AABB)
	{
	}

	Aabb(const Vec4& min, const Vec4& max)
		: Base(CollisionShapeType::AABB)
		, m_min(min)
		, m_max(max)
	{
		ANKI_ASSERT(min.w() == 0.0f && max.w() == 0.0f);
		ANKI_ASSERT(m_min.xyz() < m_max.xyz());
	}

	Aabb(const Vec3& min, const Vec3& max)
		: Aabb(Vec4(min, 0.0f), Vec4(max, 0.0f))
	{
		ANKI_ASSERT(m_min.xyz() < m_max.xyz());
	}

	Aabb(const Aabb& b)
		: Base(CollisionShapeType::AABB)
	{
		operator=(b);
	}

	const Vec4& getMin() const
	{
		return m_min;
	}

	void setMin(const Vec4& x)
	{
		ANKI_ASSERT(x.w() == 0.0);
		m_min = x;
	}

	void setMin(const Vec3& x)
	{
		setMin(Vec4(x, 0.0f));
	}

	const Vec4& getMax() const
	{
		return m_max;
	}

	void setMax(const Vec4& x)
	{
		ANKI_ASSERT(x.w() == 0.0);
		m_max = x;
	}

	void setMax(const Vec3& x)
	{
		setMax(Vec4(x, 0.0f));
	}

	/// Copy.
	Aabb& operator=(const Aabb& b)
	{
		Base::operator=(b);
		m_min = b.m_min;
		m_max = b.m_max;
		return *this;
	}

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v) override
	{
		v.visit(*this);
	}
	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const override
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const override;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf) override
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& b) const override
	{
		b = *this;
	}

	/// Implements CompoundShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const override;

	/// It uses a nice trick to avoid unwanted calculations
	Aabb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not very accurate
	Aabb getCompoundShape(const Aabb& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(const void* buff, U count, PtrSize stride, PtrSize buffSize);

private:
	Vec4 m_min;
	Vec4 m_max;
};
/// @}

} // end namespace anki
