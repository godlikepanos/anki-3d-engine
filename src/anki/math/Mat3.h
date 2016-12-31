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

/// 3x3 Matrix. Mainly used for rotations. It includes many helpful member functions. Its row major. The columns are
/// the x,y,z axis @note TMat3*TMat3: 27 muls 18 adds
template<typename T>
class TMat3 : public TMat<T, 3, 3, Array<T, 9>, TMat3<T>, TVec3<T>, TVec3<T>>
{
	/// @name Friends
	/// @{
	template<typename Y>
	friend TMat3<Y> operator+(Y f, const TMat3<Y>& m3);
	template<typename Y>
	friend TMat3<Y> operator-(Y f, const TMat3<Y>& m3);
	template<typename Y>
	friend TMat3<Y> operator*(Y f, const TMat3<Y>& m3);
	template<typename Y>
	friend TMat3<Y> operator/(Y f, const TMat3<Y>& m3);
	/// @}

public:
	using Base = TMat<T, 3, 3, Array<T, 9>, TMat3<T>, TVec3<T>, TVec3<T>>;

	using Base::setRotationPart;

	/// @name Constructors
	/// @{
	TMat3()
		: Base()
	{
	}

	TMat3(const TMat3& b)
		: Base(b)
	{
	}

	TMat3(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22)
	{
		TMat3& m = *this;
		m(0, 0) = m00;
		m(0, 1) = m01;
		m(0, 2) = m02;
		m(1, 0) = m10;
		m(1, 1) = m11;
		m(1, 2) = m12;
		m(2, 0) = m20;
		m(2, 1) = m21;
		m(2, 2) = m22;
	}

	explicit TMat3(const T f)
		: Base(f)
	{
	}

	explicit TMat3(const T arr[])
		: Base(arr)
	{
	}

	explicit TMat3(const TQuat<T>& q)
	{
		setRotationPart(q);
	}

	explicit TMat3(const TEuler<T>& e)
	{
		setRotationPart(e);
	}

	explicit TMat3(const TAxisang<T>& axisang)
	{
		setRotationPart(axisang);
	}
	/// @}

	/// @name Other
	/// @{
	T getDet() const
	{
		const TMat3& m = *this;
		// For the accurate method see < r664
		return m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)) - m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0))
			+ m(0, 2) * (m(0, 1) * m(2, 1) - m(1, 1) * m(2, 0));
	}

	TMat3 getInverse() const
	{
		// Using Gramer's method Inv(A) = (1 / getDet(A)) * Adj(A)
		const TMat3& m = *this;
		TMat3 r;

		// compute determinant
		T cofactor0 = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1);
		T cofactor3 = m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2);
		T cofactor6 = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);
		T det = m(0, 0) * cofactor0 + m(1, 0) * cofactor3 + m(2, 0) * cofactor6;

		ANKI_ASSERT(!isZero<T>(det)); // Cannot invert det == 0

		// create adjoint matrix and multiply by 1/det to get inverse
		T invDet = 1.0 / det;
		r(0, 0) = invDet * cofactor0;
		r(0, 1) = invDet * cofactor3;
		r(0, 2) = invDet * cofactor6;

		r(1, 0) = invDet * (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2));
		r(1, 1) = invDet * (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0));
		r(1, 2) = invDet * (m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2));

		r(2, 0) = invDet * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));
		r(2, 1) = invDet * (m(0, 1) * m(2, 0) - m(0, 0) * m(2, 1));
		r(2, 2) = invDet * (m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0));

		return r;
	}

	void invert()
	{
		(*this) = getInverse();
	}

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TMat3& getZero()
	{
		static const TMat3 zero(0.0);
		return zero;
	}

	static const TMat3& getIdentity()
	{
		static const TMat3 ident(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
		return ident;
	}
	/// @}
};

/// @memberof TMat3
template<typename T>
TMat3<T> operator+(T f, const TMat3<T>& m3)
{
	return m3 + f;
}

/// @memberof TMat3
template<typename T>
TMat3<T> operator-(T f, const TMat3<T>& m3)
{
	TMat3<T> out;
	for(U i = 0; i < 9; i++)
	{
		out[i] = f - m3[i];
	}
	return out;
}

/// @memberof TMat3
template<typename T>
TMat3<T> operator*(T f, const TMat3<T>& m3)
{
	return m3 * f;
}

/// @memberof TMat3
template<typename T>
TMat3<T> operator/(T f, const TMat3<T>& m3)
{
	TMat3<T> out;
	for(U i = 0; i < 9; i++)
	{
		ANKI_ASSERT(m3[i] != T(0));
		out[i] = f / m3[i];
	}
	return out;
}

/// F32 3x3 matrix
using Mat3 = TMat3<F32>;
static_assert(sizeof(Mat3) == sizeof(F32) * 3 * 3, "Incorrect size");

/// F64 3x3 matrix
using DMat3 = TMat3<F64>;
static_assert(sizeof(DMat3) == sizeof(F64) * 3 * 3, "Incorrect size");
/// @}

} // end namespace anki
