#include "MathCommon.inl.h"


#define SELF (*this)


namespace M {


//======================================================================================================================
// Constructors                                                                                                        =
//======================================================================================================================

// Copy
inline Mat4::Mat4(const Mat4& b)
{
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = b.arrMm[i];
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] = b[i];
		}
	#endif
}

// float
inline Mat4::Mat4(const float f)
{
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_set1_ps(f);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] = f;
		}
	#endif
}

// float[]
inline Mat4::Mat4(const float arr_[])
{
	for(int i = 0; i < 16; i++)
	{
		SELF[i] = arr_[i];
	}
}

// many floats
inline Mat4::Mat4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20,
                  float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	SELF(0, 0) = m00;
	SELF(0, 1) = m01;
	SELF(0, 2) = m02;
	SELF(0, 3) = m03;
	SELF(1, 0) = m10;
	SELF(1, 1) = m11;
	SELF(1, 2) = m12;
	SELF(1, 3) = m13;
	SELF(2, 0) = m20;
	SELF(2, 1) = m21;
	SELF(2, 2) = m22;
	SELF(2, 3) = m23;
	SELF(3, 0) = m30;
	SELF(3, 1) = m31;
	SELF(3, 2) = m32;
	SELF(3, 3) = m33;
}

// Mat3
inline Mat4::Mat4(const Mat3& m3)
{
	SELF(0, 0) = m3(0, 0);
	SELF(0, 1) = m3(0, 1);
	SELF(0, 2) = m3(0, 2);
	SELF(1, 0) = m3(1, 0);
	SELF(1, 1) = m3(1, 1);
	SELF(1, 2) = m3(1, 2);
	SELF(2, 0) = m3(2, 0);
	SELF(2, 1) = m3(2, 1);
	SELF(2, 2) = m3(2, 2);
	SELF(3, 0) = SELF(3, 1) = SELF(3, 2) = SELF(0, 3) = SELF(1, 3) = SELF(2, 3) = 0.0;
	SELF(3, 3) = 1.0;
}

// Vec3
inline Mat4::Mat4(const Vec3& v)
{
	SELF(0, 0) = 1.0;
	SELF(0, 1) = 0.0;
	SELF(0, 2) = 0.0;
	SELF(0, 3) = v.x();
	SELF(1, 0) = 0.0;
	SELF(1, 1) = 1.0;
	SELF(1, 2) = 0.0;
	SELF(1, 3) = v.y();
	SELF(2, 0) = 0.0;
	SELF(2, 1) = 0.0;
	SELF(2, 2) = 1.0;
	SELF(2, 3) = v.z();
	SELF(3, 0) = 0.0;
	SELF(3, 1) = 0.0;
	SELF(3, 2) = 0.0;
	SELF(3, 3) = 1.0;
}

// vec4
inline Mat4::Mat4(const Vec4& v)
{
	SELF(0, 0) = 1.0;
	SELF(0, 1) = 0.0;
	SELF(0, 2) = 0.0;
	SELF(0, 3) = v.x();
	SELF(1, 0) = 0.0;
	SELF(1, 1) = 1.0;
	SELF(1, 2) = 0.0;
	SELF(1, 3) = v.y();
	SELF(2, 0) = 0.0;
	SELF(2, 1) = 0.0;
	SELF(2, 2) = 1.0;
	SELF(2, 3) = v.z();
	SELF(3, 0) = 0.0;
	SELF(3, 1) = 0.0;
	SELF(3, 2) = 0.0;
	SELF(3, 3) = v.w();
}

// Vec3, Mat3
inline Mat4::Mat4(const Vec3& transl, const Mat3& rot)
{
	setRotationPart(rot);
	setTranslationPart(transl);
	SELF(3, 0) = SELF(3, 1) = SELF(3, 2) = 0.0;
	SELF(3, 3) = 1.0;
}

// Vec3, Mat3, float
inline Mat4::Mat4(const Vec3& translate, const Mat3& rotate, float scale)
{
	if(!isZero(scale - 1.0))
	{
		setRotationPart(rotate * scale);
	}
	else
	{
		setRotationPart(rotate);
	}

	setTranslationPart(translate);

	SELF(3, 0) = SELF(3, 1) = SELF(3, 2) = 0.0;
	SELF(3, 3) = 1.0;
}

