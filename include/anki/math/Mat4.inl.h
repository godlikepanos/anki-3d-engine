#include "anki/math/MathCommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// Copy
inline Mat4::Mat4(const Mat4& b)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = b.arrMm[i];
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] = b[i];
	}
#endif
}

// F32
inline Mat4::Mat4(const F32 f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_set1_ps(f);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] = f;
	}
#endif
}

// F32[]
inline Mat4::Mat4(const F32 arr_[])
{
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] = arr_[i];
	}
}

// many F32s
inline Mat4::Mat4(const F32 m00, const F32 m01, const F32 m02,
	const F32 m03, const F32 m10, const F32 m11,
	const F32 m12, const F32 m13, const F32 m20,
	const F32 m21, const F32 m22, const F32 m23,
	const F32 m30, const F32 m31, const F32 m32,
	const F32 m33)
{
	(*this)(0, 0) = m00;
	(*this)(0, 1) = m01;
	(*this)(0, 2) = m02;
	(*this)(0, 3) = m03;
	(*this)(1, 0) = m10;
	(*this)(1, 1) = m11;
	(*this)(1, 2) = m12;
	(*this)(1, 3) = m13;
	(*this)(2, 0) = m20;
	(*this)(2, 1) = m21;
	(*this)(2, 2) = m22;
	(*this)(2, 3) = m23;
	(*this)(3, 0) = m30;
	(*this)(3, 1) = m31;
	(*this)(3, 2) = m32;
	(*this)(3, 3) = m33;
}

// Mat3
inline Mat4::Mat4(const Mat3& m3)
{
	(*this)(0, 0) = m3(0, 0);
	(*this)(0, 1) = m3(0, 1);
	(*this)(0, 2) = m3(0, 2);
	(*this)(0, 3) = 0.0;
	(*this)(1, 0) = m3(1, 0);
	(*this)(1, 1) = m3(1, 1);
	(*this)(1, 2) = m3(1, 2);
	(*this)(1, 3) = 0.0;
	(*this)(2, 0) = m3(2, 0);
	(*this)(2, 1) = m3(2, 1);
	(*this)(2, 2) = m3(2, 2);
	(*this)(2, 3) = 0.0;
	(*this)(3, 0) = 0.0;
	(*this)(3, 1) = 0.0;
	(*this)(3, 2) = 0.0;
	(*this)(3, 3) = 1.0;
}

// Vec3
inline Mat4::Mat4(const Vec3& v)
{
	(*this)(0, 0) = 1.0;
	(*this)(0, 1) = 0.0;
	(*this)(0, 2) = 0.0;
	(*this)(0, 3) = v.x();
	(*this)(1, 0) = 0.0;
	(*this)(1, 1) = 1.0;
	(*this)(1, 2) = 0.0;
	(*this)(1, 3) = v.y();
	(*this)(2, 0) = 0.0;
	(*this)(2, 1) = 0.0;
	(*this)(2, 2) = 1.0;
	(*this)(2, 3) = v.z();
	(*this)(3, 0) = 0.0;
	(*this)(3, 1) = 0.0;
	(*this)(3, 2) = 0.0;
	(*this)(3, 3) = 1.0;
}

// vec4
inline Mat4::Mat4(const Vec4& v)
{
	(*this)(0, 0) = 1.0;
	(*this)(0, 1) = 0.0;
	(*this)(0, 2) = 0.0;
	(*this)(0, 3) = v.x();
	(*this)(1, 0) = 0.0;
	(*this)(1, 1) = 1.0;
	(*this)(1, 2) = 0.0;
	(*this)(1, 3) = v.y();
	(*this)(2, 0) = 0.0;
	(*this)(2, 1) = 0.0;
	(*this)(2, 2) = 1.0;
	(*this)(2, 3) = v.z();
	(*this)(3, 0) = 0.0;
	(*this)(3, 1) = 0.0;
	(*this)(3, 2) = 0.0;
	(*this)(3, 3) = v.w();
}

// Vec3, Mat3
inline Mat4::Mat4(const Vec3& transl, const Mat3& rot)
{
	setRotationPart(rot);
	setTranslationPart(transl);
	(*this)(3, 0) = (*this)(3, 1) = (*this)(3, 2) = 0.0;
	(*this)(3, 3) = 1.0;
}

