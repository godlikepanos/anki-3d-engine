// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/Common.h>
#include <anki/math/Vec.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Matrix type.
/// @tparam T The scalar type. Eg float.
/// @tparam J The number of rows.
/// @tparam I The number of columns.
template<typename T, U J, U I>
class alignas(MathSimd<T, I>::ALIGNMENT) TMat
{
public:
	using Scalar = T;
	using Simd = typename MathSimd<T, I>::Type;
	using SimdArray = Array<Simd, J>;
	using RowVec = TVec<T, I>;
	using ColumnVec = TVec<T, J>;

	static constexpr U ROW_SIZE = J; ///< Number of rows
	static constexpr U COLUMN_SIZE = I; ///< Number of columns
	static constexpr U SIZE = J * I; ///< Number of total elements
	static constexpr Bool HAS_SIMD = I == 4 && std::is_same<T, F32>::value && ANKI_SIMD_SSE;
	static constexpr Bool HAS_MAT4_SIMD = J == 4 && I == 4 && std::is_same<T, F32>::value && ANKI_SIMD_SSE;
	static constexpr Bool HAS_MAT3X4_SIMD = J == 3 && I == 4 && std::is_same<T, F32>::value && ANKI_SIMD_SSE;

	/// @name Constructors
	/// @{
	TMat()
	{
	}

