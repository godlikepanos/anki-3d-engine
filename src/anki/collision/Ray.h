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

/// Ray.
class Ray
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::RAY;

	/// Will not initialize any memory, nothing.
	Ray()
	{
		// Do nothing.
	}

	Ray(const Vec4& origin, const Vec4& dir)
		: m_origin(origin)
		, m_dir(dir)
	{
		check();
	}

	Ray(const Vec3& origin, const Vec3& dir)
		: m_origin(origin.xyz0())
		, m_dir(dir.xyz0())
	{
		check();
	}

	/// Copy.
	Ray(const Ray& other)
	{
		operator=(other);
	}

	Ray& operator=(const Ray& other)
	{
		other.check();
		m_origin = other.m_origin;
		m_dir = other.m_dir;
		return *this;
	}

	void setOrigin(const Vec4& origin)
	{
		m_origin = origin;
	}

	void setOrigin(const Vec3& origin)
	{
		m_origin = origin.xyz0();
	}

	ANKI_USE_RESULT const Vec4& getOrigin() const
	{
		check();
		return m_origin;
	}

	void setDirection(const Vec4& dir)
	{
		m_dir = dir;
	}

	void setDirection(const Vec3& dir)
	{
		m_dir = dir.xyz0();
	}

	ANKI_USE_RESULT const Vec4& getDirection() const
	{
		check();
		return m_dir;
	}

	ANKI_USE_RESULT Ray getTransformed(const Transform& trf) const
	{
		check();
		Ray out;
		out.m_origin = trf.transform(m_origin);
		out.m_dir = Vec4(trf.getRotation() * m_dir, 0.0f);
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

	void check() const
	{
		ANKI_ASSERT(m_origin.w() == 0.0f && m_dir.w() == 0.0f
					&& isZero(m_dir.getLengthSquared() - 1.0f, EPSILON * 100.0f));
	}
};
/// @}

} // end namespace anki