// Vec3, Mat3, F32
inline Mat4::Mat4(const Vec3& translate, const Mat3& rotate, const F32 scale)
{
	if(!Math::isZero(scale - 1.0))
	{
		setRotationPart(rotate * scale);
	}
	else
	{
		setRotationPart(rotate);
	}

	setTranslationPart(translate);

	(*this)(3, 0) = (*this)(3, 1) = (*this)(3, 2) = 0.0;
	(*this)(3, 3) = 1.0;
}

// Transform
inline Mat4::Mat4(const Transform& t)
{
	(*this) = Mat4(t.getOrigin(), t.getRotation(), t.getScale());
}

//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline F32& Mat4::operator()(const U i, const U j)
{
	return arr2[i][j];
}

inline const F32& Mat4::operator()(const U i, const U j) const
{
	return arr2[i][j];
}

inline F32& Mat4::operator[](const U i)
{
	return arr1[i];
}

inline const F32& Mat4::operator[](const U i) const
{
	return arr1[i];
}

#if defined(ANKI_MATH_INTEL_SIMD)
inline const __m128& Mat4::getMm(const U i) const
{
	return arrMm[i];
}

inline __m128& Mat4::getMm(const U i)
{
	return arrMm[i];
}
#endif

//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Mat4& Mat4::operator=(const Mat4& b)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = b.arrMm[i];
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] = b[i];
	}
#endif
	return (*this);
}

// +
inline Mat4 Mat4::operator+(const Mat4& b) const
{
	Mat4 c;
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		c.arrMm[i] = _mm_add_ps(arrMm[i], b.arrMm[i]);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		c[i] = (*this)[i] + b[i];
	}
#endif
	return c;
}

// +=
inline Mat4& Mat4::operator+=(const Mat4& b)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_add_ps(arrMm[i], b.arrMm[i]);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] += b[i];
	}
#endif
	return (*this);
}

// -
inline Mat4 Mat4::operator-(const Mat4& b) const
{
	Mat4 c;
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		c.arrMm[i] = _mm_sub_ps(arrMm[i], b.arrMm[i]);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		c[i] = (*this)[i] - b[i];
	}
#endif
	return c;
}

// -=
inline Mat4& Mat4::operator-=(const Mat4& b)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_sub_ps(arrMm[i], b.arrMm[i]);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] -= b[i];
	}
#endif
	return (*this);
}

// *
inline Mat4 Mat4::operator*(const Mat4& b) const
{
	Mat4 c;
#if defined(ANKI_MATH_INTEL_SIMD)
	Mat4 t(b);
	t.transpose();
	for(U i = 0; i < 4; i++)
	{
		for(U j = 0; j < 4; j++)
		{
			_mm_store_ss(&c(i, j), _mm_dp_ps(arrMm[i], t.arrMm[j], 0xF1));
		}
	}
#else
	for(U i = 0; i < 4; i++)
	{
		for(U j = 0; j < 4; j++)
		{
			c(i, j) = (*this)(i, 0) * b(0, j) + (*this)(i, 1) * b(1, j) 
				+ (*this)(i, 2) * b(2, j) + (*this)(i, 3) * b(3, j);
		}
	}
#endif
	return c;
}

// *=
inline Mat4& Mat4::operator*=(const Mat4& b)
{
	(*this) = (*this) * b;
	return (*this);
}

// ==
inline Bool Mat4::operator==(const Mat4& b) const
{
	for(U i = 0; i < 16; i++)
	{
		if(!Math::isZero((*this)[i] - b[i]))
		{
			return false;
		}
	}
	return true;
}

// !=
inline Bool Mat4::operator!=(const Mat4& b) const
{
	for(U i = 0; i < 16; i++)
	{
		if(!Math::isZero((*this)[i]-b[i]))
		{
			return true;
		}
	}
	return false;
}

//==============================================================================
// Operators with F32                                                          =
//==============================================================================

// 4x4 + F32
inline Mat4 Mat4::operator+(const F32 f) const
{
	Mat4 c;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		c.arrMm[i] = _mm_add_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		c[i] = (*this)[i] + f;
	}
