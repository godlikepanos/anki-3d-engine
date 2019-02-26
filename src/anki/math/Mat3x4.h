// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

/// 3x4 Matrix. Mainly used for transformations. It includes many helpful member functions. Its row major. The columns
/// are the x,y,z axis
template<typename T>
class alignas(16) TMat3x4 : public TMat<T, 3, 4, TMat3x4<T>, TVec4<T>, TVec3<T>>
{
public:
	using Base = TMat<T, 3, 4, TMat3x4<T>, TVec4<T>, TVec3<T>>;

	using Base::Base;

	/// @name Constructors
	/// @{
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
};

#if ANKI_SIMD == ANKI_SIMD_SSE

// Forward declare specializations

using TMat3x4F32Base = TMat3x4<F32>::Base;

template<>
inline TVec3<F32> TMat3x4F32Base::operator*(const TVec4<F32>& b) const;

#elif ANKI_SIMD == ANKI_SIMD_NEON

#	error "TODO"

#endif

/// F32 4x4 matrix
using Mat3x4 = TMat3x4<F32>;
static_assert(sizeof(Mat3x4) == sizeof(F32) * 3 * 4, "Incorrect size");
/// @}

} // end namespace anki

#include <anki/math/Mat3x4.inl.h>
