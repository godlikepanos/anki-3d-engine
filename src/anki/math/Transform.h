// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Transformation
template<typename T>
class TTransform
{
public:
	/// @name Constructors
	/// @{
	TTransform()
	{
	}

	TTransform(const TTransform& b)
		: m_origin(b.m_origin)
		, m_rotation(b.m_rotation)
		, m_scale(b.m_scale)
	{
		checkW();
	}

	explicit TTransform(const TMat4<T>& m4)
	{
		m_rotation = TMat3x4<T>(m4.getRotationPart());
		m_origin = m4.getTranslationPart().xyz0();
		m_scale = 1.0;
		checkW();
	}

	TTransform(const TVec4<T>& origin, const TMat3x4<T>& rotation, const T scale)
		: m_origin(origin)
		, m_rotation(rotation)
		, m_scale(scale)
	{
		checkW();
	}

	explicit TTransform(const TVec4<T>& origin)
		: m_origin(origin)
		, m_rotation(Mat3x4::getIdentity())
		, m_scale(1.0f)
	{
		checkW();
	}

	explicit TTransform(const TMat3x4<T>& rotation)
		: m_origin(Vec4(0.0f))
		, m_rotation(rotation)
		, m_scale(1.0f)
	{
		checkW();
	}

	TTransform(const T scale)
		: m_origin(Vec4(0.0f))
		, m_rotation(Mat3x4::getIdentity())
		, m_scale(scale)
	{
		checkW();
	}
	/// @}

	/// @name Accessors
	/// @{
	const TVec4<T>& getOrigin() const
	{
		return m_origin;
	}

	TVec4<T>& getOrigin()
	{
		return m_origin;
	}

	void setOrigin(const TVec4<T>& o)
	{
		m_origin = o;
		checkW();
	}

	const TMat3x4<T>& getRotation() const
	{
		return m_rotation;
	}

	TMat3x4<T>& getRotation()
	{
		return m_rotation;
	}

	void setRotation(const TMat3x4<T>& r)
	{
		m_rotation = r;
	}

	T getScale() const
	{
		return m_scale;
	}

	T& getScale()
	{
		return m_scale;
	}

	void setScale(const T s)
	{
		m_scale = s;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TTransform& operator=(const TTransform& b)
	{
		m_origin = b.m_origin;
		m_rotation = b.m_rotation;
		m_scale = b.m_scale;
		checkW();
		return *this;
	}

	Bool operator==(const TTransform& b) const
	{
		return m_origin == b.m_origin && m_rotation == b.m_rotation && m_scale == b.m_scale;
	}

	Bool operator!=(const TTransform& b) const
	{
		return !operator==(b);
	}
	/// @}

	/// @name Other
	/// @{
	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TTransform& getIdentity()
	{
		static const TTransform ident(TVec4<T>(0.0), TMat3x4<T>::getIdentity(), 1.0);
		return ident;
	}

	/// @copybrief combineTTransformations
	TTransform combineTransformations(const TTransform& b) const
	{
		checkW();
		const TTransform& a = *this;
		TTransform out;

		out.m_origin = TVec4<T>(a.m_rotation * (b.m_origin * a.m_scale), 0.0) + a.m_origin;

		out.m_rotation = a.m_rotation.combineTransformations(b.m_rotation);
		out.m_scale = a.m_scale * b.m_scale;

		return out;
	}

	/// Get the inverse transformation. Its faster that inverting a Mat4
	TTransform getInverse() const
	{
		checkW();
		TTransform o;
		o.m_rotation = m_rotation;
		o.m_rotation.transposeRotationPart();
		o.m_scale = 1.0 / m_scale;
		o.m_origin = -(o.m_rotation * (o.m_scale * m_origin)).xyz0();
		return o;
	}

	void invert()
	{
		m_rotation.transposeRotationPart();
		m_scale = 1.0 / m_scale;
		m_origin = -(m_rotation * (m_scale * m_origin));
	}

	/// Transform a TVec3
	ANKI_USE_RESULT TVec3<T> transform(const TVec3<T>& b) const
	{
		checkW();
		return (m_rotation.getRotationPart() * (b * m_scale)) + m_origin.xyz();
	}

	/// Transform a TVec4. SIMD optimized
	ANKI_USE_RESULT TVec4<T> transform(const TVec4<T>& b) const
	{
		checkW();
		TVec4<T> out = TVec4<T>(m_rotation * (b * m_scale), 0.0) + m_origin;
		return out;
	}

	template<typename TAlloc>
	String toString(TAlloc alloc) const
	{
		ANKI_ASSERT(0 && "TODO");
		return String();
	}
	/// @}

private:
	/// @name Data
	/// @{
	TVec4<T> m_origin; ///< The rotation
	TMat3x4<T> m_rotation; ///< The translation
	T m_scale; ///< The uniform scaling
	/// @}

	void checkW() const
	{
		ANKI_ASSERT(m_origin.w() == 0.0);
	}
};

/// F32 transformation
using Transform = TTransform<F32>;
/// @}

} // end namespace anki