#endif
	return c;
}

// 4x4 += F32
inline Mat4& Mat4::operator+=(const F32 f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_add_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] += f;
	}
#endif
	return (*this);
}

// 4x4 - F32
inline Mat4 Mat4::operator-(const F32 f) const
{
	Mat4 r;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		r.arrMm[i] = _mm_sub_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		r[i] = (*this)[i] - f;
	}
#endif
	return r;
}

// 4x4 -= F32
inline Mat4& Mat4::operator-=(const F32 f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_sub_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] -= f;
	}
#endif
	return (*this);
}

// 4x4 * F32
inline Mat4 Mat4::operator*(const F32 f) const
{
	Mat4 r;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		r.arrMm[i] = _mm_mul_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		r[i] = (*this)[i] * f;
	}
#endif
	return r;
}

// 4x4 *= F32
inline Mat4& Mat4::operator*=(const F32 f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_mul_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] *= f;
	}
#endif
	return (*this);
}

// 4x4 / F32
inline Mat4 Mat4::operator/(const F32 f) const
{
	Mat4 r;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		r.arrMm[i] = _mm_div_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		r[i] = (*this)[i] / f;
	}
#endif
	return r;
}

// 4x4 /= F32
inline Mat4& Mat4::operator/=(const F32 f)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		arrMm[i] = _mm_div_ps(arrMm[i], mm);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		(*this)[i] /= f;
	}
#endif
	return (*this);
}

//==============================================================================
// Operators with other                                                        =
//==============================================================================

// Mat4 * Vec4
inline Vec4 Mat4::operator*(const Vec4& b) const
{
#if defined(ANKI_MATH_INTEL_SIMD)
	Vec4 v;
	for(U i = 0; i < 4; i++)
	{
		_mm_store_ss(&v[i], _mm_dp_ps(arrMm[i], b.getMm(), 0xF1));
	}
	return v;
#else
	Vec4 out;

	out.x() = (*this)(0, 0) * b.x() + (*this)(0, 1) * b.y() 
		+ (*this)(0, 2) * b.z() + (*this)(0, 3) * b.w();

	out.y() = (*this)(1, 0) * b.x() + (*this)(1, 1) * b.y() 
		+ (*this)(1, 2) * b.z() + (*this)(1, 3) * b.w();

	out.z() = (*this)(2, 0) * b.x() + (*this)(2, 1) * b.y() 
		+ (*this)(2, 2) * b.z() + (*this)(2, 3) * b.w();

	out.w() = (*this)(3, 0) * b.x() + (*this)(3, 1) * b.y() 
		+ (*this)(3, 2) * b.z() + (*this)(3, 3) * b.w();

	return out;
#endif
}

//==============================================================================
// Other                                                                       =
//==============================================================================

// setRows
inline void Mat4::setRows(const Vec4& a, const Vec4& b, const Vec4& c,
	const Vec4& d)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	arrMm[0] = a.getMm();
	arrMm[1] = b.getMm();
	arrMm[2] = c.getMm();
	arrMm[3] = d.getMm();
#else
	(*this)(0, 0) = a.x();
	(*this)(0, 1) = a.y();
	(*this)(0, 2) = a.z();
	(*this)(0, 3) = a.w();
	(*this)(1, 0) = b.x();
	(*this)(1, 1) = b.y();
	(*this)(1, 2) = b.z();
	(*this)(1, 3) = b.w();
	(*this)(2, 0) = c.x();
	(*this)(2, 1) = c.y();
	(*this)(2, 2) = c.z();
	(*this)(2, 3) = c.w();
	(*this)(3, 0) = d.x();
	(*this)(3, 1) = d.y();
	(*this)(3, 2) = d.z();
	(*this)(3, 3) = d.w();
#endif
}

// setRow
inline void Mat4::setRow(const U i, const Vec4& v)
{
#if defined(ANKI_MATH_INTEL_SIMD)
	arrMm[i] = v.getMm();
#else
	(*this)(i, 0) = v.x();
	(*this)(i, 1) = v.y();
	(*this)(i, 2) = v.z();
	(*this)(i, 3) = v.w();
#endif
}

