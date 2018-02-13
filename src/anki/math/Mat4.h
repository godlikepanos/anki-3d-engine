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

/// Template struct that gives the type of the TVec4 SIMD
template<typename T>
class TMat4Simd
{
public:
	using Type = Array<T, 16>;
};

#if ANKI_SIMD == ANKI_SIMD_SSE
// Specialize for F32
template<>
class TMat4Simd<F32>
{
public:
	using Type = Array<__m128, 4>;
};
#endif

/// 4x4 Matrix. Used mainly for transformations but not necessarily. It's row major. SSE optimized
/// @note TMat4*TMat4: 64 muls 48 adds
template<typename T>
class alignas(16) TMat4 : public TMat<T, 4, 4, typename TMat4Simd<T>::Type, TMat4<T>, TVec4<T>, TVec4<T>>
{
	/// @name Friends
	/// @{
	template<typename Y>
	friend TMat4<Y> operator+(const Y f, const TMat4<Y>& m4);
	template<typename Y>
	friend TMat4<Y> operator-(const Y f, const TMat4<Y>& m4);
	template<typename Y>
	friend TMat4<Y> operator*(const Y f, const TMat4<Y>& m4);
	template<typename Y>
	friend TMat4<Y> operator/(const Y f, const TMat4<Y>& m4);
	/// @}

public:
	using Base = TMat<T, 4, 4, typename TMat4Simd<T>::Type, TMat4<T>, TVec4<T>, TVec4<T>>;

	using Base::getTranslationPart;
	using Base::setTranslationPart;
	using Base::getRotationPart;
	using Base::setRotationPart;

	/// @name Constructors
	/// @{
	TMat4()
		: Base()
	{
	}

	TMat4(const TMat4& b)
		: Base(b)
	{
	}

	TMat4(
		T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13, T m20, T m21, T m22, T m23, T m30, T m31, T m32, T m33)
	{
		TMat4& m = *this;
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
		m(3, 0) = m30;
		m(3, 1) = m31;
		m(3, 2) = m32;
		m(3, 3) = m33;
	}

	explicit TMat4(const T f)
		: Base(f)
	{
	}

	explicit TMat4(const TMat3<T>& m3)
	{
		TMat4& m = *this;
		m(0, 0) = m3(0, 0);
		m(0, 1) = m3(0, 1);
		m(0, 2) = m3(0, 2);
		m(0, 3) = 0.0;
		m(1, 0) = m3(1, 0);
		m(1, 1) = m3(1, 1);
		m(1, 2) = m3(1, 2);
		m(1, 3) = 0.0;
		m(2, 0) = m3(2, 0);
		m(2, 1) = m3(2, 1);
		m(2, 2) = m3(2, 2);
		m(2, 3) = 0.0;
		m(3, 0) = 0.0;
		m(3, 1) = 0.0;
		m(3, 2) = 0.0;
		m(3, 3) = 1.0;
	}

	explicit TMat4(const TVec3<T>& v)
	{
		TMat4& m = *this;
		m(0, 0) = 1.0;
		m(0, 1) = 0.0;
		m(0, 2) = 0.0;
		m(0, 3) = v.x();
		m(1, 0) = 0.0;
		m(1, 1) = 1.0;
		m(1, 2) = 0.0;
		m(1, 3) = v.y();
		m(2, 0) = 0.0;
		m(2, 1) = 0.0;
		m(2, 2) = 1.0;
		m(2, 3) = v.z();
		m(3, 0) = 0.0;
		m(3, 1) = 0.0;
		m(3, 2) = 0.0;
		m(3, 3) = 1.0;
	}

