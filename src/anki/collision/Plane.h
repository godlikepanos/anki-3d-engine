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

/// Plane collision shape
class Plane
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::PLANE;

	/// Will not initialize any memory, nothing.
	Plane()
	{
	}

	/// Copy constructor
	Plane(const Plane& b)
	{
		operator=(b);
	}

	/// Constructor
	Plane(const Vec4& normal, F32 offset)
		: m_normal(normal)
		, m_offset(offset)
	{
		check();
	}

	/// @see setFrom3Points
	Plane(const Vec4& p0, const Vec4& p1, const Vec4& p2)
	{
		setFrom3Points(p0, p1, p2);
	}

	/// @see setFromPlaneEquation
	Plane(F32 a, F32 b, F32 c, F32 d)
	{
		setFromPlaneEquation(a, b, c, d);
	}

	Plane& operator=(const Plane& b)
	{
		b.check();
		m_normal = b.m_normal;
		m_offset = b.m_offset;
		return *this;
	}

	const Vec4& getNormal() const
	{
		check();
		return m_normal;
	}

	void setNormal(const Vec4& x)
	{
		m_normal = x;
	}

	F32 getOffset() const
	{
		check();
		return m_offset;
	}

	void setOffset(const F32 x)
	{
		m_offset = x;
	}

	/// Set the plane from 3 points
	void setFrom3Points(const Vec4& p0, const Vec4& p1, const Vec4& p2);

	/// Set from plane equation is ax+by+cz+d
	void setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d);

	/// Return the transformed
	Plane getTransformed(const Transform& trf) const;

private:
	Vec4 m_normal
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	F32 m_offset
#if ANKI_ENABLE_ASSERTS
		= MAX_F32
#endif
		;

	void check() const
	{
		ANKI_ASSERT(m_normal.w() == 0.0f);
		ANKI_ASSERT(m_offset != MAX_F32);
	}
};
/// @}

} // end namespace anki