// setColumns
inline void Mat4::setColumns(const Vec4& a, const Vec4& b, const Vec4& c,
	const Vec4& d)
{
	(*this)(0, 0) = a.x();
	(*this)(1, 0) = a.y();
	(*this)(2, 0) = a.z();
	(*this)(3, 0) = a.w();
	(*this)(0, 1) = b.x();
	(*this)(1, 1) = b.y();
	(*this)(2, 1) = b.z();
	(*this)(3, 1) = b.w();
	(*this)(0, 2) = c.x();
	(*this)(1, 2) = c.y();
	(*this)(2, 2) = c.z();
	(*this)(3, 2) = c.w();
	(*this)(0, 3) = d.x();
	(*this)(1, 3) = d.y();
	(*this)(2, 3) = d.z();
	(*this)(3, 3) = d.w();
}

// setColumn
inline void Mat4::setColumn(const U i, const Vec4& v)
{
	(*this)(0, i) = v.x();
	(*this)(1, i) = v.y();
	(*this)(2, i) = v.z();
	(*this)(3, i) = v.w();
}

// transpose
inline void Mat4::transpose()
{
#if defined(ANKI_MATH_INTEL_SIMD)
	_MM_TRANSPOSE4_PS(arrMm[0], arrMm[1], arrMm[2], arrMm[3]);
#else
	F32 tmp = (*this)(0, 1);
	(*this)(0, 1) = (*this)(1, 0);
	(*this)(1, 0) = tmp;
	tmp = (*this)(0, 2);
	(*this)(0, 2) = (*this)(2, 0);
	(*this)(2, 0) = tmp;
	tmp = (*this)(0, 3);
	(*this)(0, 3) = (*this)(3, 0);
	(*this)(3, 0) = tmp;
	tmp = (*this)(1, 2);
	(*this)(1, 2) = (*this)(2, 1);
	(*this)(2, 1) = tmp;
	tmp = (*this)(1, 3);
	(*this)(1, 3) = (*this)(3, 1);
	(*this)(3, 1) = tmp;
	tmp = (*this)(2, 3);
	(*this)(2, 3) = (*this)(3, 2);
	(*this)(3, 2) = tmp;
#endif
}

// getTransposed
inline Mat4 Mat4::getTransposed() const
{
	Mat4 m4;
	m4[0] = (*this)[0];
	m4[1] = (*this)[4];
	m4[2] = (*this)[8];
	m4[3] = (*this)[12];
	m4[4] = (*this)[1];
	m4[5] = (*this)[5];
	m4[6] = (*this)[9];
	m4[7] = (*this)[13];
	m4[8] = (*this)[2];
	m4[9] = (*this)[6];
	m4[10] = (*this)[10];
	m4[11] = (*this)[14];
	m4[12] = (*this)[3];
	m4[13] = (*this)[7];
	m4[14] = (*this)[11];
	m4[15] = (*this)[15];
	return m4;
}

// setRotationPart
inline void Mat4::setRotationPart(const Mat3& m3)
{
	(*this)(0, 0) = m3(0, 0);
	(*this)(0, 1) = m3(0, 1);
	(*this)(0, 2) = m3(0, 2);
	(*this)(1, 0) = m3(1, 0);
	(*this)(1, 1) = m3(1, 1);
	(*this)(1, 2) = m3(1, 2);
	(*this)(2, 0) = m3(2, 0);
	(*this)(2, 1) = m3(2, 1);
	(*this)(2, 2) = m3(2, 2);
}

// getRotationPart
inline Mat3 Mat4::getRotationPart() const
{
	Mat3 m3;
	m3(0, 0) = (*this)(0, 0);
	m3(0, 1) = (*this)(0, 1);
	m3(0, 2) = (*this)(0, 2);
	m3(1, 0) = (*this)(1, 0);
	m3(1, 1) = (*this)(1, 1);
	m3(1, 2) = (*this)(1, 2);
	m3(2, 0) = (*this)(2, 0);
	m3(2, 1) = (*this)(2, 1);
	m3(2, 2) = (*this)(2, 2);
	return m3;
}

// setTranslationPart
inline void Mat4::setTranslationPart(const Vec4& v)
{
	(*this)(0, 3) = v.x();
	(*this)(1, 3) = v.y();
	(*this)(2, 3) = v.z();
	(*this)(3, 3) = v.w();
}

