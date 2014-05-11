#ifndef ANKI_MATH_MAT3X4_H
#define ANKI_MATH_MAT3X4_H

#include "anki/math/CommonIncludes.h"
#include "anki/math/Mat.h"

namespace anki {

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

/// 3x4 Matrix. Mainly used for rotations. It includes many helpful member
/// functions. Its row major. The columns are the x,y,z axis
template<typename T>
class alignas(16) TMat3x4: public TMat<T, 3, 4, typename TMat3x4Simd<T>::Type, 
	  TMat3x4<T>, TVec4<T>, TVec3<T>>
{
public:
	using Base = TMat<T, 3, 4, typename TMat3x4Simd<T>::Type, 
	  TMat3x4<T>, TVec4<T>, TVec3<T>>;

	using Base::Base;

	/// @name Constructors
	/// @{
	explicit TMat3x4(const TMat3<T>& m3)
	{
		TMat3x4& m = *this;
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
	}

	explicit TMat3x4(const TVec3<T>& v)
	{
		TMat3x4& m = *this;
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
	}

	explicit TMat3x4(const TVec3<T>& transl, const TMat3<T>& rot)
	{
		setRotationPart(rot);
		setTranslationPart(transl);
	}

	explicit TMat3x4(const TVec3<T>& transl, const TMat3<T>& rot, const T scale)
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
	}

	explicit TMat3x4(const TTransform<T>& t)
	{
		(*this) = TMat3x4(t.getOrigin(), t.getRotation(), t.getScale());
	}
	/// @}

	/// @name Other
	/// @{
	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TMat3x4& getIdentity()
	{
		static const TMat3x4 ident(
			1.0, 0.0, 0.0, 0.0, 
			0.0, 1.0, 0.0, 0.0, 
			0.0, 0.0, 1.0, 0.0);
		return ident;
	}
	/// @}	
};

/// @}

} // end namespace anki

#endif
