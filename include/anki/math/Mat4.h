#ifndef ANKI_MATH_MAT4_H
#define ANKI_MATH_MAT4_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Template struct that gives the type of the TVec4 SIMD
template<typename T>
struct TMat4Simd
{
	typedef Array<T, 16> Type;
};

#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE
// Specialize for F32
template<>
struct TMat4Simd<F32>
{
	typedef Array<__m128, 4> Type;
};
#endif

/// 4x4 Matrix. Used mainly for transformations but not necessarily. Its
/// row major. SSE optimized
template<typename T>
ANKI_ATTRIBUTE_ALIGNED(class, 16) TMat4
{
public:
	typedef typename TMat4Simd<T>::Type Simd;

	/// @name Constructors
	/// @{
	explicit TMat4()
	{}

	explicit TMat4(const T f)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] = f;
		}
	}

	explicit TMat4(const T m00, const T m01, const T m02,
		const T m03, const T m10, const T m11,
		const T m12, const T m13, const T m20,
		const T m21, const T m22, const T m23,
		const T m30, const T m31, const T m32,
		const T m33)
	{
		arr2[0][0] = m00;
		arr2[0][1] = m01;
		arr2[0][2] = m02;
		arr2[0][3] = m03;
		arr2[1][0] = m10;
		arr2[1][1] = m11;
		arr2[1][2] = m12;
		arr2[1][3] = m13;
		arr2[2][0] = m20;
		arr2[2][1] = m21;
		arr2[2][2] = m22;
		arr2[2][3] = m23;
		arr2[3][0] = m30;
		arr2[3][1] = m31;
		arr2[3][2] = m32;
		arr2[3][3] = m33;
	}

	explicit TMat4(const T arr_[])
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] = arr_[i];
		}	
	}

	TMat4(const TMat4& b)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] = b.arr1[i];
		}
	}

	explicit TMat4(const TMat3<T>& m3)
	{
		arr2[0][0] = m3(0, 0);
		arr2[0][1] = m3(0, 1);
		arr2[0][2] = m3(0, 2);
		arr2[0][3] = 0.0;
		arr2[1][0] = m3(1, 0);
		arr2[1][1] = m3(1, 1);
		arr2[1][2] = m3(1, 2);
		arr2[1][3] = 0.0;
		arr2[2][0] = m3(2, 0);
		arr2[2][1] = m3(2, 1);
		arr2[2][2] = m3(2, 2);
		arr2[2][3] = 0.0;
		arr2[3][0] = 0.0;
		arr2[3][1] = 0.0;
		arr2[3][2] = 0.0;
		arr2[3][3] = 1.0;
	}

	explicit TMat4(const TVec3<T>& v)
	{
		arr2[0][0] = 1.0;
		arr2[0][1] = 0.0;
		arr2[0][2] = 0.0;
		arr2[0][3] = v.x();
		arr2[1][0] = 0.0;
		arr2[1][1] = 1.0;
		arr2[1][2] = 0.0;
		arr2[1][3] = v.y();
		arr2[2][0] = 0.0;
		arr2[2][1] = 0.0;
		arr2[2][2] = 1.0;
		arr2[2][3] = v.z();
		arr2[3][0] = 0.0;
		arr2[3][1] = 0.0;
		arr2[3][2] = 0.0;
		arr2[3][3] = 1.0;
	}

	explicit TMat4(const TVec4<T>& v)
	{
		arr2[0][0] = 1.0;
		arr2[0][1] = 0.0;
		arr2[0][2] = 0.0;
		arr2[0][3] = v.x();
		arr2[1][0] = 0.0;
		arr2[1][1] = 1.0;
		arr2[1][2] = 0.0;
		arr2[1][3] = v.y();
		arr2[2][0] = 0.0;
		arr2[2][1] = 0.0;
		arr2[2][2] = 1.0;
		arr2[2][3] = v.z();
		arr2[3][0] = 0.0;
		arr2[3][1] = 0.0;
		arr2[3][2] = 0.0;
		arr2[3][3] = v.w();
	}

	explicit TMat4(const TVec3<T>& transl, const TMat3<T>& rot)
	{
		setRotationPart(rot);
		setTranslationPart(transl);
		arr2[3][0] = arr2[3][1] = arr2[3][2] = 0.0;
		arr2[3][3] = 1.0;
	}

	explicit TMat4(const TVec3<T>& transl, const TMat3<T>& rot, const T scale)
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

		arr2[3][0] = arr2[3][1] = arr2[3][2] = 0.0;
		arr2[3][3] = 1.0;
	}

	explicit TMat4(const TTransform<T>& t)
	{
		(*this) = TMat4(t.getOrigin(), t.getRotation(), t.getScale());
	}
	/// @}

	/// @name Accessors
	/// @{
	T& operator()(const U i, const U j)
	{
		return arr2[i][j];
	}

	const T& operator()(const U i, const U j) const
	{
		return arr2[i][j];
	}

	T& operator[](const U i)
	{
		return arr1[i];
	}

	const T& operator[](const U i) const
	{
		return arr1[i];
	}

	Simd& getSimd(const U i)
	{
		return simd[i];
	}

	const Simd& getSimd(const U i) const
	{
		return simd[i];
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TMat4& operator=(const TMat4& b)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] = b.arr1[i];
		}
	}

	TMat4 operator+(const TMat4& b) const
	{
		TMat4<T> c;
		for(U i = 0; i < 16; i++)
		{
			c.arr1[i] = arr1[i] + b.arr1[i];
		}
		return c;
	}

	TMat4& operator+=(const TMat4& b)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] += b.arr1[i];
		}
		return (*this);
	}

	TMat4 operator-(const TMat4& b) const
	{
		TMat4<T> c;
		for(U i = 0; i < 16; i++)
		{
			c.arr1[i] = arr1[i] - b.arr1[i];
		}
		return c;
	}

	TMat4& operator-=(const TMat4& b)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] -= b.arr1[i];
		}
		return (*this);
	}

	/// @note 64 muls, 48 adds
	TMat4 operator*(const TMat4& b) const
	{
		TMat4<T> c;
		for(U i = 0; i < 4; i++)
		{
			for(U j = 0; j < 4; j++)
			{
				c(i, j) = arr2[i][0] * b(0, j) + arr2[i][1] * b(1, j) 
					+ arr2[i][2] * b(2, j) + arr2[i][3] * b(3, j);
			}
		}
		return c;
	}

	TMat4& operator*=(const TMat4& b)
	{
		(*this) = (*this) * b;
		return (*this);
	}

	Bool operator==(const TMat4& b) const
	{
		for(U i = 0; i < 16; i++)
		{
			if(!isZero<T>(arr1[i] - b[i]))
			{
				return false;
			}
		}
		return true;
	}

	Bool operator!=(const TMat4& b) const
	{
		for(U i = 0; i < 16; i++)
		{
			if(!isZero(arr1[i] - b[i]))
			{
				return true;
			}
		}
		return false;
	}
	/// @}

	/// @name Operators with T
	/// @{
	TMat4 operator+(const T f) const
	{
		TMat4 c;
		for(U i = 0; i < 16; i++)
		{
			c[i] = arr1[i] + f;
		}
		return c;
	}

	TMat4& operator+=(const T f)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] += f;
		}
	}

	TMat4 operator-(const T f) const
	{
		TMat4 c;
		for(U i = 0; i < 16; i++)
		{
			c[i] = arr1[i] - f;
		}
		return c;
	}

	TMat4& operator-=(const T f)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] -= f;
		}
	}

	TMat4 operator*(const T f) const
	{
		TMat4 c;
		for(U i = 0; i < 16; i++)
		{
			c[i] = arr1[i] * f;
		}
		return c;
	}

	TMat4& operator*=(const T f)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] *= f;
		}
	}

	TMat4 operator/(const T f) const
	{
		TMat4 c;
		for(U i = 0; i < 16; i++)
		{
			c[i] = arr1[i] / f;
		}
		return c;
	}

	TMat4& operator/=(const T f)
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] /= f;
		}
	}
	/// @}

	/// @name Operators with other types
	/// @{

	/// 16 muls, 12 adds
	TVec4<T> operator*(const TVec4<T>& v4) const
	{
		TVec4<T> out;

		out.x() = arr2[0][0] * v4.x() + arr2[0][1] * v4.y() 
			+ arr2[0][2] * v4.z() + arr2[0][3] * v4.w();

		out.y() = arr2[1][0] * v4.x() + arr2[1][1] * v4.y() 
			+ arr2[1][2] * v4.z() + arr2[1][3] * v4.w();

		out.z() = arr2[2][0] * v4.x() + arr2[2][1] * v4.y() 
			+ arr2[2][2] * v4.z() + arr2[2][3] * v4.w();

		out.w() = arr2[3][0] * v4.x() + arr2[3][1] * v4.y() 
			+ arr2[3][2] * v4.z() + arr2[3][3] * v4.w();

		return out;
	}
	/// @}

	/// @name Other
	/// @{
	void setRows(const TVec4<T>& a, const TVec4<T>& b, const TVec4<T>& c,
		const TVec4<T>& d)
	{
		arr2[0][0] = a.x();
		arr2[0][1] = a.y();
		arr2[0][2] = a.z();
		arr2[0][3] = a.w();
		arr2[1][0] = b.x();
		arr2[1][1] = b.y();
		arr2[1][2] = b.z();
		arr2[1][3] = b.w();
		arr2[2][0] = c.x();
		arr2[2][1] = c.y();
		arr2[2][2] = c.z();
		arr2[2][3] = c.w();
		arr2[3][0] = d.x();
		arr2[3][1] = d.y();
		arr2[3][2] = d.z();
		arr2[3][3] = d.w();
	}

	void setRow(const U i, const TVec4<T>& v)
	{
		arr2[i][0] = v.x();
		arr2[i][1] = v.y();
		arr2[i][2] = v.z();
		arr2[i][3] = v.w();
	}

	TVec4<T> getRow(const U i) const
	{
		return TVec4<T>(arr2[i][0], arr2[i][1], arr2[i][2], arr2[i][3]);
	}

	void setColumns(const TVec4<T>& a, const TVec4<T>& b, const TVec4<T>& c,
		const TVec4<T>& d)
	{
		arr2[0][0] = a.x();
		arr2[1][0] = a.y();
		arr2[2][0] = a.z();
		arr2[3][0] = a.w();
		arr2[0][1] = b.x();
		arr2[1][1] = b.y();
		arr2[2][1] = b.z();
		arr2[3][1] = b.w();
		arr2[0][2] = c.x();
		arr2[1][2] = c.y();
		arr2[2][2] = c.z();
		arr2[3][2] = c.w();
		arr2[0][3] = d.x();
		arr2[1][3] = d.y();
		arr2[2][3] = d.z();
		arr2[3][3] = d.w();
	}

	void setColumn(const U i, const TVec4<T>& v)
	{
		arr2[0][i] = v.x();
		arr2[1][i] = v.y();
		arr2[2][i] = v.z();
		arr2[3][i] = v.w();
	}

	TVec4<T> getColumn(const U i) const
	{
		return TVec4<T>(arr2[0][i], arr2[1][i], arr2[2][i], arr2[3][i]);
	}

	void setRotationPart(const TMat3<T>& m3)
	{
		arr2[0][0] = m3(0, 0);
		arr2[0][1] = m3(0, 1);
		arr2[0][2] = m3(0, 2);
		arr2[1][0] = m3(1, 0);
		arr2[1][1] = m3(1, 1);
		arr2[1][2] = m3(1, 2);
		arr2[2][0] = m3(2, 0);
		arr2[2][1] = m3(2, 1);
		arr2[2][2] = m3(2, 2);
	}

	TMat3<T> getRotationPart() const
	{
		TMat3<T> m3;
		m3(0, 0) = arr2[0][0];
		m3(0, 1) = arr2[0][1];
		m3(0, 2) = arr2[0][2];
		m3(1, 0) = arr2[1][0];
		m3(1, 1) = arr2[1][1];
		m3(1, 2) = arr2[1][2];
		m3(2, 0) = arr2[2][0];
		m3(2, 1) = arr2[2][1];
		m3(2, 2) = arr2[2][2];
		return m3;
	}

	void setTranslationPart(const TVec4<T>& v)
	{
		arr2[0][3] = v.x();
		arr2[1][3] = v.y();
		arr2[2][3] = v.z();
		arr2[3][3] = v.w();
	}

	void setTranslationPart(const TVec3<T>& v)
	{
		arr2[0][3] = v.x();
		arr2[1][3] = v.y();
		arr2[2][3] = v.z();
		arr2[3][3] = v.w();
	}

	TVec3<T> getTranslationPart() const
	{
		return TVec3<T>(arr2[0][3], arr2[1][3], arr2[2][3]);
	}

	void transpose()
	{
		T tmp = arr2[0][1];
		arr2[0][1] = arr2[1][0];
		arr2[1][0] = tmp;
		tmp = arr2[0][2];
		arr2[0][2] = arr2[2][0];
		arr2[2][0] = tmp;
		tmp = arr2[0][3];
		arr2[0][3] = arr2[3][0];
		arr2[3][0] = tmp;
		tmp = arr2[1][2];
		arr2[1][2] = arr2[2][1];
		arr2[2][1] = tmp;
		tmp = arr2[1][3];
		arr2[1][3] = arr2[3][1];
		arr2[3][1] = tmp;
		tmp = arr2[2][3];
		arr2[2][3] = arr2[3][2];
		arr2[3][2] = tmp;
	}

	TMat4 getTransposed() const
	{
		TMat4 out;
		for(U i = 0; i < 4; i++)
		{
			for(U j = 0; j < 4; j++)
			{
				out(i, j) = arr2[j][i];
			}
		}
		return out;
	}

	T getDet() const
	{
		const TMat4& t = *this;
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

		T det = in(0, 0) * m4(0, 0) + in(1, 0) * m4(0, 1) 
			+ in(2, 0) * m4(0, 2) + in(3, 0) * m4(0, 3);

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

	/// If we suppose this matrix represents a transformation, return the
	/// inverted transformation
	TMat4 getInverseTransformation() const
	{
		TMat3<T> invertedRot = getRotationPart().getTransposed();
		TVec3<T> invertedTsl = getTranslationPart();
		invertedTsl = -(invertedRot * invertedTsl);
		return TMat4(invertedTsl, invertedRot);
	}

	TMat4 lerp(const TMat4& b, T t) const
	{
		return ((*this) * (1.0 - t)) + (b * t);
	}

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TMat4& getIdentity()
	{
		static const TMat4 ident(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 
			0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
		return ident;
	}

	static const TMat4& getZero()
	{
		static const TMat4 zero(0.0);
		return zero;
	}

	/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching
	/// the 4rth row and allot faster
	static TMat4 combineTransformations(const TMat4& m0, const TMat4& m1)
	{
		// See the clean code in < r664

		// one of the 2 mat4 doesnt represent transformation
		ANKI_ASSERT(isZero<T>(m0(3, 0) + m0(3, 1) + m0(3, 2) + m0(3, 3) - 1.0)
			&& isZero<T>(m1(3, 0) + m1(3, 1) + m1(3, 2) + m1(3, 3) - 1.0));

		TMat4 m4;

		m4(0, 0) = 
			m0(0, 0) * m1(0, 0) + m0(0, 1) * m1(1, 0) + m0(0, 2) * m1(2, 0);
		m4(0, 1) = 
			m0(0, 0) * m1(0, 1) + m0(0, 1) * m1(1, 1) + m0(0, 2) * m1(2, 1);
		m4(0, 2) = 
			m0(0, 0) * m1(0, 2) + m0(0, 1) * m1(1, 2) + m0(0, 2) * m1(2, 2);
		m4(1, 0) = 
			m0(1, 0) * m1(0, 0) + m0(1, 1) * m1(1, 0) + m0(1, 2) * m1(2, 0);
		m4(1, 1) = 
			m0(1, 0) * m1(0, 1) + m0(1, 1) * m1(1, 1) + m0(1, 2) * m1(2, 1);
		m4(1, 2) = 
			m0(1, 0) * m1(0, 2) + m0(1, 1) * m1(1, 2) + m0(1, 2) * m1(2, 2);
		m4(2, 0) =
			m0(2, 0) * m1(0, 0) + m0(2, 1) * m1(1, 0) + m0(2, 2) * m1(2, 0);
		m4(2, 1) = 
			m0(2, 0) * m1(0, 1) + m0(2, 1) * m1(1, 1) + m0(2, 2) * m1(2, 1);
		m4(2, 2) =
			m0(2, 0) * m1(0, 2) + m0(2, 1) * m1(1, 2) + m0(2, 2) * m1(2, 2);

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

	std::string toString() const;
	/// @}

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

private:
	/// @name Data
	/// @{
	union
	{
		Array<T, 16> arr1;
		Array<Array<T, 4>, 4> arr2;
		T carr1[16]; ///< For gdb
		T carr2[4][4]; ///< For gdb
		Simd simd;
	};
	/// @}
};

#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE

// Forward declare specializations

template<>
TMat4<F32>::TMat4(const TMat4<F32>& b);

template<>
TMat4<F32>::TMat4(const F32 f);

template<>
TMat4<F32>& TMat4<F32>::operator=(const TMat4<F32>& b);

template<>
TMat4<F32> TMat4<F32>::operator+(const TMat4<F32>& b) const;

template<>
TMat4<F32>& TMat4<F32>::operator+=(const TMat4<F32>& b);

template<>
TMat4<F32> TMat4<F32>::operator-(const TMat4<F32>& b) const;

template<>
TMat4<F32>& TMat4<F32>::operator-=(const TMat4<F32>& b);

template<>
TMat4<F32> TMat4<F32>::operator*(const TMat4<F32>& b) const;

template<>
TMat4<F32> TMat4<F32>::operator+(const F32 f) const;

template<>
TMat4<F32>& TMat4<F32>::operator+=(const F32 f);

template<>
TMat4<F32> TMat4<F32>::operator-(const F32 f) const;

template<>
TMat4<F32>& TMat4<F32>::operator-=(const F32 f);

template<>
TMat4<F32> TMat4<F32>::operator*(const F32 f) const;

template<>
TMat4<F32>& TMat4<F32>::operator*=(const F32 f);

template<>
TMat4<F32> TMat4<F32>::operator/(const F32 f) const;

template<>
TMat4<F32>& TMat4<F32>::operator/=(const F32 f);

template<>
TVec4<F32> TMat4<F32>::operator*(const TVec4<F32>& b) const;

template<>
void TMat4<F32>::setRows(const TVec4<F32>& a, const TVec4<F32>& b, 
	const TVec4<F32>& c, const TVec4<F32>& d);

template<>
void TMat4<F32>::setRow(const U i, const TVec4<F32>& v);

template<>
void TMat4<F32>::transpose();

template<>
TMat4<F32> operator-(const F32 f, const TMat4<F32>& m4);

template<>
TMat4<F32> operator/(const F32 f, const TMat4<F32>& m4);

#endif


/// 4x4 Matrix. Used mainly for transformations but not necessarily. Its
/// row major. SSE optimized
ANKI_ATTRIBUTE_ALIGNED(class, 16) Mat4
{
public:
	/// @name Constructors
	/// @{
	explicit Mat4() {}
	explicit Mat4(const F32 f);
	explicit Mat4(const F32 m00, const F32 m01, const F32 m02,
		const F32 m03, const F32 m10, const F32 m11,
		const F32 m12, const F32 m13, const F32 m20,
		const F32 m21, const F32 m22, const F32 m23,
		const F32 m30, const F32 m31, const F32 m32,
		const F32 m33);
	explicit Mat4(const F32 arr[]);
	Mat4(const Mat4& b);
	explicit Mat4(const Mat3& m3);
	explicit Mat4(const Vec3& v);
	explicit Mat4(const Vec4& v);
	explicit Mat4(const Vec3& transl, const Mat3& rot);
	explicit Mat4(const Vec3& transl, const Mat3& rot, const F32 scale);
	explicit Mat4(const Transform& t);
	/// @}

	/// @name Accessors
	/// @{
	F32& operator()(const U i, const U j);
	const F32& operator()(const U i, const U j) const;
	F32& operator[](const U i);
	const F32& operator[](const U i) const;
#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE
	__m128& getMm(const U i);
	const __m128& getMm(const U i) const;
#endif
	/// @}

	/// @name Operators with same type
	/// @{
	Mat4& operator=(const Mat4& b);
	Mat4 operator+(const Mat4& b) const;
	Mat4& operator+=(const Mat4& b);
	Mat4 operator-(const Mat4& b) const;
	Mat4& operator-=(const Mat4& b);
	Mat4 operator*(const Mat4& b) const; ///< 64 muls, 48 adds
	Mat4& operator*=(const Mat4& b);
	Mat4 operator/(const Mat4& b) const;
	Mat4& operator/=(const Mat4& b);
	Bool operator==(const Mat4& b) const;
	Bool operator!=(const Mat4& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Mat4 operator+(const F32 f) const;
	Mat4& operator+=(const F32 f);
	Mat4 operator-(const F32 f) const;
	Mat4& operator-=(const F32 f);
	Mat4 operator*(const F32 f) const;
	Mat4& operator*=(const F32 f);
	Mat4 operator/(const F32 f) const;
	Mat4& operator/=(const F32 f);
	/// @}

	/// @name Operators with other types
	/// @{
	Vec4 operator*(const Vec4& v4) const; ///< 16 muls, 12 adds
	/// @}

	/// @name Other
	/// @{
	void setRows(const Vec4& a, const Vec4& b, const Vec4& c,
		const Vec4& d);
	void setRow(const U i, const Vec4& v);
	Vec4 getRow(const U i) const;
	void setColumns(const Vec4& a, const Vec4& b, const Vec4& c,
		const Vec4& d);
	void setColumn(const U i, const Vec4& v);
	Vec4 getColumn(const U i) const;
	void setRotationPart(const Mat3& m3);
	void setTranslationPart(const Vec4& v4);
	Mat3 getRotationPart() const;
	void setTranslationPart(const Vec3& v3);
	Vec3 getTranslationPart() const;
	void transpose();
	Mat4 getTransposed() const;
	F32 getDet() const;
	Mat4 getInverse() const; ///< Invert using Cramer's rule
	void invert(); ///< See getInverse
	/// If we suppose this matrix represents a transformation, return the
	/// inverted transformation
	Mat4 getInverseTransformation() const;
	Mat4 lerp(const Mat4& b, F32 t) const;
	void setIdentity();
	/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching
	/// the 4rth row and allot faster
	static Mat4 combineTransformations(const Mat4& m0, const Mat4& m1);
	static const Mat4& getIdentity();
	static const Mat4& getZero();
	/// @}

	/// @name Friends
	/// @{
	friend Mat4 operator+(const F32 f, const Mat4& m4);
	friend Mat4 operator-(const F32 f, const Mat4& m4);
	friend Mat4 operator*(const F32 f, const Mat4& m4);
	friend Mat4 operator/(const F32 f, const Mat4& m4);
	friend std::ostream& operator<<(std::ostream& s, const Mat4& m);
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		Array<F32, 16> arr1;
		Array<Array<F32, 4>, 4> arr2;
		F32 carr1[16]; ///< For gdb
		F32 carr2[4][4]; ///< For gdb
#if ANKI_MATH_SIMD == ANKI_MATH_SIMD_SSE
		Array<__m128, 4> arrMm;
#endif
	};
	/// @}
};
/// @}

static_assert(sizeof(Mat4) == sizeof(F32) * 4 * 4, "Incorrect size");

} // end namespace

#include "anki/math/Mat4.inl.h"

#endif