// setTranslationPart
inline void Mat4::setTranslationPart(const Vec3& v)
{
	(*this)(0, 3) = v.x();
	(*this)(1, 3) = v.y();
	(*this)(2, 3) = v.z();
}

// getTranslationPart
inline Vec3 Mat4::getTranslationPart() const
{
	return Vec3((*this)(0, 3), (*this)(1, 3), (*this)(2, 3));
}

// getIdentity
inline const Mat4& Mat4::getIdentity()
{
	static const Mat4 ident(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
		1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
	return ident;
}

// getZero
inline const Mat4& Mat4::getZero()
{
	static const Mat4 zero(0.0);
	return zero;
}

// Determinant
inline F32 Mat4::getDet() const
{
	const Mat4& t = *this;
	return t(0, 3) * t(1, 2) * t(2, 1) * t(3, 0) 
		- t(0, 2) * t(1, 3) * t(2, 1) * t(3, 0) 
		- t(0, 3) * t(1, 1) * t(2, 2) * t(3, 0) 
		+ t(0, 1) * t(1, 3) * t(2, 2) * t(3, 0)
		+ t(0, 2) * t(1, 1) * t(2, 3) * t(3, 0) 
		- t(0, 1) * t(1, 2) * t(2, 3) * t(3, 0) 
		- t(0, 3) * t(1, 2) * t(2, 0) * t(3, 1) 
		+ t(0, 2) * t(1, 3) * t(2, 0) * t(3, 1) 
		+ t(0, 3) * t(1, 0) * t(2, 2) * t(3, 1) 
		- t(0, 0) * t(1, 3) * t(2, 2) * t(3, 1)
		- t(0, 2) * t(1, 0) * t(2, 3) * t(3, 1) 
		+ t(0, 0) * t(1, 2) * t(2, 3) * t(3, 1) 
		+ t(0, 3) * t(1, 1) * t(2, 0) * t(3, 2) 
		- t(0, 1) * t(1, 3) * t(2, 0) * t(3, 2)
		- t(0, 3) * t(1, 0) * t(2, 1) * t(3, 2)
		+ t(0, 0) * t(1, 3) * t(2, 1) * t(3, 2) 
		+ t(0, 1) * t(1, 0) * t(2, 3) * t(3, 2)
		- t(0, 0) * t(1, 1) * t(2, 3) * t(3, 2)
		- t(0, 2) * t(1, 1) * t(2, 0) * t(3, 3)
		+ t(0, 1) * t(1, 2) * t(2, 0) * t(3, 3)
		+ t(0, 2) * t(1, 0) * t(2, 1) * t(3, 3)
		- t(0, 0) * t(1, 2) * t(2, 1) * t(3, 3) 
		- t(0, 1) * t(1, 0) * t(2, 2) * t(3, 3) 
		+ t(0, 0) * t(1, 1) * t(2, 2) * t(3, 3);
}

// getInverse
inline Mat4 Mat4::getInverse() const
{
	std::array<F32, 12> tmp;
	const Mat4& in = (*this);
	Mat4 m4;

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

	m4(0, 0) =  tmp[0] * in(1, 1) + tmp[3] * in(2, 1) + tmp[4] * in(3, 1);
	m4(0, 0) -= tmp[1] * in(1, 1) + tmp[2] * in(2, 1) + tmp[5] * in(3, 1);
	m4(0, 1) =  tmp[1] * in(0, 1) + tmp[6] * in(2, 1) + tmp[9] * in(3, 1);
	m4(0, 1) -= tmp[0] * in(0, 1) + tmp[7] * in(2, 1) + tmp[8] * in(3, 1);
	m4(0, 2) =  tmp[2] * in(0, 1) + tmp[7] * in(1, 1) + tmp[10] * in(3, 1);
	m4(0, 2) -= tmp[3] * in(0, 1) + tmp[6] * in(1, 1) + tmp[11] * in(3, 1);
	m4(0, 3) =  tmp[5] * in(0, 1) + tmp[8] * in(1, 1) + tmp[11] * in(2, 1);
	m4(0, 3) -= tmp[4] * in(0, 1) + tmp[9] * in(1, 1) + tmp[10] * in(2, 1);
	m4(1, 0) =  tmp[1] * in(1, 0) + tmp[2] * in(2, 0) + tmp[5] * in(3, 0);
	m4(1, 0) -= tmp[0] * in(1, 0) + tmp[3] * in(2, 0) + tmp[4] * in(3, 0);
	m4(1, 1) =  tmp[0] * in(0, 0) + tmp[7] * in(2, 0) + tmp[8] * in(3, 0);
	m4(1, 1) -= tmp[1] * in(0, 0) + tmp[6] * in(2, 0) + tmp[9] * in(3, 0);
	m4(1, 2) =  tmp[3] * in(0, 0) + tmp[6] * in(1, 0) + tmp[11] * in(3, 0);
	m4(1, 2) -= tmp[2] * in(0, 0) + tmp[7] * in(1, 0) + tmp[10] * in(3, 0);
	m4(1, 3) =  tmp[4] * in(0, 0) + tmp[9] * in(1, 0) + tmp[10] * in(2, 0);
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

	m4(2, 0) =  tmp[0] * in(1, 3) + tmp[3] * in(2, 3) + tmp[4] * in(3, 3);
	m4(2, 0) -= tmp[1] * in(1, 3) + tmp[2] * in(2, 3) + tmp[5] * in(3, 3);
	m4(2, 1) =  tmp[1] * in(0, 3) + tmp[6] * in(2, 3) + tmp[9] * in(3, 3);
	m4(2, 1) -= tmp[0] * in(0, 3) + tmp[7] * in(2, 3) + tmp[8] * in(3, 3);
	m4(2, 2) =  tmp[2] * in(0, 3) + tmp[7] * in(1, 3) + tmp[10] * in(3, 3);
	m4(2, 2) -= tmp[3] * in(0, 3) + tmp[6] * in(1, 3) + tmp[11] * in(3, 3);
	m4(2, 3) =  tmp[5] * in(0, 3) + tmp[8] * in(1, 3) + tmp[11] * in(2, 3);
	m4(2, 3) -= tmp[4] * in(0, 3) + tmp[9] * in(1, 3) + tmp[10] * in(2, 3);
	m4(3, 0) =  tmp[2] * in(2, 2) + tmp[5] * in(3, 2) + tmp[1] * in(1, 2);
	m4(3, 0) -= tmp[4] * in(3, 2) + tmp[0] * in(1, 2) + tmp[3] * in(2, 2);
	m4(3, 1) =  tmp[8] * in(3, 2) + tmp[0] * in(0, 2) + tmp[7] * in(2, 2);
	m4(3, 1) -= tmp[6] * in(2, 2) + tmp[9] * in(3, 2) + tmp[1] * in(0, 2);
	m4(3, 2) =  tmp[6] * in(1, 2) + tmp[11] * in(3, 2) + tmp[3] * in(0, 2);
	m4(3, 2) -= tmp[10] * in(3, 2) + tmp[2] * in(0, 2) + tmp[7] * in(1, 2);
	m4(3, 3) =  tmp[10] * in(2, 2) + tmp[4] * in(0, 2) + tmp[9] * in(1, 2);
	m4(3, 3) -= tmp[8] * in(1, 2) + tmp[11] * in(2, 2) + tmp[5] * in(0, 2);

	F32 det = in(0, 0) * m4(0, 0) + in(1, 0) * m4(0, 1) 
		+ in(2, 0) * m4(0, 2) + in(3, 0) * m4(0, 3);

	ANKI_ASSERT(!Math::isZero(det)); // Cannot invert, det == 0
	det = 1.0 / det;
	m4 *= det;
	return m4;
}

// invert
inline void Mat4::invert()
{
	(*this) = getInverse();
}

// getInverseTransformation
inline Mat4 Mat4::getInverseTransformation() const
{
	Mat3 invertedRot = getRotationPart().getTransposed();
	Vec3 invertedTsl = getTranslationPart();
	invertedTsl = -(invertedRot * invertedTsl);
	return Mat4(invertedTsl, invertedRot);
}

// lerp
inline Mat4 Mat4::lerp(const Mat4& b, const F32 t) const
{
	return ((*this) * (1.0 - t)) + (b * t);
}

// setIdentity
inline void Mat4::setIdentity()
{
	(*this) = getIdentity();
}

// combineTransformations
inline Mat4 Mat4::combineTransformations(const Mat4& m0, const Mat4& m1)
{
	// See the clean code in < r664

	// one of the 2 mat4 doesnt represent transformation
	ANKI_ASSERT(Math::isZero(m0(3, 0) + m0(3, 1) + m0(3, 2) + m0(3, 3) - 1.0)
		&& Math::isZero(m1(3, 0) + m1(3, 1) + m1(3, 2) + m1(3, 3) - 1.0));

	Mat4 m4;

	m4(0, 0) = m0(0, 0) * m1(0, 0) + m0(0, 1) * m1(1, 0) + m0(0, 2) * m1(2, 0);
	m4(0, 1) = m0(0, 0) * m1(0, 1) + m0(0, 1) * m1(1, 1) + m0(0, 2) * m1(2, 1);
	m4(0, 2) = m0(0, 0) * m1(0, 2) + m0(0, 1) * m1(1, 2) + m0(0, 2) * m1(2, 2);
	m4(1, 0) = m0(1, 0) * m1(0, 0) + m0(1, 1) * m1(1, 0) + m0(1, 2) * m1(2, 0);
	m4(1, 1) = m0(1, 0) * m1(0, 1) + m0(1, 1) * m1(1, 1) + m0(1, 2) * m1(2, 1);
	m4(1, 2) = m0(1, 0) * m1(0, 2) + m0(1, 1) * m1(1, 2) + m0(1, 2) * m1(2, 2);
	m4(2, 0) = m0(2, 0) * m1(0, 0) + m0(2, 1) * m1(1, 0) + m0(2, 2) * m1(2, 0);
	m4(2, 1) = m0(2, 0) * m1(0, 1) + m0(2, 1) * m1(1, 1) + m0(2, 2) * m1(2, 1);
	m4(2, 2) = m0(2, 0) * m1(0, 2) + m0(2, 1) * m1(1, 2) + m0(2, 2) * m1(2, 2);

	m4(0, 3) = m0(0, 0) * m1(0, 3) + m0(0, 1) * m1(1, 3) 
		+ m0(0, 2) * m1(2, 3) + m0(0, 3);

	m4(1, 3) = m0(1, 0) * m1(0, 3) + m0(1, 1) * m1(1, 3) 
		+ m0(1, 2) * m1(2, 3) + m0(1, 3);

	m4(2, 3) = m0(2, 0) * m1(0, 3) + m0(2, 1) * m1(1, 3) 
		+ m0(2, 2) * m1(2, 3) + m0(2, 3);

	m4(3, 0) = m4(3, 1) = m4(3, 2) = 0.0;
	m4(3, 3) = 1.0;

	return m4;
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

// F32 + 4x4
inline Mat4 operator+(const F32 f, const Mat4& m4)
{
	return m4 + f;
}

// F32 - 4x4
inline Mat4 operator-(const F32 f, const Mat4& m4)
{
	Mat4 r;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		r.arrMm[i] = _mm_sub_ps(mm, m4.arrMm[i]);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		r[i] = f - m4[i];
	}
#endif
	return r;
}

// F32 * 4x4
inline Mat4 operator*(const F32 f, const Mat4& m4)
{
	return m4 * f;
}

// F32 / 4x4
inline Mat4 operator/(const F32 f, const Mat4& m4)
{
	Mat4 r;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128 mm;
	mm = _mm_set1_ps(f);
	for(U i = 0; i < 4; i++)
	{
		r.arrMm[i] = _mm_div_ps(mm, m4.arrMm[i]);
	}
#else
	for(U i = 0; i < 16; i++)
	{
		r[i] = f / m4[i];
	}
#endif
	return r;
}

// Print
inline std::ostream& operator<<(std::ostream& s, const Mat4& m)
{
	for(U i = 0; i < 4; i++)
	{
		for(U j = 0; j < 4; j++)
		{
			s << m(i, j) << ' ';
		}

		if(i != 3)
		{
			s << "\n";
		}
	}
	return s;
}

} // end namespace