	explicit TMat4(const TVec4<T>& v)
	{
		TMat4& m = *this;
		m(0, 0) = 1.0;
		m(0, 1) = 0.0;
		m(0, 2) = 0.0;
		m(0, 3) = v.x();
		m(1, 0) = 0.0;
		m(1, 1) = 1.0;
		m(1, 2) = 0.0;
		m(1, 3) = v.y();
		m(2, 0) = 0.0;
		m(2, 1) = 0.0;
		m(2, 2) = 1.0;
		m(2, 3) = v.z();
		m(3, 0) = 0.0;
		m(3, 1) = 0.0;
		m(3, 2) = 0.0;
		m(3, 3) = v.w();
	}

	TMat4(const TVec4<T>& transl, const TMat3<T>& rot)
	{
		setRotationPart(rot);
		setTranslationPart(transl);
		TMat4& m = *this;
		m(3, 0) = m(3, 1) = m(3, 2) = 0.0;
	}

	TMat4(const TVec4<T>& transl, const TMat3<T>& rot, const T scale)
	{
		if(isZero<T>(scale - 1.0))
		{
			setRotationPart(rot);
		}
		else
		{
			setRotationPart(rot * scale);
		}

		setTranslationPart(transl);

		TMat4& m = *this;
		m(3, 0) = m(3, 1) = m(3, 2) = 0.0;
	}

	explicit TMat4(const TTransform<T>& t)
		: TMat4(TVec4<T>(t.getOrigin().xyz(), 1.0), t.getRotation().getRotationPart(), t.getScale())
	{
	}
	/// @}

	/// @name Other
	/// @{
	T getDet() const
	{
		const TMat4& t = *this;
		return t(0, 3) * t(1, 2) * t(2, 1) * t(3, 0) - t(0, 2) * t(1, 3) * t(2, 1) * t(3, 0)
			   - t(0, 3) * t(1, 1) * t(2, 2) * t(3, 0) + t(0, 1) * t(1, 3) * t(2, 2) * t(3, 0)
			   + t(0, 2) * t(1, 1) * t(2, 3) * t(3, 0) - t(0, 1) * t(1, 2) * t(2, 3) * t(3, 0)
			   - t(0, 3) * t(1, 2) * t(2, 0) * t(3, 1) + t(0, 2) * t(1, 3) * t(2, 0) * t(3, 1)
			   + t(0, 3) * t(1, 0) * t(2, 2) * t(3, 1) - t(0, 0) * t(1, 3) * t(2, 2) * t(3, 1)
			   - t(0, 2) * t(1, 0) * t(2, 3) * t(3, 1) + t(0, 0) * t(1, 2) * t(2, 3) * t(3, 1)
			   + t(0, 3) * t(1, 1) * t(2, 0) * t(3, 2) - t(0, 1) * t(1, 3) * t(2, 0) * t(3, 2)
			   - t(0, 3) * t(1, 0) * t(2, 1) * t(3, 2) + t(0, 0) * t(1, 3) * t(2, 1) * t(3, 2)
			   + t(0, 1) * t(1, 0) * t(2, 3) * t(3, 2) - t(0, 0) * t(1, 1) * t(2, 3) * t(3, 2)
			   - t(0, 2) * t(1, 1) * t(2, 0) * t(3, 3) + t(0, 1) * t(1, 2) * t(2, 0) * t(3, 3)
			   + t(0, 2) * t(1, 0) * t(2, 1) * t(3, 3) - t(0, 0) * t(1, 2) * t(2, 1) * t(3, 3)
			   - t(0, 1) * t(1, 0) * t(2, 2) * t(3, 3) + t(0, 0) * t(1, 1) * t(2, 2) * t(3, 3);
	}

