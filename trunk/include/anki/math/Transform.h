// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_MATH_TRANSFORM_H
#define ANKI_MATH_TRANSFORM_H

#include "anki/math/CommonIncludes.h"

namespace anki {

#define ANKI_CHECK_W() ANKI_ASSERT(m_origin.w() == 0.0)

/// @addtogroup math
/// @{

/// Transformation
template<typename T>
class TTransform
{
public:
	/// @name Constructors
	/// @{
	explicit TTransform()
	{}

	TTransform(const TTransform& b)
		: m_origin(b.m_origin), m_rotation(b.m_rotation), m_scale(b.m_scale)
	{
		ANKI_CHECK_W();
	}

	explicit TTransform(const Mat4& m4)
	{
		m_rotation = m4.getRotationPart();
		m_origin = m4.getTranslationPart();
		m_scale = 1.0;
		ANKI_CHECK_W();
	}

	explicit TTransform(const TVec4<T>& origin, const TMat3x4<T>& rotation,
		const T scale)
		: m_origin(origin), m_rotation(rotation), m_scale(scale)
	{
		ANKI_CHECK_W();
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
		ANKI_CHECK_W();
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
		ANKI_CHECK_W();
		return *this;
	}

	Bool operator==(const TTransform& b) const
	{
		return m_origin == b.m_origin && m_rotation == b.m_rotation 
			&& m_scale == b.m_scale;
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
		static const TTransform ident(
			TVec4<T>(0.0), TMat3x4<T>::getIdentity(), 1.0);
		return ident;
	}

	/// @copybrief combineTTransformations
	TTransform combineTransformations(const TTransform& b) const
	{
		ANKI_CHECK_W();
		const TTransform& a = *this;
		TTransform out;

		out.m_origin = 
			TVec4<T>(a.m_rotation * (b.m_origin * a.m_scale), 0.0) + a.m_origin;

		out.m_rotation = a.m_rotation.combineTransformations(b.m_rotation);
		out.m_scale = a.m_scale * b.m_scale;

		return out;
	}

	/// Get the inverse transformation. Its faster that inverting a Mat4
	TTransform getInverse() const
	{
		ANKI_CHECK_W();
		TTransform o;
		o.m_rotation = m_rotation;
		o.m_rotation.transposeRotationPart();
		o.m_scale = 1.0 / m_scale;
		o.m_origin = -((o.m_rotation * o.m_scale) * m_origin).xyz0();
		return o;
	}

	void invert()
	{
		m_rotation.transposeRotationPart();
		m_scale = 1.0 / m_scale;
		m_origin = -((m_rotation * m_scale) * m_origin);
	}

	/// Transform a TVec3
	TVec3<T> transform(const TVec3<T>& b) const
	{
		ANKI_CHECK_W();
		return (m_rotation.getRotationPart() * (b * m_scale)) + m_origin.xyz();
	}

	/// Transform a TVec4. SIMD optimized
	TVec4<T> transform(const TVec4<T>& b) const
	{
		ANKI_CHECK_W();
		TVec4<T> out = TVec4<T>(m_rotation * (b * m_scale), 0.0) + m_origin;
		return out;
	}

	template<typename TAlloc>
	StringBase<TAlloc> toString(
		typename StringBase<TAlloc>::Allocator& alloc) const
	{
		StringBase<TAlloc> out(alloc);
		out = StringBase<TAlloc>("t: ", alloc) + m_origin.toString(alloc) 
			+ CString("\n\nr: ") + m_rotation.toString(alloc) 
			+ CString("\ns: ") + StringBase<TAlloc>::toString(m_scale, alloc);
	}
	/// @}

private:
	/// @name Data
	/// @{
	TVec4<T> m_origin; ///< The rotation
	TMat3x4<T> m_rotation; ///< The translation
	T m_scale; ///< The uniform scaling
	/// @}
};

/// F32 transformation
typedef TTransform<F32> Transform;

/// @}

#undef ANKI_CHECK_W

} // end namespace anki

#endif
