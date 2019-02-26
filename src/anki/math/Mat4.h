// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>

namespace anki
{

/// @addtogroup math
/// @{

/// 4x4 Matrix. Used mainly for transformations but not necessarily. It's row major. SSE optimized
/// @note TMat4*TMat4: 64 muls 48 adds
template<typename T>
class alignas(16) TMat4 : public TMat<T, 4, 4, TMat4<T>, TVec4<T>, TVec4<T>>
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
	using Base = TMat<T, 4, 4, TMat4<T>, TVec4<T>, TVec4<T>>;

	using Base::getTranslationPart;
	using Base::setTranslationPart;
	using Base::getRotationPart;
	using Base::setRotationPart;
	using Base::Base;

	/// @name Constructors
	/// @{
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

	/// If we suppose this matrix represents a transformation, return the inverted transformation
	TMat4 getInverseTransformation() const
	{
		TMat3<T> invertedRot = getRotationPart().getTransposed();
		TVec3<T> invertedTsl = getTranslationPart().xyz();
		invertedTsl = -(invertedRot * invertedTsl);
		return TMat4(invertedTsl.xyz0(), invertedRot);
	}

	/// @note 9 muls, 9 adds
	TVec3<T> transform(const TVec3<T>& v) const
	{
		const TMat4& m = *this;

		return TVec3<T>(m(0, 0) * v.x() + m(0, 1) * v.y() + m(0, 2) * v.z() + m(0, 3),
			m(1, 0) * v.x() + m(1, 1) * v.y() + m(1, 2) * v.z() + m(1, 3),
			m(2, 0) * v.x() + m(2, 1) * v.y() + m(2, 2) * v.z() + m(2, 3));
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
TVec4<F32> TMat4F32Base::operator*(const TVec4<F32>& b) const;

#elif ANKI_SIMD == ANKI_SIMD_NEON

#	error "TODO"

#endif

/// F32 4x4 matrix
using Mat4 = TMat4<F32>;
static_assert(sizeof(Mat4) == sizeof(F32) * 4 * 4, "Incorrect size");
/// @}

} // end namespace anki

#include <anki/math/Mat4.inl.h>
