// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Common.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Axis align bounding box collision shape.
class Aabb
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::AABB;

	/// Will not initialize any memory, nothing.
	Aabb()
	{
	}

	Aabb(const Vec4& min, const Vec4& max)
		: m_min(min)
		, m_max(max)
	{
		check();
	}

	Aabb(const Vec3& min, const Vec3& max)
		: Aabb(Vec4(min, 0.0f), Vec4(max, 0.0f))
	{
		check();
	}

	/// Set from point cloud.
	Aabb(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize)
	{
		setFromPointCloud(pointBuffer, pointCount, pointStride, buffSize);
	}

	/// Copy.
	Aabb(const Aabb& b)
	{
		operator=(b);
	}

	/// Copy.
	Aabb& operator=(const Aabb& b)
	{
		b.check();
		m_min = b.m_min;
		m_max = b.m_max;
		return *this;
	}

	const Vec4& getMin() const
	{
		check();
		return m_min;
	}

	void setMin(const Vec4& x)
	{
		ANKI_ASSERT(x.w() == 0.0f);
		m_min = x;
	}

	void setMin(const Vec3& x)
	{
		setMin(Vec4(x, 0.0f));
	}

	const Vec4& getMax() const
	{
		check();
		return m_max;
	}

	void setMax(const Vec4& x)
	{
		ANKI_ASSERT(x.w() == 0.0f);
		m_max = x;
	}

	void setMax(const Vec3& x)
	{
		setMax(Vec4(x, 0.0f));
	}

	/// Compute the GJK support.
	ANKI_USE_RESULT Vec4 computeSupport(const Vec4& dir) const;

	/// It uses a nice trick to avoid unwanted calculations
	Aabb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not very accurate
	Aabb getCompoundShape(const Aabb& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize);

private:
	Vec4 m_min
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	Vec4 m_max
#if ANKI_ENABLE_ASSERTS
		= Vec4(MIN_F32)
#endif
		;

	void check() const
	{
		ANKI_ASSERT(m_min.xyz() < m_max.xyz());
		ANKI_ASSERT(m_min.w() == 0.0f);
		ANKI_ASSERT(m_max.w() == 0.0f);
	}
};
/// @}

} // end namespace anki