// Transform
inline Mat4::Mat4(const Transform& t)
{
	SELF = Mat4(t.getOrigin(), t.getRotation(), t.getScale());
}


//======================================================================================================================
// Accessors                                                                                                           =
//======================================================================================================================

inline float& Mat4::operator()(const uint i, const uint j)
{
	return arr2[i][j];
}

inline const float& Mat4::operator()(const uint i, const uint j) const
{
	return arr2[i][j];
}

inline float& Mat4::operator[](const uint i)
{
	return arr1[i];
}

inline const float& Mat4::operator[](const uint i) const
{
	return arr1[i];
}


//======================================================================================================================
// Operators with same                                                                                                 =
//======================================================================================================================

// =
inline Mat4& Mat4::operator=(const Mat4& b)
{
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = b.arrMm[i];
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] = b[i];
		}
	#endif
	return SELF;
}

// +
inline Mat4 Mat4::operator+(const Mat4& b) const
{
	Mat4 c;
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			c.arrMm[i] = _mm_add_ps(arrMm[i], b.arrMm[i]);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			c[i] = SELF[i] + b[i];
		}
	#endif
	return c;
}

// +=
inline Mat4& Mat4::operator+=(const Mat4& b)
{
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_add_ps(arrMm[i], b.arrMm[i]);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] += b[i];
		}
	#endif
	return SELF;
}

// -
inline Mat4 Mat4::operator-(const Mat4& b) const
{
	Mat4 c;
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			c.arrMm[i] = _mm_sub_ps(arrMm[i], b.arrMm[i]);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			c[i] = SELF[i] - b[i];
		}
	#endif
	return c;
}

// -=
inline Mat4& Mat4::operator-=(const Mat4& b)
{
	#if defined(MATH_INTEL_SIMD)
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_sub_ps(arrMm[i], b.arrMm[i]);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] -= b[i];
		}
	#endif
	return SELF;
}

// *
inline Mat4 Mat4::operator*(const Mat4& b) const
{
	Mat4 c;
	#if defined(MATH_INTEL_SIMD)
		Mat4 t(b);
		t.transpose();
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				_mm_store_ss(&c(i, j), _mm_dp_ps(arrMm[i], t.arrMm[j], 0xF1));
			}
		}
	#else
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				c(i, j) = SELF(i, 0) * b(0, j) + SELF(i, 1) * b(1, j) + SELF(i, 2) * b(2, j) + SELF(i, 3) * b(3, j);
			}
		}
	#endif
	return c;
}

// *=
inline Mat4& Mat4::operator*=(const Mat4& b)
{
	SELF = SELF * b;
	return SELF;
}

// ==
inline bool Mat4::operator==(const Mat4& b) const
{
	for(int i = 0; i < 16; i++)
	{
		if(!isZero(SELF[i] - b[i]))
		{
			return false;
		}
	}
	return true;
}

// !=
inline bool Mat4::operator!=(const Mat4& b) const
{
	for(int i = 0; i < 16; i++)
	{
		if(!isZero(SELF[i]-b[i]))
		{
			return true;
		}
	}
	return false;
}

//======================================================================================================================
// Operators with float                                                                                                =
//======================================================================================================================

// 4x4 + float
inline Mat4 Mat4::operator+(float f) const
{
	Mat4 c;
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			c.arrMm[i] = _mm_add_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			c[i] = SELF[i] + f;
		}
	#endif
	return c;
}

// float + 4x4
inline Mat4 operator+(float f, const Mat4& m4)
{
	return m4 + f;
}

// 4x4 += float
inline Mat4& Mat4::operator+=(float f)
{
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_add_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] += f;
		}
	#endif
	return SELF;
}

// 4x4 - float
inline Mat4 Mat4::operator-(float f) const
{
	Mat4 r;
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			r.arrMm[i] = _mm_sub_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			r[i] = SELF[i] - f;
		}
	#endif
	return r;
}

