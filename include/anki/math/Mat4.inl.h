#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Friends                                                                     =
//==============================================================================

//==============================================================================
template<typename T>
TMat4<T> operator+(const T f, const TMat4<T>& m4)
{
	return m4 + f;
}

//==============================================================================
template<typename T>
TMat4<T> operator-(const T f, const TMat4<T>& m4)
{
	TMat4<T> out;
	for(U i = 0; i < 16; i++)
	{
		out[i] = f - m4[i];
	}
	return out;
}

//==============================================================================
template<typename T>
TMat4<T> operator*(const T f, const TMat4<T>& m4)
{
	return m4 * f;
}

//==============================================================================
template<typename T>
TMat4<T> operator/(const T f, const TMat4<T>& m4)
{
	TMat4<T> out;
	for(U i = 0; i < 16; i++)
	{
		out[i] = f / m4[i];
	}
	return out;
}

#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE

//==============================================================================
// SSE specializations                                                         =
//==============================================================================

//==============================================================================
// Constructors                                                                =
//==============================================================================

//==============================================================================
template<>
inline TMat4<F32>::TMat4(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		simd[i] = b.simd[i];
	}
}

//==============================================================================
template<>
inline TMat4<F32>::TMat4(const F32 f)
{
	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_set1_ps(f);
	}
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator=(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		simd[i] = b.simd[i];
	}
	return *this;
}

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator+(const TMat4<F32>& b) const
{
	TMat4<F32> c;
	for(U i = 0; i < 4; i++)
	{
		c.simd[i] = _mm_add_ps(simd[i], b.simd[i]);
	}
	return c;
}

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator+=(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_add_ps(simd[i], b.simd[i]);
	}
	return *this;
}

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator-(const TMat4<F32>& b) const
{
	TMat4<F32> c;
	for(U i = 0; i < 4; i++)
	{
		c.simd[i] = _mm_sub_ps(simd[i], b.simd[i]);
	}
	return c;
}

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator-=(const TMat4<F32>& b)
{
	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_sub_ps(simd[i], b.simd[i]);
	}
	return *this;
}

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator*(const TMat4<F32>& b) const
{
	TMat4<F32> c;
	TMat4<F32> t(b);
	t.transpose();
	
	// XXX See if this is optimal
	for(U i = 0; i < 4; i++)
	{
		for(U j = 0; j < 4; j++)
		{
			_mm_store_ss(&c(i, j), _mm_dp_ps(simd[i], t.simd[j], 0xF1));
		}
	}

	return c;
}

//==============================================================================
// Operators with F32                                                          =
//==============================================================================

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator+(const F32 f) const
{
	TMat4<F32> c;
	__m128 mm = _mm_set1_ps(f);
	
	for(U i = 0; i < 4; i++)
	{
		c.simd[i] = _mm_add_ps(simd[i], mm);
	}

	return c;
}

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator+=(const F32 f)
{
	__m128 mm = _mm_set1_ps(f);
	
	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_add_ps(simd[i], mm);
	}

	return *this;
}

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator-(const F32 f) const
{
	TMat4<F32> r;
	__m128 mm = _mm_set1_ps(f);
	
	for(U i = 0; i < 4; i++)
	{
		r.simd[i] = _mm_sub_ps(simd[i], mm);
	}

	return r;
}

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator-=(const F32 f)
{
	__m128 mm = _mm_set1_ps(f);
	
	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_sub_ps(simd[i], mm);
	}

	return (*this);
}

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator*(const F32 f) const
{
	TMat4<F32> r;
	__m128 mm = _mm_set1_ps(f);
	
	for(U i = 0; i < 4; i++)
	{
		r.simd[i] = _mm_mul_ps(simd[i], mm);
	}

	return r;
}

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator*=(const F32 f)
{
	__m128 mm = _mm_set1_ps(f);

	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_mul_ps(simd[i], mm);
	}

	return *this;
}

//==============================================================================
template<>
inline TMat4<F32> TMat4<F32>::operator/(const F32 f) const
{
	TMat4<F32> r;
	__m128 mm = _mm_set1_ps(f);

	for(U i = 0; i < 4; i++)
	{
		r.simd[i] = _mm_div_ps(simd[i], mm);
	}

	return r;
}

//==============================================================================
template<>
inline TMat4<F32>& TMat4<F32>::operator/=(const F32 f)
{
	__m128 mm = _mm_set1_ps(f);

	for(U i = 0; i < 4; i++)
	{
		simd[i] = _mm_div_ps(simd[i], mm);
	}

	return *this;
}

//==============================================================================
// Operators with other                                                        =
//==============================================================================

//==============================================================================
template<>
inline TVec4<F32> TMat4<F32>::operator*(const TVec4<F32>& b) const
{
	TVec4<F32> v;
	
	for(U i = 0; i < 4; i++)
	{
		_mm_store_ss(&v[i], _mm_dp_ps(simd[i], b.getSimd(), 0xF1));
	}

	return v;
}

//==============================================================================
// Other                                                                       =
//==============================================================================

//==============================================================================
template<>
inline void TMat4<F32>::setRows(const TVec4<F32>& a, const TVec4<F32>& b, 
	const TVec4<F32>& c, const TVec4<F32>& d)
{
	simd[0] = a.getSimd();
	simd[1] = b.getSimd();
	simd[2] = c.getSimd();
	simd[3] = d.getSimd();
}

//==============================================================================
template<>
inline void TMat4<F32>::setRow(const U i, const TVec4<F32>& v)
{
	simd[i] = v.getSimd();
}

//==============================================================================
template<>
inline void TMat4<F32>::transpose()
{
	_MM_TRANSPOSE4_PS(simd[0], simd[1], simd[2], simd[3]);
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

//==============================================================================
template<>
inline TMat4<F32> operator-(const F32 f, const TMat4<F32>& m4)
{
	TMat4<F32> r;
	__m128 mm = _mm_set1_ps(f);

	for(U i = 0; i < 4; i++)
	{
		r.simd[i] = _mm_sub_ps(mm, m4.simd[i]);
	}

	return r;
}

//==============================================================================
template<>
inline TMat4<F32> operator/(const F32 f, const TMat4<F32>& m4)
{
	TMat4<F32> r;
	__m128 mm = _mm_set1_ps(f);

	for(U i = 0; i < 4; i++)
	{
		r.simd[i] = _mm_div_ps(mm, m4.simd[i]);
	}

	return r;
}

#endif

} // end namespace anki
