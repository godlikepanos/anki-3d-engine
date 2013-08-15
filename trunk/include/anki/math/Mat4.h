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

	explicit TMat4(const T arr[])
	{
		for(U i = 0; i < 16; i++)
		{
			arr1[i] = arr[i];
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

	explicit TMat4(const TVec3<T>& transl, const TMat3<T>& rot)
	{
		setRotationPart(rot);
		setTranslationPart(transl);
		TMat4& m = *this;
		m(3, 0) = m(3, 1) = m(3, 2) = 0.0;
		m(3, 3) = 1.0;
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

		TMat4& m = *this;
		m(3, 0) = m(3, 1) = m(3, 2) = 0.0;
		m(3, 3) = 1.0;
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
		const TMat4& m = *this;
		for(U i = 0; i < 4; i++)
		{
			for(U j = 0; j < 4; j++)
			{
				c(i, j) = m(i, 0) * b(0, j) + m(i, 1) * b(1, j) 
					+ m(i, 2) * b(2, j) + m(i, 3) * b(3, j);
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
			if(!isZero<T>(arr1[i] - b.arr1[i]))
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
			if(!isZero(arr1[i] - b.arr1[i]))
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
			c.arr1[i] = arr1[i] + f;
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
			c.arr1[i] = arr1[i] - f;
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
			c.arr1[i] = arr1[i] * f;
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
		const TMat4& m = *this;

		out.x() = m(0, 0) * v4.x() + m(0, 1) * v4.y() 
			+ m(0, 2) * v4.z() + m(0, 3) * v4.w();

		out.y() = m(1, 0) * v4.x() + m(1, 1) * v4.y() 
			+ m(1, 2) * v4.z() + m(1, 3) * v4.w();

		out.z() = m(2, 0) * v4.x() + m(2, 1) * v4.y() 
			+ m(2, 2) * v4.z() + m(2, 3) * v4.w();

		out.w() = m(3, 0) * v4.x() + m(3, 1) * v4.y() 
			+ m(3, 2) * v4.z() + m(3, 3) * v4.w();

		return out;
	}
	/// @}

	/// @name Other
	/// @{
	void setRow(const U i, const TVec4<T>& v)
	{
		TMat4& m = *this;
		m(i, 0) = v.x();
		m(i, 1) = v.y();
		m(i, 2) = v.z();
		m(i, 3) = v.w();
	}

	void setRows(const TVec4<T>& a, const TVec4<T>& b, const TVec4<T>& c,
		const TVec4<T>& d)
	{
		setRow(0, a);
		setRow(1, b);
		setRow(2, c);
		setRow(3, d);
	}

	TVec4<T> getRow(const U i) const
	{
		const TMat4& m = *this;
		return TVec4<T>(m(i, 0), m(i, 1), m(i, 2), m(i, 3));
	}

	void getRows(TVec4<T>& a, TVec4<T>& b, TVec4<T>& c, TVec4<T>& d) const
	{
		a = getRow(0);
		b = getRow(1);
		c = getRow(2);
		d = getRow(3);
	}

	void setColumn(const U i, const TVec4<T>& v)
	{
		TMat4& m = *this;
		m(0, i) = v.x();
		m(1, i) = v.y();
		m(2, i) = v.z();
		m(3, i) = v.w();
	}

	void setColumns(const TVec4<T>& a, const TVec4<T>& b, const TVec4<T>& c,
		const TVec4<T>& d)
	{
		setColumn(0, a);
		setColumn(1, b);
		setColumn(2, c);
		setColumn(3, d);
	}

	TVec4<T> getColumn(const U i) const
	{
		const TMat4& m = *this;
		return TVec4<T>(m(0, i), m(1, i), m(2, i), m(3, i));
	}

	void getColumns(TVec4<T>& a, TVec4<T>& b, TVec4<T>& c, TVec4<T>& d) const
	{
		a = getColumn(0);
		b = getColumn(1);
		c = getColumn(2);
		d = getColumn(3);
	}

	void setRotationPart(const TMat3<T>& m3)
	{
		TMat4& m = *this;
		m(0, 0) = m3(0, 0);
		m(0, 1) = m3(0, 1);
		m(0, 2) = m3(0, 2);
		m(1, 0) = m3(1, 0);
		m(1, 1) = m3(1, 1);
		m(1, 2) = m3(1, 2);
		m(2, 0) = m3(2, 0);
		m(2, 1) = m3(2, 1);
		m(2, 2) = m3(2, 2);
	}

	TMat3<T> getRotationPart() const
	{
		const TMat4& m = *this;
		TMat3<T> m3;
		m3(0, 0) = m(0, 0);
		m3(0, 1) = m(0, 1);
		m3(0, 2) = m(0, 2);
		m3(1, 0) = m(1, 0);
		m3(1, 1) = m(1, 1);
		m3(1, 2) = m(1, 2);
		m3(2, 0) = m(2, 0);
		m3(2, 1) = m(2, 1);
		m3(2, 2) = m(2, 2);
		return m3;
	}

	void setTranslationPart(const TVec4<T>& v)
	{
		TMat4& m = *this;
		m(0, 3) = v.x();
		m(1, 3) = v.y();
		m(2, 3) = v.z();
		m(3, 3) = v.w();
	}

	void setTranslationPart(const TVec3<T>& v)
	{
		TMat4& m = *this;
		m(0, 3) = v.x();
		m(1, 3) = v.y();
		m(2, 3) = v.z();
	}

	TVec3<T> getTranslationPart() const
	{
		const TMat4& m = *this;
		return TVec3<T>(m(0, 3), m(1, 3), m(2, 3));
	}

	void transpose()
	{
		TMat4& m = *this;
		T tmp = m(0, 1);
		m(0, 1) = m(1, 0);
		m(1, 0) = tmp;
		tmp = m(0, 2);
		m(0, 2) = m(2, 0);
		m(2, 0) = tmp;
		tmp = m(0, 3);
		m(0, 3) = m(3, 0);
		m(3, 0) = tmp;
		tmp = m(1, 2);
		m(1, 2) = m(2, 1);
		m(2, 1) = tmp;
		tmp = m(1, 3);
		m(1, 3) = m(3, 1);
		m(3, 1) = tmp;
		tmp = m(2, 3);
		m(2, 3) = m(3, 2);
		m(3, 2) = tmp;
	}

	TMat4 getTransposed() const
	{
		const TMat4& m = *this;
		TMat4 out;
		for(U i = 0; i < 4; i++)
		{
			for(U j = 0; j < 4; j++)
			{
				out(i, j) = m(j, i);
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

	std::string toString() const
	{
		const TMat4& m = *this;
		std::string s;
		for(U i = 0; i < 4; i++)
		{
			for(U j = 0; j < 4; j++)
			{
				s += std::to_string(m(i, j)) + " ";
			}
			s += "\n";
		}
		return s;
	}
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		Array<T, 16> arr1;
		Array<Array<T, 4>, 4> arr2;
		T carr1[16]; ///< For easier debugging with gdb
		T carr2[4][4]; ///< For easier debugging with gdb
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

/// F32 4x4 matrix
typedef TMat4<F32> Mat4;
static_assert(sizeof(Mat4) == sizeof(F32) * 4 * 4, "Incorrect size");
/// @}

} // end namespace anki

#include "anki/math/Mat4.inl.h"

#endif