// float - 4x4
inline Mat4 operator-(float f, const Mat4& m4)
{
	Mat4 r;
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			r.arrMm[i] = _mm_sub_ps(mm, m4.arrMm[i]);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			r[i] = f - m4[i];
		}
	#endif
	return r;
}

// 4x4 -= float
inline Mat4& Mat4::operator-=(float f)
{
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_sub_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] -= f;
		}
	#endif
	return SELF;
}

// 4x4 * float
inline Mat4 Mat4::operator*(float f) const
{
	Mat4 r;
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			r.arrMm[i] = _mm_mul_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			r[i] = SELF[i] * f;
		}
	#endif
	return r;
}

// float * 4x4
inline Mat4 operator*(float f, const Mat4& m4)
{
	return m4 * f;
}

// 4x4 *= float
inline Mat4& Mat4::operator*=(float f)
{
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_mul_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] *= f;
		}
	#endif
	return SELF;
}

// 4x4 / float
inline Mat4 Mat4::operator/(float f) const
{
	Mat4 r;
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			r.arrMm[i] = _mm_div_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			r[i] = SELF[i] / f;
		}
	#endif
	return r;
}

// float / 4x4
inline Mat4 operator/(float f, const Mat4& m4)
{
	Mat4 r;
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			r.arrMm[i] = _mm_div_ps(mm, m4.arrMm[i]);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			r[i] = f / m4[i];
		}
	#endif
	return r;
}

// 4x4 /= float
inline Mat4& Mat4::operator/=(float f)
{
	#if defined(MATH_INTEL_SIMD)
		__m128 mm;
		mm = _mm_set1_ps(f);
		for(int i = 0; i < 4; i++)
		{
			arrMm[i] = _mm_div_ps(arrMm[i], mm);
		}
	#else
		for(int i = 0; i < 16; i++)
		{
			SELF[i] /= f;
		}
	#endif
	return SELF;
}


//======================================================================================================================
// Operators with other                                                                                                =
//======================================================================================================================

// Mat4 * Vec4
inline Vec4 Mat4::operator*(const Vec4& b) const
{
	#if defined(MATH_INTEL_SIMD)
		Vec4 v;
		for(int i = 0; i < 4; i++)
		{
			_mm_store_ss(&v[i], _mm_dp_ps(arrMm[i], b.getMm(), 0xF1));
		}
		return v;
	#else
		return Vec4(SELF(0, 0) * b.x() + SELF(0, 1) * b.y() + SELF(0, 2) * b.z() + SELF(0, 3) * b.w(),
		            SELF(1, 0) * b.x() + SELF(1, 1) * b.y() + SELF(1, 2) * b.z() + SELF(1, 3) * b.w(),
		            SELF(2, 0) * b.x() + SELF(2, 1) * b.y() + SELF(2, 2) * b.z() + SELF(2, 3) * b.w(),
		            SELF(3, 0) * b.x() + SELF(3, 1) * b.y() + SELF(3, 2) * b.z() + SELF(3, 3) * b.w());
	#endif
}


//======================================================================================================================
// Misc methods                                                                                                        =
//======================================================================================================================

// setRows
inline void Mat4::setRows(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d)
{
	#if defined(MATH_INTEL_SIMD)
		arrMm[0] = a.getMm();
		arrMm[1] = b.getMm();
		arrMm[2] = c.getMm();
		arrMm[3] = d.getMm();
	#else
		SELF(0, 0) = a.x();
		SELF(0, 1) = a.y();
		SELF(0, 2) = a.z();
		SELF(0, 3) = a.w();
		SELF(1, 0) = b.x();
		SELF(1, 1) = b.y();
		SELF(1, 2) = b.z();
		SELF(1, 3) = b.w();
		SELF(2, 0) = c.x();
		SELF(2, 1) = c.y();
		SELF(2, 2) = c.z();
		SELF(2, 3) = c.w();
		SELF(3, 0) = d.x();
		SELF(3, 1) = d.y();
		SELF(3, 2) = d.z();
		SELF(3, 3) = d.w();
	#endif
}

