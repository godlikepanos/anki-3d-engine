// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math/Common.h>

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
	constexpr TTransform()
		: m_origin(T(0))
		, m_rotation(TMat<T, 3, 4>::getIdentity())
		, m_scale(T(1))
	{
	}

	TTransform(const TTransform& b)
		: m_origin(b.m_origin)
		, m_rotation(b.m_rotation)
		, m_scale(b.m_scale)
	{
		check();
	}

	explicit TTransform(const TMat<T, 4, 4>& m4)
	{
		const TVec<T, 3> s0 = m4.getColumn(0).xyz();
		const TVec<T, 3> s1 = m4.getColumn(1).xyz();
		const TVec<T, 3> s2 = m4.getColumn(2).xyz();

		const TVec<T, 3> scales(s0.getLength(), s1.getLength(), s2.getLength());
		[[maybe_unused]] const T E = T(0.001);
		ANKI_ASSERT(isZero(scales.x() - scales.y(), E) && isZero(scales.y() - scales.z(), E) && "Expecting uniform scale");

		m_rotation.setColumns(s0 / scales.x(), s1 / scales.x(), s2 / scales.x(), TVec<T, 3>(T(0)));
		m_origin = m4.getTranslationPart().xyz0();
		m_scale = scales.x();
		check();
	}

	TTransform(const TVec<T, 4>& origin, const TMat<T, 3, 4>& rotation, const T scale)
		: m_origin(origin)
		, m_rotation(rotation)
		, m_scale(scale)
	{
		check();
	}
	/// @}

	/// @name Accessors
	/// @{
	[[nodiscard]] const TVec<T, 4>& getOrigin() const
	{
		return m_origin;
	}

	void setOrigin(const TVec<T, 4>& o)
	{
		m_origin = o;
		check();
	}

	void setOrigin(const TVec<T, 3>& o)
	{
		m_origin = o.xyz0();
	}

	[[nodiscard]] const TMat<T, 3, 4>& getRotation() const
	{
		return m_rotation;
	}

	void setRotation(const TMat<T, 3, 4>& r)
	{
		m_rotation = r;
	}

	void setRotation(const TMat<T, 3, 3>& r)
	{
		m_rotation.setRotationPart(r);
		m_rotation.setTranslationPart(TVec<T, 3>(T(0)));
	}

	[[nodiscard]] T getScale() const
	{
		return m_scale;
	}

	void setScale(const T s)
	{
		m_scale = s;
		check();
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TTransform& operator=(const TTransform& b)
	{
		m_origin = b.m_origin;
		m_rotation = b.m_rotation;
		m_scale = b.m_scale;
		check();
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

	[[nodiscard]] static TTransform getIdentity()
	{
		return TTransform(TVec<T, 4>(T(0)), TMat<T, 3, 4>::getIdentity(), T(1));
	}

	/// @copybrief combineTTransformations
	[[nodiscard]] TTransform combineTransformations(const TTransform& b) const
	{
		check();
		const TTransform& a = *this;
		TTransform out;

		out.m_origin = TVec<T, 4>(a.m_rotation * (b.m_origin * a.m_scale), T(0)) + a.m_origin;

		out.m_rotation = a.m_rotation.combineTransformations(b.m_rotation);
		out.m_scale = a.m_scale * b.m_scale;

		return out;
	}

	/// Get the inverse transformation. Its faster that inverting a Mat4
	[[nodiscard]] TTransform getInverse() const
	{
		check();
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
		m_scale = T(1) / m_scale;
		m_origin = -(m_rotation * (m_scale * m_origin));
	}

	/// Transform a TVec3
	[[nodiscard]] TVec<T, 3> transform(const TVec<T, 3>& b) const
	{
		check();
		return (m_rotation.getRotationPart() * (b * m_scale)) + m_origin.xyz();
	}

	/// Transform a TVec4. SIMD optimized
	[[nodiscard]] TVec<T, 4> transform(const TVec<T, 4>& b) const
	{
		check();
		TVec<T, 4> out = TVec<T, 4>(m_rotation * (b * m_scale), T(0)) + m_origin;
		return out;
	}

	template<U kVecComponentCount>
	TTransform& lookAt(const TVec<T, kVecComponentCount>& refPoint, const TVec<T, kVecComponentCount>& up)
	{
		const TVec<T, 4> j = up.xyz0();
		const TVec<T, 4> vdir = (refPoint.xyz0() - m_origin).getNormalized();
		const TVec<T, 4> vup = (j - vdir * j.dot(vdir)).getNormalized();
		const TVec<T, 4> vside = vdir.cross(vup);
		m_rotation.setColumns(vside.xyz(), vup.xyz(), (-vdir).xyz());
		return *this;
	}

	[[nodiscard]] String toString() const requires(std::is_floating_point<T>::value)
	{
		String str;
		String b = m_origin.toString();
		str += b;
		str += "\n";

		b = m_rotation.toString();
		str += b;
		str += "\n";

		b = String().sprintf("%f", m_scale);
		str += b;

		return str;
	}
	/// @}

private:
	/// @name Data
	/// @{
	TVec<T, 4> m_origin; ///< The rotation
	TMat<T, 3, 4> m_rotation; ///< The translation
	T m_scale; ///< The uniform scaling
	/// @}

	void check() const
	{
		ANKI_ASSERT(m_origin.w() == T(0));
		ANKI_ASSERT(m_scale > T(0));
	}
};

/// F32 transformation
using Transform = TTransform<F32>;

/// F64 transformation
using DTransform = TTransform<F64>;
/// @}

} // end namespace anki
