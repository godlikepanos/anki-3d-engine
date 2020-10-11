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

/// Sphere collision shape.
class Sphere
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::SPHERE;

	/// Will not initialize any memory, nothing.
	Sphere()
	{
	}

	/// Copy constructor
	Sphere(const Sphere& b)
	{
		operator=(b);
	}

	/// Constructor
	Sphere(const Vec4& center, F32 radius)
		: m_center(center)
		, m_radius(radius)
	{
		check();
	}

	/// Set from point cloud.
	Sphere(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize)
	{
		setFromPointCloud(pointBuffer, pointCount, pointStride, buffSize);
	}

	Sphere& operator=(const Sphere& b)
	{
		b.check();
		m_center = b.m_center;
		m_radius = b.m_radius;
		return *this;
	}

	const Vec4& getCenter() const
	{
		check();
		return m_center;
	}

	void setCenter(const Vec4& x)
	{
		ANKI_ASSERT(x.w() == 0.0f);
		m_center = x;
	}

	void setCenter(const Vec3& x)
	{
		m_center = x.xyz0();
	}

	F32 getRadius() const
	{
		check();
		return m_radius;
	}

	void setRadius(const F32 x)
	{
		m_radius = x;
	}

	/// Calculate from a set of points
	void setFromPointCloud(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize);

	Sphere getTransformed(const Transform& transform) const
	{
		check();
		Sphere out;
		out.m_center = transform.transform(m_center);
		out.m_radius = m_radius * transform.getScale();
		return out;
	}

	/// Get the sphere that includes this sphere and the given. See a drawing in the docs dir for more info about the
	/// algorithm
	Sphere getCompoundShape(const Sphere& b) const;

	/// Compute the GJK support.
	Vec4 computeSupport(const Vec4& dir) const
	{
		return m_center + dir.getNormalized() * m_radius;
	}

private:
	Vec4 m_center
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	F32 m_radius
#if ANKI_ENABLE_ASSERTS
		= -1.0f
#endif
		;

	void check() const
	{
		ANKI_ASSERT(m_center.w() == 0.0f);
		ANKI_ASSERT(m_radius > 0.0f);
	}
};
/// @}

} // end namespace anki