	/// Invert using Cramer's rule
	TMat4 getInverse() const
	{
		Array<T, 12> tmp;
		const TMat4& in = (*this);
		TMat4 m4;

		tmp[0] = in(2, 2) * in(3, 3);
		tmp[1] = in(3, 2) * in(2, 3);
		tmp[2] = in(1, 2) * in(3, 3);
		tmp[3] = in(3, 2) * in(1, 3);
		tmp[4] = in(1, 2) * in(2, 3);
		tmp[5] = in(2, 2) * in(1, 3);
		tmp[6] = in(0, 2) * in(3, 3);
		tmp[7] = in(3, 2) * in(0, 3);
		tmp[8] = in(0, 2) * in(2, 3);
		tmp[9] = in(2, 2) * in(0, 3);
		tmp[10] = in(0, 2) * in(1, 3);
		tmp[11] = in(1, 2) * in(0, 3);

		m4(0, 0) = tmp[0] * in(1, 1) + tmp[3] * in(2, 1) + tmp[4] * in(3, 1);
		m4(0, 0) -= tmp[1] * in(1, 1) + tmp[2] * in(2, 1) + tmp[5] * in(3, 1);
		m4(0, 1) = tmp[1] * in(0, 1) + tmp[6] * in(2, 1) + tmp[9] * in(3, 1);
		m4(0, 1) -= tmp[0] * in(0, 1) + tmp[7] * in(2, 1) + tmp[8] * in(3, 1);
		m4(0, 2) = tmp[2] * in(0, 1) + tmp[7] * in(1, 1) + tmp[10] * in(3, 1);
		m4(0, 2) -= tmp[3] * in(0, 1) + tmp[6] * in(1, 1) + tmp[11] * in(3, 1);
		m4(0, 3) = tmp[5] * in(0, 1) + tmp[8] * in(1, 1) + tmp[11] * in(2, 1);
		m4(0, 3) -= tmp[4] * in(0, 1) + tmp[9] * in(1, 1) + tmp[10] * in(2, 1);
		m4(1, 0) = tmp[1] * in(1, 0) + tmp[2] * in(2, 0) + tmp[5] * in(3, 0);
		m4(1, 0) -= tmp[0] * in(1, 0) + tmp[3] * in(2, 0) + tmp[4] * in(3, 0);
		m4(1, 1) = tmp[0] * in(0, 0) + tmp[7] * in(2, 0) + tmp[8] * in(3, 0);
		m4(1, 1) -= tmp[1] * in(0, 0) + tmp[6] * in(2, 0) + tmp[9] * in(3, 0);
		m4(1, 2) = tmp[3] * in(0, 0) + tmp[6] * in(1, 0) + tmp[11] * in(3, 0);
		m4(1, 2) -= tmp[2] * in(0, 0) + tmp[7] * in(1, 0) + tmp[10] * in(3, 0);
		m4(1, 3) = tmp[4] * in(0, 0) + tmp[9] * in(1, 0) + tmp[10] * in(2, 0);
		m4(1, 3) -= tmp[5] * in(0, 0) + tmp[8] * in(1, 0) + tmp[11] * in(2, 0);

		tmp[0] = in(2, 0) * in(3, 1);
		tmp[1] = in(3, 0) * in(2, 1);
		tmp[2] = in(1, 0) * in(3, 1);
		tmp[3] = in(3, 0) * in(1, 1);
		tmp[4] = in(1, 0) * in(2, 1);
		tmp[5] = in(2, 0) * in(1, 1);
		tmp[6] = in(0, 0) * in(3, 1);
		tmp[7] = in(3, 0) * in(0, 1);
		tmp[8] = in(0, 0) * in(2, 1);
		tmp[9] = in(2, 0) * in(0, 1);
		tmp[10] = in(0, 0) * in(1, 1);
		tmp[11] = in(1, 0) * in(0, 1);

		m4(2, 0) = tmp[0] * in(1, 3) + tmp[3] * in(2, 3) + tmp[4] * in(3, 3);
		m4(2, 0) -= tmp[1] * in(1, 3) + tmp[2] * in(2, 3) + tmp[5] * in(3, 3);
		m4(2, 1) = tmp[1] * in(0, 3) + tmp[6] * in(2, 3) + tmp[9] * in(3, 3);
		m4(2, 1) -= tmp[0] * in(0, 3) + tmp[7] * in(2, 3) + tmp[8] * in(3, 3);
		m4(2, 2) = tmp[2] * in(0, 3) + tmp[7] * in(1, 3) + tmp[10] * in(3, 3);
		m4(2, 2) -= tmp[3] * in(0, 3) + tmp[6] * in(1, 3) + tmp[11] * in(3, 3);
		m4(2, 3) = tmp[5] * in(0, 3) + tmp[8] * in(1, 3) + tmp[11] * in(2, 3);
		m4(2, 3) -= tmp[4] * in(0, 3) + tmp[9] * in(1, 3) + tmp[10] * in(2, 3);
		m4(3, 0) = tmp[2] * in(2, 2) + tmp[5] * in(3, 2) + tmp[1] * in(1, 2);
		m4(3, 0) -= tmp[4] * in(3, 2) + tmp[0] * in(1, 2) + tmp[3] * in(2, 2);
		m4(3, 1) = tmp[8] * in(3, 2) + tmp[0] * in(0, 2) + tmp[7] * in(2, 2);
		m4(3, 1) -= tmp[6] * in(2, 2) + tmp[9] * in(3, 2) + tmp[1] * in(0, 2);
		m4(3, 2) = tmp[6] * in(1, 2) + tmp[11] * in(3, 2) + tmp[3] * in(0, 2);
		m4(3, 2) -= tmp[10] * in(3, 2) + tmp[2] * in(0, 2) + tmp[7] * in(1, 2);
		m4(3, 3) = tmp[10] * in(2, 2) + tmp[4] * in(0, 2) + tmp[9] * in(1, 2);
		m4(3, 3) -= tmp[8] * in(1, 2) + tmp[11] * in(2, 2) + tmp[5] * in(0, 2);

		T det = in(0, 0) * m4(0, 0) + in(1, 0) * m4(0, 1) + in(2, 0) * m4(0, 2) + in(3, 0) * m4(0, 3);

		ANKI_ASSERT(!isZero<T>(det)); // Cannot invert, det == 0
		det = 1.0 / det;
		m4 *= det;
		return m4;
	}