// setRow
inline void Mat4::setRow(uint i, const Vec4& v)
{
	#if defined(MATH_INTEL_SIMD)
		arrMm[i] = v.getMm();
	#else
		SELF(i, 0) = v.x();
		SELF(i, 1) = v.y();
		SELF(i, 2) = v.z();
		SELF(i, 3) = v.w();
	#endif
}

// setColumns
inline void Mat4::setColumns(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d)
{
	SELF(0, 0) = a.x();
	SELF(1, 0) = a.y();
	SELF(2, 0) = a.z();
	SELF(3, 0) = a.w();
	SELF(0, 1) = b.x();
	SELF(1, 1) = b.y();
	SELF(2, 1) = b.z();
	SELF(3, 1) = b.w();
	SELF(0, 2) = c.x();
	SELF(1, 2) = c.y();
	SELF(2, 2) = c.z();
	SELF(3, 2) = c.w();
	SELF(0, 3) = d.x();
	SELF(1, 3) = d.y();
	SELF(2, 3) = d.z();
	SELF(3, 3) = d.w();
}

// setColumn
inline void Mat4::setColumn(uint i, const Vec4& v)
{
	SELF(0, i) = v.x();
	SELF(1, i) = v.y();
	SELF(2, i) = v.z();
	SELF(3, i) = v.w();
}

// transpose
inline void Mat4::transpose()
{
	#if defined(MATH_INTEL_SIMD)
		_MM_TRANSPOSE4_PS(arrMm[0], arrMm[1], arrMm[2], arrMm[3]);
	#else
		float tmp = SELF(0, 1);
		SELF(0, 1) = SELF(1, 0);
		SELF(1, 0) = tmp;
		tmp = SELF(0, 2);
		SELF(0, 2) = SELF(2, 0);
		SELF(2, 0) = tmp;
		tmp = SELF(0, 3);
		SELF(0, 3) = SELF(3, 0);
		SELF(3, 0) = tmp;
		tmp = SELF(1, 2);
		SELF(1, 2) = SELF(2, 1);
		SELF(2, 1) = tmp;
		tmp = SELF(1, 3);
		SELF(1, 3) = SELF(3, 1);
		SELF(3, 1) = tmp;
		tmp = SELF(2, 3);
		SELF(2, 3) = SELF(3, 2);
		SELF(3, 2) = tmp;
	#endif
}

// getTransposed
inline Mat4 Mat4::getTransposed() const
{
	Mat4 m4;
	m4[0] = SELF[0];
	m4[1] = SELF[4];
	m4[2] = SELF[8];
	m4[3] = SELF[12];
	m4[4] = SELF[1];
	m4[5] = SELF[5];
	m4[6] = SELF[9];
	m4[7] = SELF[13];
	m4[8] = SELF[2];
	m4[9] = SELF[6];
	m4[10] = SELF[10];
	m4[11] = SELF[14];
	m4[12] = SELF[3];
	m4[13] = SELF[7];
	m4[14] = SELF[11];
	m4[15] = SELF[15];
	return m4;
}

// setRotationPart
inline void Mat4::setRotationPart(const Mat3& m3)
{
	SELF(0, 0) = m3(0, 0);
	SELF(0, 1) = m3(0, 1);
	SELF(0, 2) = m3(0, 2);
	SELF(1, 0) = m3(1, 0);
	SELF(1, 1) = m3(1, 1);
	SELF(1, 2) = m3(1, 2);
	SELF(2, 0) = m3(2, 0);
	SELF(2, 1) = m3(2, 1);
	SELF(2, 2) = m3(2, 2);
}

// getRotationPart
inline Mat3 Mat4::getRotationPart() const
{
	Mat3 m3;
	m3(0, 0) = SELF(0, 0);
	m3(0, 1) = SELF(0, 1);
	m3(0, 2) = SELF(0, 2);
	m3(1, 0) = SELF(1, 0);
	m3(1, 1) = SELF(1, 1);
	m3(1, 2) = SELF(1, 2);
	m3(2, 0) = SELF(2, 0);
	m3(2, 1) = SELF(2, 1);
	m3(2, 2) = SELF(2, 2);
	return m3;
}

