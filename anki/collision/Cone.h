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

/// Cone collision shape.
class Cone
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::CONE;

	/// Will not initialize any memory, nothing.
	Cone()
	{
	}

	Cone(const Vec4& origin, const Vec4& dir, F32 length, F32 angle)
		: m_origin(origin)
		, m_dir(dir)
		, m_length(length)
		, m_angle(angle)
	{
		check();
	}

	/// Copy.
	Cone(const Cone& b)
	{
		operator=(b);
	}

	/// Copy.
	Cone& operator=(const Cone& b)
	{
		b.check();
		m_origin = b.m_origin;
		m_dir = b.m_dir;
		m_length = b.m_length;
		m_angle = b.m_angle;
		return *this;
	}

	void setOrigin(const Vec4& origin)
	{
		m_origin = origin;
	}

	const Vec4& getOrigin() const
	{
		check();
		return m_origin;
	}

	void setDirection(const Vec4& dir)
	{
		m_dir = dir;
	}

	const Vec4& getDirection() const
	{
		check();
		return m_dir;
	}

	void setLength(F32 len)
	{
		ANKI_ASSERT(len > EPSILON);
		m_length = len;
	}

	F32 getLength() const
	{
		check();
		return m_length;
	}

	void setAngle(F32 ang)
	{
		ANKI_ASSERT(ang > 0.0f && ang < 2.0f * PI);
		m_angle = ang;
	}

	F32 getAngle() const
	{
		check();
		return m_angle;
	}

	Cone getTransformed(const Transform& transform) const
	{
		Cone out;
		out.m_origin = transform.transform(m_origin);
		out.m_dir = (transform.getRotation() * m_dir.xyz0()).xyz0();
		out.m_length *= transform.getScale();
		return out;
	}

private:
	Vec4 m_origin
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	Vec4 m_dir
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	F32 m_length
#if ANKI_ENABLE_ASSERTS
		= -1.0f
#endif
		;

	F32 m_angle
#if ANKI_ENABLE_ASSERTS
		= -1.0f
#endif
		;

	void check() const
	{
		ANKI_ASSERT(m_origin.w() == 0.0f);
		ANKI_ASSERT(m_dir.w() == 0.0f);
		ANKI_ASSERT(m_length > 0.0f);
		ANKI_ASSERT(m_angle > 0.0f && m_angle < 2.0f * PI);
	}
};
/// @}

} // end namespace anki
