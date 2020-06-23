// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Common.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Line segment. Line from a point to a point. P0 = origin and P1 = direction + origin
class LineSegment
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::LINE_SEGMENT;

	/// Will not initialize any memory, nothing.
	LineSegment()
	{
	}

	LineSegment(const Vec4& origin, const Vec4& direction)
		: m_origin(origin)
		, m_dir(direction)
	{
		check();
	}

	LineSegment(const LineSegment& b)
	{
		operator=(b);
	}

	LineSegment& operator=(const LineSegment& b)
	{
		b.check();
		m_origin = b.m_origin;
		m_dir = b.m_dir;
		return *this;
	}

	const Vec4& getOrigin() const
	{
		check();
		return m_origin;
	}

	void setOrigin(const Vec4& origin)
	{
		m_origin = origin;
	}

	const Vec4& getDirection() const
	{
		check();
		return m_dir;
	}

	void setDirection(const Vec4& dir)
	{
		m_dir = dir;
	}

	LineSegment getTransformed(const Transform& trf) const
	{
		check();
		LineSegment out;
		out.m_origin = trf.transform(m_origin);
		out.m_dir = Vec4(trf.getRotation() * (m_dir * trf.getScale()), 0.0f);
		return out;
	}

private:
	Vec4 m_origin ///< P0
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	Vec4 m_dir ///< P1 = origin+dir so dir = P1-origin
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	void check() const
	{
		ANKI_ASSERT(m_origin.w() != 0.0f && m_dir.w() != 0.0f);
	}
};
/// @}

} // end namespace anki
