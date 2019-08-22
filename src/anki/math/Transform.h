// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/Common.h>

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

	explicit TTransform(const TMat<T, 4, 4>& m4)
	{
		m_rotation = TMat<T, 3, 4>(m4.getRotationPart());
		m_origin = m4.getTranslationPart().xyz0();
		m_scale = 1.0;
		checkW();
	}

	TTransform(const TVec<T, 4>& origin, const TMat<T, 3, 4>& rotation, const T scale)
		: m_origin(origin)
		, m_rotation(rotation)
		, m_scale(scale)
	{
		checkW();
	}

	explicit TTransform(const TVec<T, 4>& origin)
		: m_origin(origin)
		, m_rotation(Mat3x4::getIdentity())
		, m_scale(1.0)
	{
		checkW();
	}

	explicit TTransform(const TMat<T, 3, 4>& rotation)
		: m_origin(Vec4(0.0))
		, m_rotation(rotation)
		, m_scale(1.0)
	{
		checkW();
	}

	TTransform(const T scale)
		: m_origin(Vec4(0.0))
		, m_rotation(Mat3x4::getIdentity())
		, m_scale(scale)
	{
		checkW();
	}
	/// @}

	/// @name Accessors
	/// @{
	const TVec<T, 4>& getOrigin() const
	{
		return m_origin;
	}

	TVec<T, 4>& getOrigin()
	{
		return m_origin;
	}

	void setOrigin(const TVec<T, 4>& o)
	{
		m_origin = o;
		checkW();
	}

	const TMat<T, 3, 4>& getRotation() const
	{
		return m_rotation;
	}

	TMat<T, 3, 4>& getRotation()
	{
		return m_rotation;
	}

	void setRotation(const TMat<T, 3, 4>& r)
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
		static const TTransform ident(TVec<T, 4>(0.0), TMat<T, 3, 4>::getIdentity(), 1.0);
		return ident;
	}

	/// @copybrief combineTTransformations
	TTransform combineTransformations(const TTransform& b) const
	{
		checkW();
		const TTransform& a = *this;
		TTransform out;

		out.m_origin = TVec<T, 4>(a.m_rotation * (b.m_origin * a.m_scale), 0.0) + a.m_origin;

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
		o.m_scale = T(1) / m_scale;
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
	TVec<T, 3> transform(const TVec<T, 3>& b) const
	{
		checkW();
		return (m_rotation.getRotationPart() * (b * m_scale)) + m_origin.xyz();
	}

	/// Transform a TVec4. SIMD optimized
	TVec<T, 4> transform(const TVec<T, 4>& b) const
	{
		checkW();
		TVec<T, 4> out = TVec<T, 4>(m_rotation * (b * m_scale), T(0)) + m_origin;
		return out;
	}
	/// @}

private:
	/// @name Data
	/// @{
	TVec<T, 4> m_origin; ///< The rotation
	TMat<T, 3, 4> m_rotation; ///< The translation
	T m_scale; ///< The uniform scaling
	/// @}

	void checkW() const
	{
		ANKI_ASSERT(m_origin.w() == 0.0);
	}
};

/// F32 transformation
using Transform = TTransform<F32>;

/// F64 transformation
using DTransform = TTransform<F64>;
/// @}

} // end namespace anki
