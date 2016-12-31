// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>
#include <anki/math/Mat.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Template struct that gives the type of the TMat4 SIMD
template<typename T>
class TMat3x4Simd
{
public:
	using Type = Array<T, 12>;
};

#if ANKI_SIMD == ANKI_SIMD_SSE
// Specialize for F32
template<>
class TMat3x4Simd<F32>
{
public:
	using Type = Array<__m128, 3>;
};
#endif

/// 3x4 Matrix. Mainly used for transformations. It includes many helpful member functions. Its row major. The columns
/// are the x,y,z axis
template<typename T>
class alignas(16) TMat3x4 : public TMat<T, 3, 4, typename TMat3x4Simd<T>::Type, TMat3x4<T>, TVec4<T>, TVec3<T>>
{
public:
	using Base = TMat<T, 3, 4, typename TMat3x4Simd<T>::Type, TMat3x4<T>, TVec4<T>, TVec3<T>>;

	using Base::Base;

	/// @name Constructors
	/// @{
	TMat3x4()
		: Base()
	{
	}

	TMat3x4(const TMat3x4& b)
		: Base(b)
	{
	}

	TMat3x4(T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13, T m20, T m21, T m22, T m23)
	{
		TMat3x4& m = *this;
		m(0, 0) = m00;
		m(0, 1) = m01;
		m(0, 2) = m02;
		m(0, 3) = m03;
		m(1, 0) = m10;
		m(1, 1) = m11;
		m(1, 2) = m12;
		m(1, 3) = m13;
		m(2, 0) = m20;
		m(2, 1) = m21;
		m(2, 2) = m22;
		m(2, 3) = m23;
	}

	explicit TMat3x4(const T f)
		: Base(f)
	{
	}

	explicit TMat3x4(const TMat3<T>& m3)
	{
		TMat3x4& m = *this;
		m(0, 0) = m3(0, 0);
		m(0, 1) = m3(0, 1);
		m(0, 2) = m3(0, 2);
		m(0, 3) = static_cast<T>(0);
		m(1, 0) = m3(1, 0);
		m(1, 1) = m3(1, 1);
		m(1, 2) = m3(1, 2);
		m(1, 3) = static_cast<T>(0);
		m(2, 0) = m3(2, 0);
		m(2, 1) = m3(2, 1);
		m(2, 2) = m3(2, 2);
		m(2, 3) = static_cast<T>(0);
	}

	explicit TMat3x4(const TMat4<T>& m3)
	{
		TMat3x4& m = *this;
		m(0, 0) = m3(0, 0);
		m(0, 1) = m3(0, 1);
		m(0, 2) = m3(0, 2);
		m(0, 3) = m3(0, 3);
		m(1, 0) = m3(1, 0);
		m(1, 1) = m3(1, 1);
		m(1, 2) = m3(1, 2);
		m(1, 3) = m3(1, 3);
		m(2, 0) = m3(2, 0);
		m(2, 1) = m3(2, 1);
		m(2, 2) = m3(2, 2);
		m(2, 3) = m3(2, 3);
	}

	explicit TMat3x4(const TVec3<T>& v)
	{
		TMat3x4& m = *this;
		m(0, 0) = static_cast<T>(1);
		m(0, 1) = static_cast<T>(0);
		m(0, 2) = static_cast<T>(0);
		m(0, 3) = v.x();
		m(1, 0) = static_cast<T>(0);
		m(1, 1) = static_cast<T>(1);
		m(1, 2) = static_cast<T>(0);
		m(1, 3) = v.y();
		m(2, 0) = static_cast<T>(0);
		m(2, 1) = static_cast<T>(0);
		m(2, 2) = static_cast<T>(1);
		m(2, 3) = v.z();
	}

	explicit TMat3x4(const TQuat<T>& q)
	{
		Base::setRotationPart(TMat3<T>(q));
		Base::setTranslationPart(TVec3<T>(static_cast<T>(0)));
	}

	explicit TMat3x4(const TEuler<T>& b)
	{
		Base::setRotationPart(TMat3<T>(b));
		Base::setTranslationPart(TVec3<T>(static_cast<T>(0)));
	}

	explicit TMat3x4(const TAxisang<T>& b)
	{
		Base::setRotationPart(TAxisang<T>(b));
		Base::setTranslationPart(TVec3<T>(static_cast<T>(0)));
	}

	TMat3x4(const TVec3<T>& transl, const TMat3<T>& rot)
	{
		Base::setRotationPart(rot);
		Base::setTranslationPart(transl);
	}

	TMat3x4(const TVec3<T>& transl, const TMat3<T>& rot, const T scale)
	{
		if(isZero<T>(scale - static_cast<T>(1)))
		{
			Base::setRotationPart(rot);
		}
		else
		{
			setRotationPart(rot * scale);
		}

		Base::setTranslationPart(transl);
	}

	explicit TMat3x4(const TTransform<T>& t)
	{
		(*this) = TMat3x4(t.getOrigin(), t.getRotation(), t.getScale());
	}
	/// @}

	/// @name Other
	/// @{

	/// Create a new matrix that is equivalent to Mat4(this)*Mat4(b)
	TMat3x4 combineTransformations(const TMat3x4& b) const
	{
		const TMat3x4& a = *static_cast<const TMat3x4*>(this);
		TMat3x4 c;

		c(0, 0) = a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0) + a(0, 2) * b(2, 0);
		c(0, 1) = a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1) + a(0, 2) * b(2, 1);
		c(0, 2) = a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2) * b(2, 2);
		c(1, 0) = a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0) + a(1, 2) * b(2, 0);
		c(1, 1) = a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1) + a(1, 2) * b(2, 1);
		c(1, 2) = a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2) * b(2, 2);
		c(2, 0) = a(2, 0) * b(0, 0) + a(2, 1) * b(1, 0) + a(2, 2) * b(2, 0);
		c(2, 1) = a(2, 0) * b(0, 1) + a(2, 1) * b(1, 1) + a(2, 2) * b(2, 1);
		c(2, 2) = a(2, 0) * b(0, 2) + a(2, 1) * b(1, 2) + a(2, 2) * b(2, 2);

		c(0, 3) = a(0, 0) * b(0, 3) + a(0, 1) * b(1, 3) + a(0, 2) * b(2, 3) + a(0, 3);

		c(1, 3) = a(1, 0) * b(0, 3) + a(1, 1) * b(1, 3) + a(1, 2) * b(2, 3) + a(1, 3);

		c(2, 3) = a(2, 0) * b(0, 3) + a(2, 1) * b(1, 3) + a(2, 2) * b(2, 3) + a(2, 3);

		return c;
	}

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TMat3x4& getIdentity()
	{
		static const TMat3x4 ident(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		return ident;
	}
	/// @}
};

#if ANKI_SIMD == ANKI_SIMD_SSE

// Forward declare specializations

template<>
TMat3x4<F32>::Base::TMat(const TMat3x4<F32>::Base& b);

template<>
TMat3x4<F32>::Base::TMat(const F32 f);

template<>
inline TVec3<F32> TMat3x4<F32>::Base::operator*(const TVec4<F32>& b) const;

template<>
TMat3x4<F32> TMat3x4<F32>::combineTransformations(const TMat3x4<F32>& b) const;

#elif ANKI_SIMD == ANKI_SIMD_NEON

#error "TODO"

#endif

/// F32 4x4 matrix
using Mat3x4 = TMat3x4<F32>;
static_assert(sizeof(Mat3x4) == sizeof(F32) * 3 * 4, "Incorrect size");
/// @}

} // end namespace anki

#include <anki/math/Mat3x4.inl.h>