	/// See getInverse
	void invert()
	{
		(*this) = getInverse();
	}

	/// If we suppose this matrix represents a transformation, return the inverted transformation
	TMat4 getInverseTransformation() const
	{
		TMat3<T> invertedRot = getRotationPart().getTransposed();
		TVec3<T> invertedTsl = getTranslationPart().xyz();
		invertedTsl = -(invertedRot * invertedTsl);
		return TMat4(invertedTsl.xyz0(), invertedRot);
	}

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TMat4& getIdentity()
	{
		static const TMat4 ident(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
		return ident;
	}

	/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching the 4rth row and allot faster
	static TMat4 combineTransformations(const TMat4& m0, const TMat4& m1)
	{
		// See the clean code in < r664

		// one of the 2 mat4 doesnt represent transformation
		ANKI_ASSERT(isZero<T>(m0(3, 0) + m0(3, 1) + m0(3, 2) + m0(3, 3) - 1.0)
					&& isZero<T>(m1(3, 0) + m1(3, 1) + m1(3, 2) + m1(3, 3) - 1.0));

		TMat4 m4;

		m4(0, 0) = m0(0, 0) * m1(0, 0) + m0(0, 1) * m1(1, 0) + m0(0, 2) * m1(2, 0);
		m4(0, 1) = m0(0, 0) * m1(0, 1) + m0(0, 1) * m1(1, 1) + m0(0, 2) * m1(2, 1);
		m4(0, 2) = m0(0, 0) * m1(0, 2) + m0(0, 1) * m1(1, 2) + m0(0, 2) * m1(2, 2);
		m4(1, 0) = m0(1, 0) * m1(0, 0) + m0(1, 1) * m1(1, 0) + m0(1, 2) * m1(2, 0);
		m4(1, 1) = m0(1, 0) * m1(0, 1) + m0(1, 1) * m1(1, 1) + m0(1, 2) * m1(2, 1);
		m4(1, 2) = m0(1, 0) * m1(0, 2) + m0(1, 1) * m1(1, 2) + m0(1, 2) * m1(2, 2);
		m4(2, 0) = m0(2, 0) * m1(0, 0) + m0(2, 1) * m1(1, 0) + m0(2, 2) * m1(2, 0);
		m4(2, 1) = m0(2, 0) * m1(0, 1) + m0(2, 1) * m1(1, 1) + m0(2, 2) * m1(2, 1);
		m4(2, 2) = m0(2, 0) * m1(0, 2) + m0(2, 1) * m1(1, 2) + m0(2, 2) * m1(2, 2);

		m4(0, 3) = m0(0, 0) * m1(0, 3) + m0(0, 1) * m1(1, 3) + m0(0, 2) * m1(2, 3) + m0(0, 3);

		m4(1, 3) = m0(1, 0) * m1(0, 3) + m0(1, 1) * m1(1, 3) + m0(1, 2) * m1(2, 3) + m0(1, 3);

		m4(2, 3) = m0(2, 0) * m1(0, 3) + m0(2, 1) * m1(1, 3) + m0(2, 2) * m1(2, 3) + m0(2, 3);

		m4(3, 0) = m4(3, 1) = m4(3, 2) = 0.0;
		m4(3, 3) = 1.0;

		return m4;
	}

	/// @note 9 muls, 9 adds
	TVec3<T> transform(const TVec3<T>& v) const
	{
		const TMat4& m = *this;

		return TVec3<T>(m(0, 0) * v.x() + m(0, 1) * v.y() + m(0, 2) * v.z() + m(0, 3),
			m(1, 0) * v.x() + m(1, 1) * v.y() + m(1, 2) * v.z() + m(1, 3),
			m(2, 0) * v.x() + m(2, 1) * v.y() + m(2, 2) * v.z() + m(2, 3));
	}

	/// Calculate a perspective projection matrix. The z is mapped in [0, 1] range just like DX and Vulkan.
	static TMat4 calculatePerspectiveProjectionMatrix(T fovX, T fovY, T near, T far)
	{
		ANKI_ASSERT(fovX > T(0) && fovY > T(0) && near > T(0) && far > T(0));
		T g = near - far;

		T f = T(1) / tan(fovY / T(2)); // f = cot(fovY/2)

		TMat4 proj;
		proj(0, 0) = f * (fovY / fovX); // = f/aspectRatio;
		proj(0, 1) = T(0);
		proj(0, 2) = T(0);
		proj(0, 3) = T(0);
		proj(1, 0) = T(0);
		proj(1, 1) = f;
		proj(1, 2) = T(0);
		proj(1, 3) = T(0);
		proj(2, 0) = T(0);
		proj(2, 1) = T(0);
		proj(2, 2) = far / g;
		proj(2, 3) = (far * near) / g;
		proj(3, 0) = T(0);
		proj(3, 1) = T(0);
		proj(3, 2) = T(-1);
		proj(3, 3) = T(0);

		return proj;
	}

	/// Calculate an orthographic projection matrix. The z is mapped in [0, 1] range just like DX and Vulkan.
	static TMat4 calculateOrthographicProjectionMatrix(T right, T left, T top, T bottom, T near, T far)
	{
		ANKI_ASSERT(right != T(0) && left != T(0) && top != T(0) && bottom != T(0) && near != T(0) && far != T(0));
		T difx = right - left;
		T dify = top - bottom;
		T difz = far - near;
		T tx = -(right + left) / difx;
		T ty = -(top + bottom) / dify;
		T tz = -near / difz;
		TMat4 m;

		m(0, 0) = T(2) / difx;
		m(0, 1) = T(0);
		m(0, 2) = T(0);
		m(0, 3) = tx;
		m(1, 0) = T(0);
		m(1, 1) = T(2) / dify;
		m(1, 2) = T(0);
		m(1, 3) = ty;
		m(2, 0) = T(0);
		m(2, 1) = T(0);
		m(2, 2) = T(-1) / difz;
		m(2, 3) = tz;
		m(3, 0) = T(0);
		m(3, 1) = T(0);
		m(3, 2) = T(0);
		m(3, 3) = T(1);

		return m;
	}

	/// Given the parameters that construct a projection matrix extract 4 values that can be used to unproject a point
	/// from NDC to view space.
	/// @code
	/// Vec4 unprojParams = calculatePerspectiveUnprojectionParams(...);
	/// F32 z = unprojParams.z() / (unprojParams.w() + depth);
	/// Vec2 xy = ndc.xy() * unprojParams.xy() * z;
	/// Vec3 posViewSpace(xy, z);
	/// @endcode
	static TVec4<T> calculatePerspectiveUnprojectionParams(T fovX, T fovY, T near, T far)
	{
		TVec4<T> out;
		T g = near - far;
		T f = T(1) / tan(fovY / T(2)); // f = cot(fovY/2)

		T m00 = f * (fovY / fovX);
		T m11 = f;
		T m22 = far / g;
		T m23 = (far * near) / g;

		// First, clip = (m * Pv) where Pv is the view space position.
		// ndc.z = clip.z / clip.w = (m22 * Pv.z + m23) / -Pv.z. Note that ndc.z == depth in zero_to_one projection.
		// Solving that for Pv.z we get
		// Pv.z = A / (depth + B)
		// where A = -m23 and B = m22
		// so we save the A and B in the projection params vector
		out.z() = -m23;
		out.w() = m22;

		// Using the same logic the Pv.x = x' * w / m00
		// so Pv.x = x' * Pv.z * (-1 / m00)
		out.x() = -T(1.0) / m00;

		// Same for y
		out.y() = -T(1.0) / m11;

		return out;
	}

	/// Assuming this is a projection matrix extract the unprojection parameters. See
	/// calculatePerspectiveUnprojectionParams for more info.
	TVec4<T> extractPerspectiveUnprojectionParams() const
	{
		TVec4<T> out;
		const TMat4& m = *this;
		out.z() = -m(2, 3);
		out.w() = m(2, 2);
		out.x() = -T(1.0) / m(0, 0);
		out.y() = -T(1.0) / m(1, 1);
		return out;
	}
	/// @}
};

#if ANKI_SIMD == ANKI_SIMD_SSE

// Forward declare specializations

using TMat4F32Base = TMat4<F32>::Base;

template<>
TMat4F32Base::TMat(const TMat4F32Base& b);

template<>
TMat4F32Base::TMat(const F32 f);

template<>
TMat4<F32>& TMat4F32Base::operator=(const TMat4<F32>& b);

template<>
TMat4<F32> TMat4F32Base::operator+(const TMat4<F32>& b) const;

template<>
TMat4<F32>& TMat4F32Base::operator+=(const TMat4<F32>& b);

template<>
TMat4<F32> TMat4F32Base::operator-(const TMat4<F32>& b) const;

template<>
TMat4<F32>& TMat4F32Base::operator-=(const TMat4<F32>& b);

template<>
TMat4<F32> TMat4F32Base::operator*(const TMat4<F32>& b) const;

template<>
TVec4<F32> TMat4F32Base::operator*(const TVec4<F32>& b) const;

template<>
void TMat4F32Base::setRows(const TVec4<F32>& a, const TVec4<F32>& b, const TVec4<F32>& c, const TVec4<F32>& d);

template<>
void TMat4F32Base::setRow(const U i, const TVec4<F32>& v);

template<>
void TMat4F32Base::transpose();

#elif ANKI_SIMD == ANKI_SIMD_NEON

#	error "TODO"

#endif

/// F32 4x4 matrix
using Mat4 = TMat4<F32>;
static_assert(sizeof(Mat4) == sizeof(F32) * 4 * 4, "Incorrect size");
/// @}

} // end namespace anki

#include <anki/math/Mat4.inl.h>
