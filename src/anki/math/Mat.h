// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>
#include <anki/math/Vec.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Common code for all matrices
/// @tparam T The scalar type. Eg float.
/// @tparam J The number of rows.
/// @tparam I The number of columns.
/// @tparam TM The type of the derived class. Eg TMat3.
/// @tparam TVJ The vector type of the row.
/// @tparam TVI The vector type of the column.
template<typename T, U J, U I, typename TSimd, typename TM, typename TVJ, typename TVI>
class TMat
{
public:
	using Scalar = T;
	static constexpr U ROW_SIZE = J; ///< Number of rows
	static constexpr U COLUMN_SIZE = I; ///< Number of columns
	static constexpr U SIZE = J * I; ///< Number of total elements

	/// @name Constructors
	/// @{
	TMat()
	{
	}

	TMat(const TMat& b)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] = b.m_arr1[i];
		}
	}

	explicit TMat(const T f)
	{
		for(T& x : m_arr1)
		{
			x = f;
		}
	}

	explicit TMat(const T arr[])
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] = arr[i];
		}
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
	TM& operator=(const TM& b)
	{
		for(U n = 0; n < N; n++)
		{
			m_arr1[n] = b.m_arr1[n];
		}
		return static_cast<TM&>(*this);
	}

	TM operator+(const TM& b) const
	{
		TM c;
		for(U n = 0; n < N; n++)
		{
			c.m_arr1[n] = m_arr1[n] + b.m_arr1[n];
		}
		return c;
	}

	TM& operator+=(const TM& b)
	{
		for(U n = 0; n < N; n++)
		{
			m_arr1[n] += b.m_arr1[n];
		}
		return static_cast<TM&>(*this);
	}

	TM operator-(const TM& b) const
	{
		TM c;
		for(U n = 0; n < N; n++)
		{
			c.m_arr1[n] = m_arr1[n] - b.m_arr1[n];
		}
		return c;
	}

	TM& operator-=(const TM& b)
	{
		for(U n = 0; n < N; n++)
		{
			m_arr1[n] -= b.m_arr1[n];
		}
		return static_cast<TM&>(*this);
	}

	TM operator*(const TM& b) const
	{
		static_assert(I == J, "Only for square matrices");
		TM out;
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

	TM& operator*=(const TM& b)
	{
		(*this) = (*this) * b;
		return static_cast<TM&>(*this);
	}

	Bool operator==(const TM& b) const
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

	Bool operator!=(const TM& b) const
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
	TM operator+(const T f) const
	{
		TM out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] + f;
		}
		return out;
	}

	TM& operator+=(const T f)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] += f;
		}
		return static_cast<TM&>(*this);
	}

	TM operator-(const T f) const
	{
		TM out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] - f;
		}
		return out;
	}

	TM& operator-=(const T f)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] -= f;
		}
		return static_cast<TM&>(*this);
	}

	TM operator*(const T f) const
	{
		TM out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] * f;
		}
		return out;
	}

	TM& operator*=(const T f)
	{
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] *= f;
		}
		return static_cast<TM&>(*this);
	}

	TM operator/(const T f) const
	{
		ANKI_ASSERT(f != T(0));
		TM out;
		for(U i = 0; i < N; i++)
		{
			out.m_arr1[i] = m_arr1[i] / f;
		}
		return out;
	}

	TM& operator/=(const T f)
	{
		ANKI_ASSERT(f != T(0));
		for(U i = 0; i < N; i++)
		{
			m_arr1[i] /= f;
		}
		return static_cast<TM&>(*this);
	}
	/// @}

	/// @name Operators with other types
	/// @{
	TVI operator*(const TVJ& v) const
	{
		const TMat& m = *this;
		TVI out;
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
	/// @}

	/// @name Other
	/// @{
	void setRow(const U j, const TVJ& v)
	{
		for(U i = 0; i < I; i++)
		{
			m_arr2[j][i] = v[i];
		}
	}

	void setRows(const TVJ& a, const TVJ& b, const TVJ& c)
	{
		setRow(0, a);
		setRow(1, b);
		setRow(2, c);
	}

	void setRows(const TVJ& a, const TVJ& b, const TVJ& c, const TVJ& d)
	{
		static_assert(J > 3, "Wrong matrix");
		setRows(a, b, c);
		setRow(3, d);
	}

	TVJ getRow(const U j) const
	{
		TVJ out;
		for(U i = 0; i < I; i++)
		{
			out[i] = m_arr2[j][i];
		}
		return out;
	}

	void getRows(TVJ& a, TVJ& b, TVJ& c) const
	{
		a = getRow(0);
		b = getRow(1);
		c = getRow(2);
	}

	void getRows(TVJ& a, TVJ& b, TVJ& c, TVJ& d) const
	{
		static_assert(J > 3, "Wrong matrix");
		getRows(a, b, c);
		d = getRow(3);
	}

	void setColumn(const U i, const TVI& v)
	{
		for(U j = 0; j < J; j++)
		{
			m_arr2[j][i] = v[j];
		}
	}

	void setColumns(const TVI& a, const TVI& b, const TVI& c)
	{
		setColumn(0, a);
		setColumn(1, b);
		setColumn(2, c);
	}

	void setColumns(const TVI& a, const TVI& b, const TVI& c, const TVI& d)
	{
		static_assert(I > 3, "Check column number");
		setColumns(a, b, c);
		setColumn(3, d);
	}

	TVI getColumn(const U i) const
	{
		TVI out;
		for(U j = 0; j < J; j++)
		{
			out[j] = m_arr2[j][i];
		}
		return out;
	}

	void getColumns(TVI& a, TVI& b, TVI& c) const
	{
		a = getColumn(0);
		b = getColumn(1);
		c = getColumn(2);
	}

	void getColumns(TVI& a, TVI& b, TVI& c, TVI& d) const
	{
		static_assert(I > 3, "Check column number");
		getColumns(a, b, c);
		d = getColumn(3);
	}

	/// Get 1st column
	TVI getXAxis() const
	{
		return getColumn(0);
	}

	/// Get 2nd column
	TVI getYAxis() const
	{
		return getColumn(1);
	}

	/// Get 3rd column
	TVI getZAxis() const
	{
		return getColumn(2);
	}

	/// Set 1st column
	void setXAxis(const TVI& v)
	{
		setColumn(0, v);
	}

	/// Set 2nd column
	void setYAxis(const TVI& v)
	{
		setColumn(1, v);
	}

	/// Set 3rd column
	void setZAxis(const TVI& v)
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

	void setRotationPart(const TMat3<T>& m3)
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

		m(0, 0) = 1.0 - (yy + zz);
		m(0, 1) = xy - wz;
		m(0, 2) = xz + wy;

		m(1, 0) = xy + wz;
		m(1, 1) = 1.0 - (xx + zz);
		m(1, 2) = yz - wx;

		m(2, 0) = xz - wy;
		m(2, 1) = yz + wx;
		m(2, 2) = 1.0 - (xx + yy);
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
		ANKI_ASSERT(isZero<T>(1.0 - axisang.getAxis().getLength()));

		T c, s;
		sinCos(axisang.getAngle(), s, c);
		T t = 1.0 - c;

		const TVec3<T>& axis = axisang.getAxis();
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

	TMat3<T> getRotationPart() const
	{
		const TMat& m = *this;
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

	void setTranslationPart(const TVI& v)
	{
		if(ROW_SIZE == 4)
		{
			ANKI_ASSERT(isZero<T>(v[3] - static_cast<T>(1)) && "w should be 1");
		}
		setColumn(3, v);
	}

	TVI getTranslationPart() const
	{
		return getColumn(3);
	}

	void reorthogonalize()
	{
		// There are 2 methods, the standard and the Gram-Schmidt method with a twist for zAxis. This uses the 2nd. For
		// the first see < r664
		TVI xAxis, yAxis, zAxis;
		getColumns(xAxis, yAxis, zAxis);

		xAxis.normalize();

		yAxis = yAxis - (xAxis * xAxis.dot(yAxis));
		yAxis.normalize();

		zAxis = xAxis.cross(yAxis);

		setColumns(xAxis, yAxis, zAxis);
	}

	void transpose()
	{
		static_assert(I == J, "Only for square matrices");
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

	TM getTransposed() const
	{
		static_assert(I == J, "Only for square matrices");
		TM out;
		for(U j = 0; j < J; j++)
		{
			for(U i = 0; i < I; i++)
			{
				out.m_arr2[i][j] = m_arr2[j][i];
			}
		}
		return out;
	}

	TMat lerp(const TMat& b, T t) const
	{
		return ((*this) * (1.0 - t)) + (b * t);
	}

	static const TM& getZero()
	{
		static const TM zero(0.0);
		return zero;
	}

	void setZero()
	{
		*this = getZero();
	}

	template<typename TAlloc>
	String toString(TAlloc alloc) const
	{
		// TODO
		ANKI_ASSERT(0 && "TODO");
		return String();
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
		TSimd m_simd;
	};
	/// @}
};
/// @}

} // end namespace anki