// setTranslationPart
inline void Mat4::setTranslationPart(const Vec4& v)
{
	SELF(0, 3) = v.x();
	SELF(1, 3) = v.y();
	SELF(2, 3) = v.z();
	SELF(3, 3) = v.w();
}

// setTranslationPart
inline void Mat4::setTranslationPart(const Vec3& v)
{
	SELF(0, 3) = v.x();
	SELF(1, 3) = v.y();
	SELF(2, 3) = v.z();
}

// getTranslationPart
inline Vec3 Mat4::getTranslationPart() const
{
	return Vec3(SELF(0, 3), SELF(1, 3), SELF(2, 3));
}

// getIdentity
inline const Mat4& Mat4::getIdentity()
{
	static Mat4 ident(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
	return ident;
}

// getZero
inline const Mat4& Mat4::getZero()
{
	static Mat4 zero(0.0);
	return zero;
}

// Determinant
inline float Mat4::getDet() const
{
	return
	SELF(0, 3) * SELF(1, 2) * SELF(2, 1) * SELF(3, 0) - SELF(0, 2) * SELF(1, 3) * SELF(2, 1) * SELF(3, 0) -
	SELF(0, 3) * SELF(1, 1) * SELF(2, 2) * SELF(3, 0) + SELF(0, 1) * SELF(1, 3) * SELF(2, 2) * SELF(3, 0) +
	SELF(0, 2) * SELF(1, 1) * SELF(2, 3) * SELF(3, 0) - SELF(0, 1) * SELF(1, 2) * SELF(2, 3) * SELF(3, 0) -
	SELF(0, 3) * SELF(1, 2) * SELF(2, 0) * SELF(3, 1) + SELF(0, 2) * SELF(1, 3) * SELF(2, 0) * SELF(3, 1) +
	SELF(0, 3) * SELF(1, 0) * SELF(2, 2) * SELF(3, 1) - SELF(0, 0) * SELF(1, 3) * SELF(2, 2) * SELF(3, 1) -
	SELF(0, 2) * SELF(1, 0) * SELF(2, 3) * SELF(3, 1) + SELF(0, 0) * SELF(1, 2) * SELF(2, 3) * SELF(3, 1) +
	SELF(0, 3) * SELF(1, 1) * SELF(2, 0) * SELF(3, 2) - SELF(0, 1) * SELF(1, 3) * SELF(2, 0) * SELF(3, 2) -
	SELF(0, 3) * SELF(1, 0) * SELF(2, 1) * SELF(3, 2) + SELF(0, 0) * SELF(1, 3) * SELF(2, 1) * SELF(3, 2) +
	SELF(0, 1) * SELF(1, 0) * SELF(2, 3) * SELF(3, 2) - SELF(0, 0) * SELF(1, 1) * SELF(2, 3) * SELF(3, 2) -
	SELF(0, 2) * SELF(1, 1) * SELF(2, 0) * SELF(3, 3) + SELF(0, 1) * SELF(1, 2) * SELF(2, 0) * SELF(3, 3) +
	SELF(0, 2) * SELF(1, 0) * SELF(2, 1) * SELF(3, 3) - SELF(0, 0) * SELF(1, 2) * SELF(2, 1) * SELF(3, 3) -
	SELF(0, 1) * SELF(1, 0) * SELF(2, 2) * SELF(3, 3) + SELF(0, 0) * SELF(1, 1) * SELF(2, 2) * SELF(3, 3);
}

// getInverse
inline Mat4 Mat4::getInverse() const
{
	/// @todo test this
	#if !defined(MATH_INTEL_SIMD)
		Mat4 r(SELF);
		__m128 minor0, minor1, minor2, minor3;
		__m128 det, tmp1;

		// Transpose
		r.transpose();

		// Calc coeffs
		tmp1 = _mm_mul_ps(r.arrMm[2], r.arrMm[3]);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor0 = _mm_mul_ps(r.arrMm[1], tmp1);
		minor1 = _mm_mul_ps(r.arrMm[0], tmp1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor0 = _mm_sub_ps(_mm_mul_ps(r.arrMm[1], tmp1), minor0);
		minor1 = _mm_sub_ps(_mm_mul_ps(r.arrMm[0], tmp1), minor1);
		minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

		tmp1 = _mm_mul_ps(r.arrMm[1], r.arrMm[2]);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor0 = _mm_add_ps(_mm_mul_ps(r.arrMm[3], tmp1), minor0);
		minor3 = _mm_mul_ps(r.arrMm[0], tmp1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor0 = _mm_sub_ps(minor0, _mm_mul_ps(r.arrMm[3], tmp1));
		minor3 = _mm_sub_ps(_mm_mul_ps(r.arrMm[0], tmp1), minor3);
		minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

		tmp1 = _mm_mul_ps(_mm_shuffle_ps(r.arrMm[1], r.arrMm[1], 0x4E), r.arrMm[3]);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		r.arrMm[2] = _mm_shuffle_ps(r.arrMm[2], r.arrMm[2], 0x4E);
		minor0 = _mm_add_ps(_mm_mul_ps(r.arrMm[2], tmp1), minor0);
		minor2 = _mm_mul_ps(r.arrMm[0], tmp1);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor0 = _mm_sub_ps(minor0, _mm_mul_ps(r.arrMm[2], tmp1));
		minor2 = _mm_sub_ps(_mm_mul_ps(r.arrMm[0], tmp1), minor2);
		minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

		tmp1 = _mm_mul_ps(r.arrMm[0], r.arrMm[1]);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor2 = _mm_add_ps(_mm_mul_ps(r.arrMm[3], tmp1), minor2);
		minor3 = _mm_sub_ps(_mm_mul_ps(r.arrMm[2], tmp1), minor3);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor2 = _mm_sub_ps(_mm_mul_ps(r.arrMm[3], tmp1), minor2);
		minor3 = _mm_sub_ps(minor3, _mm_mul_ps(r.arrMm[2], tmp1));

		tmp1 = _mm_mul_ps(r.arrMm[0], r.arrMm[3]);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor1 = _mm_sub_ps(minor1, _mm_mul_ps(r.arrMm[2], tmp1));
		minor2 = _mm_add_ps(_mm_mul_ps(r.arrMm[1], tmp1), minor2);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor1 = _mm_add_ps(_mm_mul_ps(r.arrMm[2], tmp1), minor1);
		minor2 = _mm_sub_ps(minor2, _mm_mul_ps(r.arrMm[1], tmp1));

		tmp1 = _mm_mul_ps(r.arrMm[0], r.arrMm[2]);
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
		minor1 = _mm_add_ps(_mm_mul_ps(r.arrMm[3], tmp1), minor1);
		minor3 = _mm_sub_ps(minor3, _mm_mul_ps(r.arrMm[1], tmp1));
		tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
		minor1 = _mm_sub_ps(minor1, _mm_mul_ps(r.arrMm[3], tmp1));
		minor3 = _mm_add_ps(_mm_mul_ps(r.arrMm[1], tmp1), minor3);

		// 1 / det
		det = _mm_mul_ps(r.arrMm[0], minor0);
		det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
		det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
		tmp1 = _mm_rcp_ss(det);
		det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
		det = _mm_shuffle_ps(det, det, 0x00);

		// Mul and store
		minor0 = _mm_mul_ps(det, minor0);
		r.arrMm[0] = minor0;
		minor1 = _mm_mul_ps(det, minor1);
		r.arrMm[1] = minor1;
		minor2 = _mm_mul_ps(det, minor2);
		r.arrMm[2] = minor2;
		minor3 = _mm_mul_ps(det, minor3);
		r.arrMm[3] = minor3;

		return r;
	#else
		float tmp[12];
		float det;
		const Mat4& in = SELF;
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

		m4(2, 0) = tmp[0] * in(1, 3) + tmp[3] * in(2, 3) + tmp[4] * in(3, 3);
		m4(2, 0)-= tmp[1] * in(1, 3) + tmp[2] * in(2, 3) + tmp[5] * in(3, 3);
		m4(2, 1) = tmp[1] * in(0, 3) + tmp[6] * in(2, 3) + tmp[9] * in(3, 3);
		m4(2, 1)-= tmp[0] * in(0, 3) + tmp[7] * in(2, 3) + tmp[8] * in(3, 3);
		m4(2, 2) = tmp[2] * in(0, 3) + tmp[7] * in(1, 3) + tmp[10] * in(3, 3);
		m4(2, 2)-= tmp[3] * in(0, 3) + tmp[6] * in(1, 3) + tmp[11] * in(3, 3);
		m4(2, 3) = tmp[5] * in(0, 3) + tmp[8] * in(1, 3) + tmp[11] * in(2, 3);
		m4(2, 3)-= tmp[4] * in(0, 3) + tmp[9] * in(1, 3) + tmp[10] * in(2, 3);
		m4(3, 0) = tmp[2] * in(2, 2) + tmp[5] * in(3, 2) + tmp[1] * in(1, 2);
		m4(3, 0)-= tmp[4] * in(3, 2) + tmp[0] * in(1, 2) + tmp[3] * in(2, 2);
		m4(3, 1) = tmp[8] * in(3, 2) + tmp[0] * in(0, 2) + tmp[7] * in(2, 2);
		m4(3, 1)-= tmp[6] * in(2, 2) + tmp[9] * in(3, 2) + tmp[1] * in(0, 2);
		m4(3, 2) = tmp[6] * in(1, 2) + tmp[11] * in(3, 2) + tmp[3] * in(0, 2);
		m4(3, 2)-= tmp[10] * in(3, 2) + tmp[2] * in(0, 2) + tmp[7] * in(1, 2);
		m4(3, 3) = tmp[10] * in(2, 2) + tmp[4] * in(0, 2) + tmp[9] * in(1, 2);
		m4(3, 3)-= tmp[8] * in(1, 2) + tmp[11] * in(2, 2) + tmp[5] * in(0, 2);

		det = SELF(0, 0) * m4(0, 0) + SELF(1, 0) * m4(0, 1) + SELF(2, 0) * m4(0, 2) + SELF(3, 0) * m4(0, 3);

		ASSERT(!isZero(det)); // Cannot invert, det == 0
		det = 1.0 / det;
		m4 *= det;
		return m4;
	#endif
}

// invert
inline void Mat4::invert()
{
	SELF = getInverse();
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
inline Mat4 Mat4::lerp(const Mat4& b, float t) const
{
	return (SELF * (1.0 - t)) + (b * t);
}

// setIdentity
inline void Mat4::setIdentity()
{
	SELF = getIdentity();
}

// combineTransformations
inline Mat4 Mat4::combineTransformations(const Mat4& m0, const Mat4& m1)
{
	/*
	The clean code is:
	Mat3 rot = m0.getRotationPart() * m1.getRotationPart();  // combine the rotations
	Vec3 tra = (m1.getTranslationPart()).Transformed(m0.getTranslationPart(), m0.getRotationPart(), 1.0);
	return Mat4(tra, rot);
	and the optimized:
	*/
	ASSERT(isZero(m0(3, 0) + m0(3, 1) + m0(3, 2) + m0(3, 3)-1.0) &&
	       isZero(m1(3, 0) + m1(3, 1) + m1(3, 2) + m1(3, 3)-1.0)); // one of the 2 mat4 doesnt represent transformation

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

	m4(0, 3) = m0(0, 0) * m1(0, 3) + m0(0, 1) * m1(1, 3) + m0(0, 2) * m1(2, 3) + m0(0, 3);
	m4(1, 3) = m0(1, 0) * m1(0, 3) + m0(1, 1) * m1(1, 3) + m0(1, 2) * m1(2, 3) + m0(1, 3);
	m4(2, 3) = m0(2, 0) * m1(0, 3) + m0(2, 1) * m1(1, 3) + m0(2, 2) * m1(2, 3) + m0(2, 3);

	m4(3, 0) = m4(3, 1) = m4(3, 2) = 0.0;
	m4(3, 3) = 1.0;

	return m4;
}


//======================================================================================================================
// Print                                                                                                               =
//======================================================================================================================
inline std::ostream& operator<<(std::ostream& s, const Mat4& m)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
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
