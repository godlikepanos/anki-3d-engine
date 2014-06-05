// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_MATH_TRANSFORM_H
#define ANKI_MATH_TRANSFORM_H

#include "anki/math/Vec3.h"
#include "anki/math/Mat3.h"
#include "anki/math/CommonIncludes.h"

namespace anki {

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
	{}

	explicit TTransform(const Mat4& m4)
	{
		m_rotation = m4.getRotationPart();
		m_origin = m4.getTranslationPart().xyz();
		m_scale = 1.0;
	}

	explicit TTransform(const TVec3<T>& origin, const TMat3<T>& rotation,
		const T scale)
		: m_origin(origin), m_rotation(rotation), m_scale(scale)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const TVec3<T>& getOrigin() const
	{
		return m_origin;
	}

	TVec3<T>& getOrigin()
	{
		return m_origin;
	}

	void setOrigin(const TVec3<T>& o)
	{
		m_origin = o;
	}

	const TMat3<T>& getRotation() const
	{
		return m_rotation;
	}

	TMat3<T>& getRotation()
	{
		return m_rotation;
	}

	void setRotation(const TMat3<T>& r)
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
			TVec3<T>(0.0), TMat3<T>::getIdentity(), 1.0);
		return ident;
	}

	static void combineTransformations(
		const TVec3<T>& t0, const TMat3<T>& r0, const T s0,
		const TVec3<T>& t1, const TMat3<T>& r1, const T s1,
		TVec3<T>& tf, TMat3<T>& rf, T& sf)
	{
		tf = t1.getTransformed(t0, r0, s0);
		rf = r0 * r1;
		sf = s0 * s1;
	}

	static void combineTransformations(
		const TVec3<T>& t0, const TMat3<T>& r0,
		const TVec3<T>& t1, const TMat3<T>& r1,
		TVec3<T>& tf, TMat3<T>& rf)
	{
		tf = t1.getTransformed(t0, r0);
		rf = r0 * r1;
	}

	/// @copybrief combineTTransformations
	static TTransform combineTransformations(const TTransform& a,
		const TTransform& b)
	{
		TTransform out;

		out.m_origin = 
			b.m_origin.getTransformed(a.m_origin, a.m_rotation, a.m_scale);
		out.m_rotation = a.m_rotation * b.m_rotation;
		out.m_scale = a.m_scale * b.m_scale;

		return out;
	}

	/// Get the inverse transformation. Its faster that inverting a Mat4
	TTransform getInverse() const
	{
		TTransform o;
		o.m_rotation = m_rotation.getTransposed(); // Rotation
		o.m_scale = 1.0 / m_scale; // Apply scale
		o.m_origin = -((o.m_rotation * o.m_scale) * m_origin); // Translation
		return o;
	}

	void invert()
	{
		*this = getInverse();
	}

	void transform(const TTransform& b)
	{
		m_origin = b.m_origin.getTransformed(m_origin, m_rotation, m_scale);
		m_rotation = m_rotation * b.m_rotation;
		m_scale *= b.m_scale;
	}

	std::string toString() const
	{
		return m_origin.toString() + " " + m_rotation.toString() + " " + 
			std::to_string(m_scale);
	}
	/// @}

private:
	/// @name Data
	/// @{
	TVec3<T> m_origin; ///< The rotation
	TMat3<T> m_rotation; ///< The translation
	T m_scale; ///< The uniform scaling
	/// @}
};

/// F32 transformation
typedef TTransform<F32> Transform;

/// @}

} // end namespace anki

#endif