	/// Copy.
	TMat(ANKI_ENABLE_ARG(const TMat&, !HAS_SIMD) b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] = b.m_arr1[i];
		}
	}

	/// Copy.
	TMat(ANKI_ENABLE_ARG(const TMat&, HAS_SIMD) b)
	{
		for(U i = 0; i < J; i++)
		{
			m_simd[i] = b.m_simd[i];
		}
	}

	ANKI_ENABLE_METHOD(!HAS_SIMD)
	explicit TMat(const T f)
	{
		for(T& x : m_arr1)
		{
			x = f;
		}
	}

	ANKI_ENABLE_METHOD(HAS_SIMD)
	explicit TMat(const T f)
	{
		for(U i = 0; i < J; i++)
		{
			m_simd[i] = _mm_set1_ps(f);
		}
	}

	explicit TMat(const T arr[])
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] = arr[i];
		}
	}

	// 3x3 specific constructors

	ANKI_ENABLE_METHOD(J == 3 && I == 3)
	TMat(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22)
	{
		auto& m = *this;
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

	ANKI_ENABLE_METHOD(J == 3 && I == 3)
	explicit TMat(const TQuat<T>& q)
	{
		setRotationPart(q);
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 3)
	explicit TMat(const TEuler<T>& e)
	{
		setRotationPart(e);
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 3)
	explicit TMat(const TAxisang<T>& axisang)
	{
		setRotationPart(axisang);
	}

	// 4x4 specific constructors

	ANKI_ENABLE_METHOD(J == 4 && I == 4)
	TMat(T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13, T m20, T m21, T m22, T m23, T m30, T m31, T m32, T m33)
	{
		auto& m = *this;
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

	ANKI_ENABLE_METHOD(J == 4 && I == 4)
	TMat(const TVec<T, 4>& translation, const TMat<T, 3, 3>& rotation, const T scale = T(1))
	{
		if(isZero<T>(scale - T(1)))
		{
			setRotationPart(rotation);
		}
		else
		{
			setRotationPart(rotation * scale);
		}

		setTranslationPart(translation);

		auto& m = *this;
		m(3, 0) = m(3, 1) = m(3, 2) = T(0);
	}

	ANKI_ENABLE_METHOD(J == 4 && I == 4)
	explicit TMat(const TTransform<T>& t)
		: TMat(t.getOrigin().xyz1(), t.getRotation().getRotationPart(), t.getScale())
	{
	}

	// 3x4 specific constructors

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	TMat(T m00, T m01, T m02, T m03, T m10, T m11, T m12, T m13, T m20, T m21, T m22, T m23)
	{
		auto& m = *this;
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
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	explicit TMat(const TMat<T, 4, 4>& m4)
	{
		auto& m = *this;
		m(0, 0) = m4(0, 0);
		m(0, 1) = m4(0, 1);
		m(0, 2) = m4(0, 2);
		m(0, 3) = m4(0, 3);
		m(1, 0) = m4(1, 0);
		m(1, 1) = m4(1, 1);
		m(1, 2) = m4(1, 2);
		m(1, 3) = m4(1, 3);
		m(2, 0) = m4(2, 0);
		m(2, 1) = m4(2, 1);
		m(2, 2) = m4(2, 2);
		m(2, 3) = m4(2, 3);
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	explicit TMat(const TVec<T, 3>& translation, const TMat<T, 3, 3>& rotation, const T scale = T(1))
	{
		if(isZero<T>(scale - T(1)))
		{
			setRotationPart(rotation);
		}
		else
		{
			setRotationPart(rotation * scale);
		}

		setTranslationPart(translation);
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	explicit TMat(const TVec<T, 3>& translation, const TQuat<T>& q, const T scale = T(1))
		: TMat(translation, TMat<T, 3, 3>(q), scale)
	{
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	explicit TMat(const TVec<T, 3>& translation, const TEuler<T>& b, const T scale = T(1))
		: TMat(translation, TMat<T, 3, 3>(b), scale)
	{
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	explicit TMat(const TVec<T, 3>& translation, const TAxisang<T>& b, const T scale = T(1))
		: TMat(translation, TMat<T, 3, 3>(b), scale)
	{
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4)
	explicit TMat(const TTransform<T>& t)
		: TMat(t.getOrigin().xyz(), t.getRotation().getRotationPart(), t.getScale())
	{
	}
	/// @}

	/// @name Accessors
	/// @{
	T& operator()(const U j, const U i)
	{
		return m_arr2[j][i];
	}

	T operator()(const U j, const U i) const
	{
		return m_arr2[j][i];
	}

	T& operator[](const U n)
	{
		return m_arr1[n];
	}

	T operator[](const U n) const
	{
		return m_arr1[n];
	}
	/// @}

	/// @name Operators with same type
	/// @{

	/// Copy.
	TMat& operator=(ANKI_ENABLE_ARG(const TMat&, !HAS_SIMD) b)
	{
		for(U n = 0; n < N; n++)
		{
			m_arr1[n] = b.m_arr1[n];
		}
		return *this;
	}

	/// Copy.
	TMat& operator=(ANKI_ENABLE_ARG(const TMat&, HAS_SIMD) b)
	{
		for(U i = 0; i < J; i++)
		{
			m_simd[i] = b.m_simd[i];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_SIMD)
	TMat operator+(const TMat& b) const
	{
		TMat c;
		for(U n = 0; n < N; n++)
		{
			c.m_arr1[n] = m_arr1[n] + b.m_arr1[n];
		}
		return c;
	}

	ANKI_ENABLE_METHOD(HAS_SIMD)
	TMat operator+(const TMat& b) const
	{
		TMat c;
		for(U i = 0; i < J; i++)
		{
			c.m_simd[i] = _mm_add_ps(m_simd[i], b.m_simd[i]);
		}
		return c;
	}

	ANKI_ENABLE_METHOD(!HAS_SIMD)
	TMat& operator+=(const TMat& b)
	{
		for(U n = 0; n < N; n++)
		{
			m_arr1[n] += b.m_arr1[n];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(HAS_SIMD)
	TMat& operator+=(const TMat& b)
	{
		for(U i = 0; i < J; i++)
		{
			m_simd[i] = _mm_add_ps(m_simd[i], b.m_simd[i]);
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(!HAS_SIMD)
	TMat operator-(const TMat& b) const
	{
		TMat c;
		for(U n = 0; n < N; n++)
		{
			c.m_arr1[n] = m_arr1[n] - b.m_arr1[n];
		}
		return c;
	}

	ANKI_ENABLE_METHOD(HAS_SIMD)
	TMat operator-(const TMat& b) const
	{
		TMat c;
		for(U i = 0; i < J; i++)
		{
			c.m_simd[i] = _mm_sub_ps(m_simd[i], b.m_simd[i]);
		}
		return c;
	}

	ANKI_ENABLE_METHOD(!HAS_SIMD)
	TMat& operator-=(const TMat& b)
	{
		for(U n = 0; n < N; n++)
		{
			m_arr1[n] -= b.m_arr1[n];
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(HAS_SIMD)
	TMat& operator-=(const TMat& b)
	{
		for(U i = 0; i < J; i++)
		{
			m_simd[i] = _mm_sub_ps(m_simd[i], b.m_simd[i]);
		}
		return *this;
	}

	ANKI_ENABLE_METHOD(J == I && !HAS_MAT4_SIMD)
	TMat operator*(const TMat& b) const
	{
		TMat out;
		const TMat& a = *this;
		for(U j = 0; j < J; j++)
		{
			for(U i = 0; i < I; i++)
			{
				out(j, i) = T(0);
				for(U k = 0; k < I; k++)
				{
					out(j, i) += a(j, k) * b(k, i);
				}
			}
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_MAT4_SIMD)
	TMat operator*(const TMat& b) const
	{
		TMat out;
		const auto& m = *this;

		for(U i = 0; i < 4; i++)
		{
			__m128 t1, t2;

			t1 = _mm_set1_ps(m(i, 0));
			t2 = _mm_mul_ps(b.m_simd[0], t1);
			t1 = _mm_set1_ps(m(i, 1));
			t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[1], t1), t2);
			t1 = _mm_set1_ps(m(i, 2));
			t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[2], t1), t2);
			t1 = _mm_set1_ps(m(i, 3));
			t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[3], t1), t2);

			out.m_simd[i] = t2;
		}

		return out;
	}

	TMat& operator*=(const TMat& b)
	{
		(*this) = (*this) * b;
		return *this;
	}

	Bool operator==(const TMat& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(!isZero<T>(m_arr1[i] - b.m_arr1[i]))
			{
				return false;
			}
		}
		return true;
	}

	Bool operator!=(const TMat& b) const
	{
		for(U i = 0; i < N; i++)
		{
			if(!isZero<T>(m_arr1[i] - b.m_arr1[i]))
			{
				return true;
			}
		}
		return false;
	}
	/// @}

	/// @name Operators with T
	/// @{
	TMat operator+(const T f) const
	{
		TMat out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] + f;
		}
		return out;
	}

	TMat& operator+=(const T f)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] += f;
		}
		return *this;
	}

	TMat operator-(const T f) const
	{
		TMat out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] - f;
		}
		return out;
	}

	TMat& operator-=(const T f)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] -= f;
		}
		return *this;
	}

	TMat operator*(const T f) const
	{
		TMat out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] * f;
		}
		return out;
	}

	TMat& operator*=(const T f)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] *= f;
		}
		return *this;
	}

	TMat operator/(const T f) const
	{
		ANKI_ASSERT(f != T(0));
		TMat out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] / f;
		}
		return out;
	}

	TMat& operator/=(const T f)
	{
		ANKI_ASSERT(f != T(0));
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] /= f;
		}
		return *this;
	}
	/// @}

	/// @name Operators with other types
	/// @{
	ANKI_ENABLE_METHOD(!HAS_MAT3X4_SIMD)
	ColumnVec operator*(const RowVec& v) const
	{
		const TMat& m = *this;
		ColumnVec out;
		for(U j = 0; j < J; j++)
		{
			T sum = 0.0;
			for(U i = 0; i < I; i++)
			{
				sum += m(j, i) * v[i];
			}
			out[j] = sum;
		}
		return out;
	}

	ANKI_ENABLE_METHOD(HAS_MAT3X4_SIMD)
	ColumnVec operator*(const RowVec& v) const
	{
		ColumnVec out;
		for(U i = 0; i < J; i++)
		{
			_mm_store_ss(&out[i], _mm_dp_ps(m_simd[i], v.getSimd(), 0xF1));
		}
		return out;
	}
	/// @}

	/// @name Other
	/// @{
	ANKI_ENABLE_METHOD(!HAS_SIMD)
	void setRow(const U j, const RowVec& v)
	{
		for(U i = 0; i < I; i++)
		{
			m_arr2[j][i] = v[i];
		}
	}

	ANKI_ENABLE_METHOD(HAS_SIMD)
	void setRow(const U j, const RowVec& v)
	{
		m_simd[j] = v.getSimd();
	}

	void setRows(const RowVec& a, const RowVec& b, const RowVec& c)
	{
		setRow(0, a);
		setRow(1, b);
		setRow(2, c);
	}

	ANKI_ENABLE_METHOD(J > 3)
	void setRows(const RowVec& a, const RowVec& b, const RowVec& c, const RowVec& d)
	{
		setRows(a, b, c);
		setRow(3, d);
	}

	RowVec getRow(const U j) const
	{
		RowVec out;
		for(U i = 0; i < I; i++)
		{
			out[i] = m_arr2[j][i];
		}
		return out;
	}

	void getRows(RowVec& a, RowVec& b, RowVec& c) const
	{
		a = getRow(0);
		b = getRow(1);
		c = getRow(2);
	}

	void getRows(RowVec& a, RowVec& b, RowVec& c, RowVec& d) const
	{
		static_assert(J > 3, "Wrong matrix");
		getRows(a, b, c);
		d = getRow(3);
	}

	void setColumn(const U i, const ColumnVec& v)
	{
		for(U j = 0; j < J; j++)
		{
			m_arr2[j][i] = v[j];
		}
	}

	void setColumns(const ColumnVec& a, const ColumnVec& b, const ColumnVec& c)
	{
		setColumn(0, a);
		setColumn(1, b);
		setColumn(2, c);
	}

	void setColumns(const ColumnVec& a, const ColumnVec& b, const ColumnVec& c, const ColumnVec& d)
	{
		static_assert(I > 3, "Check column number");
		setColumns(a, b, c);
		setColumn(3, d);
	}

	ColumnVec getColumn(const U i) const
	{
		ColumnVec out;
		for(U j = 0; j < J; j++)
		{
			out[j] = m_arr2[j][i];
		}
		return out;
	}

	void getColumns(ColumnVec& a, ColumnVec& b, ColumnVec& c) const
	{
		a = getColumn(0);
		b = getColumn(1);
		c = getColumn(2);
	}

	void getColumns(ColumnVec& a, ColumnVec& b, ColumnVec& c, ColumnVec& d) const
	{
		static_assert(I > 3, "Check column number");
		getColumns(a, b, c);
		d = getColumn(3);
	}

	/// Get 1st column
	ColumnVec getXAxis() const
	{
		return getColumn(0);
	}

	/// Get 2nd column
	ColumnVec getYAxis() const
	{
		return getColumn(1);
	}

	/// Get 3rd column
	ColumnVec getZAxis() const
	{
		return getColumn(2);
	}

	/// Set 1st column
	void setXAxis(const ColumnVec& v)
	{
		setColumn(0, v);
	}

	/// Set 2nd column
	void setYAxis(const ColumnVec& v)
	{
		setColumn(1, v);
	}

	/// Set 3rd column
	void setZAxis(const ColumnVec& v)
	{
		setColumn(2, v);
	}

	void setRotationX(const T rad)
	{
		TMat& m = *this;
		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		m(0, 0) = 1.0;
		m(0, 1) = 0.0;
		m(0, 2) = 0.0;
		m(1, 0) = 0.0;
		m(1, 1) = costheta;
		m(1, 2) = -sintheta;
		m(2, 0) = 0.0;
		m(2, 1) = sintheta;
		m(2, 2) = costheta;
	}

	void setRotationY(const T rad)
	{
		TMat& m = *this;
		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		m(0, 0) = costheta;
		m(0, 1) = 0.0;
		m(0, 2) = sintheta;
		m(1, 0) = 0.0;
		m(1, 1) = 1.0;
		m(1, 2) = 0.0;
		m(2, 0) = -sintheta;
		m(2, 1) = 0.0;
		m(2, 2) = costheta;
	}

	void setRotationZ(const T rad)
	{
		TMat& m = *this;
		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		m(0, 0) = costheta;
		m(0, 1) = -sintheta;
		m(0, 2) = 0.0;
		m(1, 0) = sintheta;
		m(1, 1) = costheta;
		m(1, 2) = 0.0;
		m(2, 0) = 0.0;
		m(2, 1) = 0.0;
		m(2, 2) = 1.0;
	}

	/// It rotates "this" in the axis defined by the rotation AND not the
	/// world axis
	void rotateXAxis(const T rad)
	{
		TMat& m = *this;
		// If we analize the mat3 we can extract the 3 unit vectors rotated by the mat3. The 3 rotated vectors are in
		// mat's columns. This means that: mat3.colomn[0] == i * mat3. rotateXAxis() rotates rad angle not from i
		// vector (aka x axis) but from the vector from colomn 0
		// NOTE: See the clean code from < r664

		T sina, cosa;
		sinCos(rad, sina, cosa);

		// zAxis = zAxis*cosa - yAxis*sina;
		m(0, 2) = m(0, 2) * cosa - m(0, 1) * sina;
		m(1, 2) = m(1, 2) * cosa - m(1, 1) * sina;
		m(2, 2) = m(2, 2) * cosa - m(2, 1) * sina;

		// zAxis.normalize();
		T len = sqrt(m(0, 2) * m(0, 2) + m(1, 2) * m(1, 2) + m(2, 2) * m(2, 2));
		m(0, 2) /= len;
		m(1, 2) /= len;
		m(2, 2) /= len;

		// yAxis = zAxis * xAxis;
		m(0, 1) = m(1, 2) * m(2, 0) - m(2, 2) * m(1, 0);
		m(1, 1) = m(2, 2) * m(0, 0) - m(0, 2) * m(2, 0);
		m(2, 1) = m(0, 2) * m(1, 0) - m(1, 2) * m(0, 0);

		// yAxis.normalize();
	}

	/// @copybrief rotateXAxis
	void rotateYAxis(const T rad)
	{
		TMat& m = *this;
		// NOTE: See the clean code from < r664
		T sina, cosa;
		sinCos(rad, sina, cosa);

		// zAxis = zAxis*cosa + xAxis*sina;
		m(0, 2) = m(0, 2) * cosa + m(0, 0) * sina;
		m(1, 2) = m(1, 2) * cosa + m(1, 0) * sina;
		m(2, 2) = m(2, 2) * cosa + m(2, 0) * sina;

		// zAxis.normalize();
		T len = sqrt(m(0, 2) * m(0, 2) + m(1, 2) * m(1, 2) + m(2, 2) * m(2, 2));
		m(0, 2) /= len;
		m(1, 2) /= len;
		m(2, 2) /= len;

		// xAxis = (zAxis*yAxis) * -1.0f;
		m(0, 0) = m(2, 2) * m(1, 1) - m(1, 2) * m(2, 1);
		m(1, 0) = m(0, 2) * m(2, 1) - m(2, 2) * m(0, 1);
		m(2, 0) = m(1, 2) * m(0, 1) - m(0, 2) * m(1, 1);
	}

	/// @copybrief rotateXAxis
	void rotateZAxis(const T rad)
	{
		TMat& m = *this;
		// NOTE: See the clean code from < r664
		T sina, cosa;
		sinCos(rad, sina, cosa);

		// xAxis = xAxis*cosa + yAxis*sina;
		m(0, 0) = m(0, 0) * cosa + m(0, 1) * sina;
		m(1, 0) = m(1, 0) * cosa + m(1, 1) * sina;
		m(2, 0) = m(2, 0) * cosa + m(2, 1) * sina;

		// xAxis.normalize();
		T len = sqrt(m(0, 0) * m(0, 0) + m(1, 0) * m(1, 0) + m(2, 0) * m(2, 0));
		m(0, 0) /= len;
		m(1, 0) /= len;
		m(2, 0) /= len;

		// yAxis = zAxis*xAxis;
		m(0, 1) = m(1, 2) * m(2, 0) - m(2, 2) * m(1, 0);
		m(1, 1) = m(2, 2) * m(0, 0) - m(0, 2) * m(2, 0);
		m(2, 1) = m(0, 2) * m(1, 0) - m(1, 2) * m(0, 0);
	}

	void setRotationPart(const TMat<T, 3, 3>& m3)
	{
		TMat& m = *this;
		for(U j = 0; j < 3; j++)
		{
			for(U i = 0; i < 3; i++)
			{
				m(j, i) = m3(j, i);
			}
		}
	}

	void setRotationPart(const TQuat<T>& q)
	{
		TMat& m = *this;
		// If length is > 1 + 0.002 or < 1 - 0.002 then not normalized quat
		ANKI_ASSERT(absolute(1.0 - q.getLength()) <= 0.002);

		T xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

		xs = q.x() + q.x();
		ys = q.y() + q.y();
		zs = q.z() + q.z();
		wx = q.w() * xs;
		wy = q.w() * ys;
		wz = q.w() * zs;
		xx = q.x() * xs;
		xy = q.x() * ys;
		xz = q.x() * zs;
		yy = q.y() * ys;
		yz = q.y() * zs;
		zz = q.z() * zs;

		m(0, 0) = T(1) - (yy + zz);
		m(0, 1) = xy - wz;
		m(0, 2) = xz + wy;

		m(1, 0) = xy + wz;
		m(1, 1) = T(1) - (xx + zz);
		m(1, 2) = yz - wx;

		m(2, 0) = xz - wy;
		m(2, 1) = yz + wx;
		m(2, 2) = T(1) - (xx + yy);
	}

	void setRotationPart(const TEuler<T>& e)
	{
		TMat& m = *this;
		T ch, sh, ca, sa, cb, sb;
		sinCos(e.y(), sh, ch);
		sinCos(e.z(), sa, ca);
		sinCos(e.x(), sb, cb);

		m(0, 0) = ch * ca;
		m(0, 1) = sh * sb - ch * sa * cb;
		m(0, 2) = ch * sa * sb + sh * cb;
		m(1, 0) = sa;
		m(1, 1) = ca * cb;
		m(1, 2) = -ca * sb;
		m(2, 0) = -sh * ca;
		m(2, 1) = sh * sa * cb + ch * sb;
		m(2, 2) = -sh * sa * sb + ch * cb;
	}

	void setRotationPart(const TAxisang<T>& axisang)
	{
		TMat& m = *this;
		// Not normalized axis
		ANKI_ASSERT(isZero<T>(T(1) - axisang.getAxis().getLength()));

		T c, s;
		sinCos(axisang.getAngle(), s, c);
		T t = T(1) - c;

		const TVec<T, 3>& axis = axisang.getAxis();
		m(0, 0) = c + axis.x() * axis.x() * t;
		m(1, 1) = c + axis.y() * axis.y() * t;
		m(2, 2) = c + axis.z() * axis.z() * t;

		T tmp1 = axis.x() * axis.y() * t;
		T tmp2 = axis.z() * s;
		m(1, 0) = tmp1 + tmp2;
		m(0, 1) = tmp1 - tmp2;
		tmp1 = axis.x() * axis.z() * t;
		tmp2 = axis.y() * s;
		m(2, 0) = tmp1 - tmp2;
		m(0, 2) = tmp1 + tmp2;
		tmp1 = axis.y() * axis.z() * t;
		tmp2 = axis.x() * s;
		m(2, 1) = tmp1 + tmp2;
		m(1, 2) = tmp1 - tmp2;
	}

	TMat<T, 3, 3> getRotationPart() const
	{
		const TMat& m = *this;
		TMat<T, 3, 3> m3;
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

	void setTranslationPart(const ColumnVec& v)
	{
		if(ROW_SIZE == 4)
		{
			ANKI_ASSERT(isZero<T>(v[3] - T(1)) && "w should be 1");
		}
		setColumn(3, v);
	}

	ColumnVec getTranslationPart() const
	{
		return getColumn(3);
	}

	void reorthogonalize()
	{
		// There are 2 methods, the standard and the Gram-Schmidt method with a twist for zAxis. This uses the 2nd. For
		// the first see < r664
		ColumnVec xAxis, yAxis, zAxis;
		getColumns(xAxis, yAxis, zAxis);

		xAxis.normalize();

		yAxis = yAxis - (xAxis * xAxis.dot(yAxis));
		yAxis.normalize();

		zAxis = xAxis.cross(yAxis);

		setColumns(xAxis, yAxis, zAxis);
	}

	ANKI_ENABLE_METHOD(J == I && !HAS_SIMD)
	void transpose()
	{
		for(U j = 0; j < J; j++)
		{
			for(U i = j + 1; i < I; i++)
			{
				T tmp = m_arr2[j][i];
				m_arr2[j][i] = m_arr2[i][j];
				m_arr2[i][j] = tmp;
			}
		}
	}

	ANKI_ENABLE_METHOD(J == I && HAS_SIMD)
	void transpose()
	{
		_MM_TRANSPOSE4_PS(m_simd[0], m_simd[1], m_simd[2], m_simd[3]);
	}

	void transposeRotationPart()
	{
		for(U j = 0; j < 3; j++)
		{
			for(U i = j + 1; i < 3; i++)
			{
				T tmp = m_arr2[j][i];
				m_arr2[j][i] = m_arr2[i][j];
				m_arr2[i][j] = tmp;
			}
		}
	}

	ANKI_ENABLE_METHOD(I == J)
	TMat getTransposed() const
	{
		TMat out;
		for(U j = 0; j < J; j++)
		{
			for(U i = 0; i < I; i++)
			{
				out.m_arr2[i][j] = m_arr2[j][i];
			}
		}
		return out;
	}

	ANKI_ENABLE_METHOD(I == 3 && J == 3)
	T getDet() const
	{
		const auto& m = *this;
		// For the accurate method see < r664
		return m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)) - m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0))
			   + m(0, 2) * (m(0, 1) * m(2, 1) - m(1, 1) * m(2, 0));
	}

	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	T getDet() const
	{
		const auto& t = *this;
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

	ANKI_ENABLE_METHOD(I == 3 && J == 3)
	TMat getInverse() const
	{
		// Using Gramer's method Inv(A) = (1 / getDet(A)) * Adj(A)
		const TMat& m = *this;
		TMat r;

		// compute determinant
		const T cofactor0 = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1);
		const T cofactor3 = m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2);
		const T cofactor6 = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);
		const T det = m(0, 0) * cofactor0 + m(1, 0) * cofactor3 + m(2, 0) * cofactor6;

		ANKI_ASSERT(!isZero<T>(det)); // Cannot invert det == 0

		// create adjoint matrix and multiply by 1/det to get inverse
		const T invDet = 1.0 / det;
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

	/// Invert using Cramer's rule
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	TMat getInverse() const
	{
		Array<T, 12> tmp;
		const auto& in = (*this);
		TMat m4;

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
		det = T(1) / det;
		m4 *= det;
		return m4;
	}

	/// See getInverse
	ANKI_ENABLE_METHOD((I == 4 && J == 4) || (I == 3 && J == 3))
	void invert()
	{
		(*this) = getInverse();
	}

	/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching the 4rth row and allot faster
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	static TMat combineTransformations(const TMat& m0, const TMat& m1)
	{
		// See the clean code in < r664

		// one of the 2 mat4 doesnt represent transformation
		ANKI_ASSERT(isZero<T>(m0(3, 0) + m0(3, 1) + m0(3, 2) + m0(3, 3) - 1.0)
					&& isZero<T>(m1(3, 0) + m1(3, 1) + m1(3, 2) + m1(3, 3) - 1.0));

		TMat m4;

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

	/// Create a new matrix that is equivalent to Mat4(this)*Mat4(b)
	ANKI_ENABLE_METHOD(J == 3 && I == 4 && !HAS_SIMD)
	TMat combineTransformations(const TMat& b) const
	{
		const auto& a = *this;
		TMat c;

		c(0, 0) = a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0) + a(0, 2) * b(2, 0);
		c(0, 1) = a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1) + a(0, 2) * b(2, 1);
		c(0, 2) = a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2) * b(2, 2);
		c(1, 0) = a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0) + a(1, 2) * b(2, 0);
		c(1, 1) = a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1) + a(1, 2) * b(2, 1);
		c(1, 2) = a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2) * b(2, 2);
		c(2, 0) = a(2, 0) * b(0, 0) + a(2, 1) * b(1, 0) + a(2, 2) * b(2, 0);
		c(2, 1) = a(2, 0) * b(0, 1) + a(2, 1) * b(1, 1) + a(2, 2) * b(2, 1);
		c(2, 2) = a(2, 0) * b(0, 2) + a(2, 1) * b(1, 2) + a(2, 2) * b(2, 2);

		c(0, 3) = a(0, 0) * b(0, 3) + a(0, 1) * b(1, 3) + a(0, 2) * b(2, 3) + a(0, 3);

		c(1, 3) = a(1, 0) * b(0, 3) + a(1, 1) * b(1, 3) + a(1, 2) * b(2, 3) + a(1, 3);

		c(2, 3) = a(2, 0) * b(0, 3) + a(2, 1) * b(1, 3) + a(2, 2) * b(2, 3) + a(2, 3);

		return c;
	}

	ANKI_ENABLE_METHOD(J == 3 && I == 4 && HAS_SIMD)
	TMat combineTransformations(const TMat& b) const
	{
		TMat c;
		const auto& a = *this;

		for(U i = 0; i < 3; i++)
		{
			__m128 t1, t2;

			t1 = _mm_set1_ps(a(i, 0));
			t2 = _mm_mul_ps(b.m_simd[0], t1);
			t1 = _mm_set1_ps(a(i, 1));
			t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[1], t1), t2);
			t1 = _mm_set1_ps(a(i, 2));
			t2 = _mm_add_ps(_mm_mul_ps(b.m_simd[2], t1), t2);

			TVec<T, 4> v4(0.0, 0.0, 0.0, a(i, 3));
			t2 = _mm_add_ps(v4.getSimd(), t2);

			c.m_simd[i] = t2;
		}

		return c;
	}

	/// Calculate a perspective projection matrix. The z is mapped in [0, 1] range just like DX and Vulkan.
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	static ANKI_USE_RESULT TMat calculatePerspectiveProjectionMatrix(T fovX, T fovY, T near, T far)
	{
		ANKI_ASSERT(fovX > T(0) && fovY > T(0) && near > T(0) && far > T(0));
		const T g = near - far;
		const T f = T(1) / tan(fovY / T(2)); // f = cot(fovY/2)

		TMat proj;
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
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	static ANKI_USE_RESULT TMat calculateOrthographicProjectionMatrix(T right, T left, T top, T bottom, T near, T far)
	{
		ANKI_ASSERT(right != T(0) && left != T(0) && top != T(0) && bottom != T(0) && near != T(0) && far != T(0));
		const T difx = right - left;
		const T dify = top - bottom;
		const T difz = far - near;
		const T tx = -(right + left) / difx;
		const T ty = -(top + bottom) / dify;
		const T tz = -near / difz;
		TMat m;

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
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	static TVec<T, 4> calculatePerspectiveUnprojectionParams(T fovX, T fovY, T near, T far)
	{
		TVec<T, 4> out;
		const T g = near - far;
		const T f = T(1) / tan(fovY / T(2)); // f = cot(fovY/2)

		const T m00 = f * (fovY / fovX);
		const T m11 = f;
		const T m22 = far / g;
		const T m23 = (far * near) / g;

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
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	TVec<T, 4> extractPerspectiveUnprojectionParams() const
	{
		TVec<T, 4> out;
		const auto& m = *this;
		out.z() = -m(2, 3);
		out.w() = m(2, 2);
		out.x() = -T(1.0) / m(0, 0);
		out.y() = -T(1.0) / m(1, 1);
		return out;
	}

	/// If we suppose this matrix represents a transformation, return the inverted transformation
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	TMat getInverseTransformation() const
	{
		const TMat<T, 3, 3> invertedRot = getRotationPart().getTransposed();
		TVec<T, 3> invertedTsl = getTranslationPart().xyz();
		invertedTsl = -(invertedRot * invertedTsl);
		return TMat(invertedTsl.xyz0(), invertedRot);
	}

	/// @note 9 muls, 9 adds
	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	TVec<T, 3> transform(const TVec<T, 3>& v) const
	{
		const auto& m = *this;
		return TVec<T, 3>(m(0, 0) * v.x() + m(0, 1) * v.y() + m(0, 2) * v.z() + m(0, 3),
						  m(1, 0) * v.x() + m(1, 1) * v.y() + m(1, 2) * v.z() + m(1, 3),
						  m(2, 0) * v.x() + m(2, 1) * v.y() + m(2, 2) * v.z() + m(2, 3));
	}

	/// Create a new transform matrix position at eye and looking at refPoint.
	template<U VEC_DIMS, ANKI_ENABLE(J == 3 && I == 4 && VEC_DIMS >= 3)>
	static TMat lookAt(const TVec<T, VEC_DIMS>& eye, const TVec<T, VEC_DIMS>& refPoint, const TVec<T, VEC_DIMS>& up)
	{
		const TVec<T, 3> vdir = (refPoint.xyz() - eye.xyz()).getNormalized();
		const TVec<T, 3> vup = (up.xyz() - vdir * up.xyz().dot(vdir)).getNormalized();
		const TVec<T, 3> vside = vdir.cross(vup);
		TMat out;
		out.setColumns(vside, vup, -vdir, eye.xyz());
		return out;
	}

	/// Create a new transform matrix position at eye and looking at refPoint.
	template<U VEC_DIMS, ANKI_ENABLE(J == 4 && I == 4 && VEC_DIMS >= 3)>
	static TMat lookAt(const TVec<T, VEC_DIMS>& eye, const TVec<T, VEC_DIMS>& refPoint, const TVec<T, VEC_DIMS>& up)
	{
		const TVec<T, 4> vdir = (refPoint.xyz0() - eye.xyz0()).getNormalized();
		const TVec<T, 4> vup = (up.xyz0() - vdir * up.xyz0().dot(vdir)).getNormalized();
		const TVec<T, 4> vside = vdir.cross(vup);
		TMat out;
		out.setColumns(vside, vup, -vdir, eye.xyz1());
		return out;
	}

	TMat lerp(const TMat& b, T t) const
	{
		return ((*this) * (1.0 - t)) + (b * t);
	}

	static TMat getZero()
	{
		return TMat(0.0);
	}

	void setZero()
	{
		*this = getZero();
	}

	ANKI_ENABLE_METHOD(I == 3 && J == 3)
	static TMat getIdentity()
	{
		return TMat(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	}

	ANKI_ENABLE_METHOD(I == 4 && J == 4)
	static TMat getIdentity()
	{
		return TMat(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
	}

	ANKI_ENABLE_METHOD(I == 4 && J == 3)
	static TMat getIdentity()
	{
		return TMat(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static constexpr U8 getSize()
	{
		return U8(I * J);
	}

	ANKI_ENABLE_METHOD(std::is_floating_point<T>::value)
	void toString(StringAuto& str) const
	{
		for(U j = 0; j < J; ++j)
		{
			for(U i = 0; i < I; ++i)
			{
				CString fmt;
				if(i == I - 1 && j == J - 1)
				{
					fmt = "%f";
				}
				else if(i == I - 1)
				{
					fmt = "%f\n";
				}
				else
				{
					fmt = "%f ";
				}
				str.append(StringAuto(str.getAllocator()).sprintf(fmt, m_arr2[j][i]));
			}
		}
	}
	/// @}

protected:
	static constexpr U N = I * J;

	/// @name Data members
	/// @{
	union
	{
		Array<T, N> m_arr1;
		Array2d<T, J, I> m_arr2;
		T m_carr1[N]; ///< For easier debugging with gdb
		T m_carr2[J][I]; ///< For easier debugging with gdb
		SimdArray m_simd;
	};
	/// @}
};

/// @memberof TMat
template<typename T, U J, U I>
TMat<T, J, I> operator+(const T f, const TMat<T, J, I>& m)
{
	return m + f;
}

/// @memberof TMat
template<typename T, U J, U I>
TMat<T, J, I> operator-(const T f, const TMat<T, J, I>& m)
{
	TMat<T, J, I> out;
	for(U i = 0; i < J * I; i++)
	{
		out[i] = f - m[i];
	}
	return out;
}

/// @memberof TMat
template<typename T, U J, U I>
TMat<T, J, I> operator*(const T f, const TMat<T, J, I>& m)
{
	return m * f;
}

/// @memberof TMat
template<typename T, U J, U I>
TMat<T, J, I> operator/(const T f, const TMat<T, 3, 3>& m3)
{
	TMat<T, J, I> out;
	for(U i = 0; i < J * I; i++)
	{
		ANKI_ASSERT(m3[i] != T(0));
		out[i] = f / m3[i];
	}
	return out;
}

/// F32 3x3 matrix
using Mat3 = TMat<F32, 3, 3>;
static_assert(sizeof(Mat3) == sizeof(F32) * 3 * 3, "Incorrect size");

/// F64 3x3 matrix
using DMat3 = TMat<F64, 3, 3>;
static_assert(sizeof(DMat3) == sizeof(F64) * 3 * 3, "Incorrect size");

/// F32 4x4 matrix
using Mat4 = TMat<F32, 4, 4>;
static_assert(sizeof(Mat4) == sizeof(F32) * 4 * 4, "Incorrect size");

/// F64 4x4 matrix
using DMat4 = TMat<F64, 4, 4>;
static_assert(sizeof(DMat4) == sizeof(F64) * 4 * 4, "Incorrect size");

/// F32 3x4 matrix
using Mat3x4 = TMat<F32, 3, 4>;
static_assert(sizeof(Mat3x4) == sizeof(F32) * 3 * 4, "Incorrect size");

/// F64 3x4 matrix
using DMat3x4 = TMat<F64, 3, 4>;
static_assert(sizeof(DMat3x4) == sizeof(F64) * 3 * 4, "Incorrect size");
/// @}

} // end namespace anki
