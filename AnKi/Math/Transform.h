// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math/Common.h>

namespace anki {

// Transformation
template<typename T>
class TTransform
{
public:
	// Constructors //

	constexpr TTransform()
		: m_origin(T(0))
		, m_rotation(TMat<T, 3, 4>::getIdentity())
		, m_scale(T(1), T(1), T(1), T(0))
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
		m_scale = m4.extractScale().xyz0;

		const TVec<T, 3> s0 = m4.getColumn(0).xyz;
		const TVec<T, 3> s1 = m4.getColumn(1).xyz;
		const TVec<T, 3> s2 = m4.getColumn(2).xyz;
		m_rotation.setColumns(s0 / m_scale.x, s1 / m_scale.y, s2 / m_scale.z, TVec<T, 3>(T(0)));

		m_origin = m4.getTranslationPart().xyz0;
		check();
	}

	TTransform(const TVec<T, 4>& origin, const TMat<T, 3, 4>& rotation, const TVec<T, 4>& scale)
		: m_origin(origin)
		, m_rotation(rotation)
		, m_scale(scale)
	{
		check();
	}

	TTransform(const TVec<T, 3>& origin, const TMat<T, 3, 3>& rotation, const TVec<T, 3>& scale)
		: TTransform(origin.xyz0, TMat<T, 3, 4>(TVec<T, 3>(T(0)), rotation), scale.xyz0)
	{
		check();
	}

	// Accessors //
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
		m_origin = o.xyz0;
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

	[[nodiscard]] const TVec<T, 4>& getScale() const
	{
		return m_scale;
	}

	void setScale(const TVec<T, 4>& s)
	{
		m_scale = s;
		check();
	}

	void setScale(const TVec<T, 3>& s)
	{
		m_scale = s.xyz0;
		check();
	}

	// Operators with same type //
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

	// Other //

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	[[nodiscard]] static TTransform getIdentity()
	{
		return TTransform();
	}

	// See combineTTransformations
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

	// Get the inverse transformation. Its faster that inverting a Mat4
	[[nodiscard]] TTransform invert() const
	{
		TTransform o;
		o.m_rotation = m_rotation;
		o.m_rotation.transposeRotationPart();
		o.m_scale = TVec<T, 4>(T(1), T(1), T(1), T(0)) / m_scale.xyz1;
		o.m_origin = -(o.m_rotation * (o.m_scale * m_origin)).xyz0;
		o.check();
		return o;
	}

	// Transform a TVec3
	[[nodiscard]] TVec<T, 3> transform(const TVec<T, 3>& b) const
	{
		check();
		return (m_rotation.getRotationPart() * (b * m_scale.xyz)) + m_origin.xyz;
	}

	// Transform a TVec4. SIMD optimized
	[[nodiscard]] TVec<T, 4> transform(const TVec<T, 4>& b) const
	{
		check();
		TVec<T, 4> out = TVec<T, 4>(m_rotation * (b * m_scale), T(0)) + m_origin;
		return out;
	}

	template<U32 kVecComponentCount>
	[[nodiscard]] TTransform lookAt(const TVec<T, kVecComponentCount>& refPoint, const TVec<T, kVecComponentCount>& up) const
	{
		const TVec<T, 4> j = up.xyz0;
		const TVec<T, 4> vdir = (refPoint.xyz0 - m_origin).normalize();
		const TVec<T, 4> vup = (j - vdir * j.dot(vdir)).normalize();
		const TVec<T, 4> vside = vdir.cross(vup);
		TTransform out;
		out.m_origin = m_origin;
		out.m_scale = m_scale;
		out.m_rotation.setColumns(vside.xyz, vup.xyz, (-vdir).xyz);
		return out;
	}

	[[nodiscard]] String toString() const requires(std::is_floating_point<T>::value)
	{
		String str;
		String b = String("origin: ") + m_origin.toString();
		str += b;
		str += "\nrotation:\n";

		b = m_rotation.toString();
		str += b;
		str += "\n";

		b = String().sprintf("scale: %f %f %f", m_scale.x, m_scale.y, m_scale.z);
		str += b;

		return str;
	}

	[[nodiscard]] Bool hasUniformScale() const
	{
		return m_scale.x == m_scale.y && m_scale.x == m_scale.z;
	}

private:
	TVec<T, 4> m_origin; // The rotation
	TMat<T, 3, 4> m_rotation; // The translation
	TVec<T, 4> m_scale; // The scaling

	void check() const
	{
		ANKI_ASSERT(m_origin.w == T(0));
		using TT = TVec<T, 3>;
		[[maybe_unused]] TT t; // Shut up the compiler regarding TT
		ANKI_ASSERT(m_scale.w == T(0) && m_scale.xyz > T(0));
	}
};

using Transform = TTransform<F32>;
using DTransform = TTransform<F64>;

} // end namespace anki
