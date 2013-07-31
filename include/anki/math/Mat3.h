#ifndef ANKI_MATH_MAT3_H
#define ANKI_MATH_MAT3_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 3x3 Matrix. Mainly used for rotations. It includes many helpful member
/// functions. Its row major. The columns are the x,y,z axis
template<typename T>
class TMat3
{
public:
	/// @name Constructors
	/// @{
	explicit TMat3() 
	{}

	explicit TMat3(const T f)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] = f;
		}
	}

	explicit TMat3(const T m00, const T m01, const T m02,
		const T m10, const T m11, const T m12,
		const T m20, const T m21, const T m22)
	{
		TMat3& m = *this;
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

	explicit TMat3(const T arr[])
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] = arr[i];
		}
	}

	TMat3(const TMat3& b)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] = b.arr1[i];
		}
	}

	/// TQuat to TMat3. 12 muls, 12 adds
	explicit TMat3(const TQuat<T>& q)
	{
		TMat3& m = *this;
		// If length is > 1 + 0.002 or < 1 - 0.002 then not normalized quat
		ANKI_ASSERT(fabs(1.0 - q.getLength()) <= 0.002);

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

	explicit TMat3(const TEuler<T>& e)
	{
		TMat3& m = *this;
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

	explicit TMat3(const TAxisang<T>& axisang)
	{
		TMat3& m = *this;
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
	/// @}

	/// @name Operators with same type
	/// @{
	TMat3& operator=(const TMat3& b)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] = b.arr1[i];
		}
		return (*this);
	}

	TMat3 operator+(const TMat3& b) const
	{
		TMat3<T> c;
		for(U i = 0; i < 9; i++)
		{
			c.arr1[i] = arr1[i] + b.arr1[i];
		}
		return c;
	}

	TMat3& operator+=(const TMat3& b)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] += b.arr1[i];
		}
		return (*this);
	}

	TMat3 operator-(const TMat3& b) const
	{
		TMat3 c;
		for(U i = 0; i < 9; i++)
		{
			c.arr1[i] = arr1[i] - b.arr1[i];
		}
		return c;
	}

	TMat3& operator-=(const TMat3& b)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] -= b.arr1[i];
		}
		return (*this);
	}

	/// @note 27 muls, 18 adds
	TMat3 operator*(const TMat3& b) const
	{
		TMat3 c;
		const TMat3& a = *this;
		c(0, 0) = a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0) + a(0, 2) * b(2, 0);
		c(0, 1) = a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1) + a(0, 2) * b(2, 1);
		c(0, 2) = a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2) * b(2, 2);
		c(1, 0) = a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0) + a(1, 2) * b(2, 0);
		c(1, 1) = a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1) + a(1, 2) * b(2, 1);
		c(1, 2) = a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2) * b(2, 2);
		c(2, 0) = a(2, 0) * b(0, 0) + a(2, 1) * b(1, 0) + a(2, 2) * b(2, 0);
		c(2, 1) = a(2, 0) * b(0, 1) + a(2, 1) * b(1, 1) + a(2, 2) * b(2, 1);
		c(2, 2) = a(2, 0) * b(0, 2) + a(2, 1) * b(1, 2) + a(2, 2) * b(2, 2);
		return c;
	}

	TMat3& operator*=(const TMat3& b)
	{
		(*this) = (*this) * b;
		return (*this);
	}

	Bool operator==(const TMat3& b) const
	{
		for(U i = 0; i < 9; i++)
		{
			if(!isZero<T>(arr1[i] - b.arr1[i]))
			{
				return false;
			}
		}
		return true;
	}

	Bool operator!=(const TMat3& b) const
	{
		for(U i = 0; i < 9; i++)
		{
			if(!isZero<T>(arr1[i] - b.arr1[i]))
			{
				return true;
			}
		}
		return false;
	}
	/// @}

	/// @name Operators with T
	/// @{
	TMat3 operator+(const T f) const
	{
		TMat3 c;
		for(U i = 0; i < 9; i++)
		{
			c.arr1[i] = arr1[i] + f;
		}
		return c;
	}

	TMat3& operator+=(const T f)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] += f;
		}
		return (*this);
	}

	TMat3 operator-(const T f) const
	{
		TMat3 c;
		for(U i = 0; i < 9; i++)
		{
			c.arr1[i] = arr1[i] - f;
		}
		return c;
	}

	TMat3& operator-=(const T f)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] -= f;
		}
		return (*this);
	}

	TMat3 operator*(const T f) const
	{
		TMat3 c;
		for(U i = 0; i < 9; i++)
		{
			c.arr1[i] = arr1[i] * f;
		}
		return c;
	}

	TMat3& operator*=(const T f)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] *= f;
		}
		return (*this);
	}

	TMat3 operator/(const T f) const
	{
		TMat3 c;
		for(U i = 0; i < 9; i++)
		{
			c.arr1[i] = arr1[i] / f;
		}
		return c;
	}

	TMat3& operator/=(const T f)
	{
		for(U i = 0; i < 9; i++)
		{
			arr1[i] /= f;
		}
		return (*this);
	}
	/// @}

	/// @name Operators with others
	/// @{

	/// TVec3<T>(dot(row0 * b), dot(row1 * b), dot(row2 * b)). 9 muls, 6 adds
	TVec3<T> operator*(const TVec3<T>& b) const
	{
		const TMat3& m = *this;
		return TVec3<T>(
			m(0, 0) * b.x() + m(0, 1) * b.y() + m(0, 2) * b.z(),
			m(1, 0) * b.x() + m(1, 1) * b.y() + m(1, 2) * b.z(),
			m(2, 0) * b.x() + m(2, 1) * b.y() + m(2, 2) * b.z());
		
	}
	/// @}

	/// @name Other
	/// @{
	void setRows(const TVec3<T>& a, const TVec3<T>& b, const TVec3<T>& c)
	{
		setRow(0, a);
		setRow(1, b);
		setRow(2, c);
	}

	void setRow(const U i, const TVec3<T>& v)
	{
		TMat3& m = *this;
		m(i, 0) = v.x();
		m(i, 1) = v.y();
		m(i, 2) = v.z();
	}

	void getRows(TVec3<T>& a, TVec3<T>& b, TVec3<T>& c) const
	{
		a = getRow(0);
		b = getRow(1);
		c = getRow(2);
	}

	TVec3<T> getRow(const U i) const
	{
		const TMat3& m = *this;
		return TVec3<T>(m(i, 0), m(i, 1), m(i, 2));
	}

	void setColumns(const TVec3<T>& a, const TVec3<T>& b, const TVec3<T>& c)
	{
		setColumn(0, a);
		setColumn(1, b);
		setColumn(2, c);
	}

	void setColumn(const U i, const TVec3<T>& v)
	{
		TMat3& m = *this;
		m(0, i) = v.x();
		m(1, i) = v.y();
		m(2, i) = v.z();
	}

	void getColumns(TVec3<T>& a, TVec3<T>& b, TVec3<T>& c) const
	{
		a = getColumn(0);
		b = getColumn(1);
		c = getColumn(2);
	}

	TVec3<T> getColumn(const U i) const
	{
		const TMat3& m = *this;
		return TVec3<T>(m(0, i), m(1, i), m(2, i));
	}

	/// Get 1st column
	TVec3<T> getXAxis() const
	{
		return getColumn(0);
	}

	/// Get 2nd column
	TVec3<T> getYAxis() const
	{
		return getColumn(1);
	}

	/// Get 3rd column
	TVec3<T> getZAxis() const
	{
		return getColumn(2);
	}

	/// Set 1st column
	void setXAxis(const TVec3<T>& v3)
	{
		setColumn(0, v3);
	}

	/// Set 2nd column
	void setYAxis(const TVec3<T>& v3)
	{
		setColumn(1, v3);
	}

	/// Set 3rd column
	void setZAxis(const TVec3<T>& v3)
	{
		setColumn(2, v3);
	}

	void setRotationX(const T rad)
	{
		TMat3& m = *this;
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
		TMat3& m = *this;
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
		TMat3& m = *this;
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
		TMat3& m = *this;
		// If we analize the mat3 we can extract the 3 unit vectors rotated by 
		// the mat3. The 3 rotated vectors are in mat's columns. This means 
		// that: mat3.colomn[0] == i * mat3. rotateXAxis() rotates rad angle 
		// not from i vector (aka x axis) but from the vector from colomn 0
		// NOTE: See the clean code from < r664

		T sina, cosa;
		sinCos(rad, sina, cosa);

		// zAxis = zAxis*cosa - yAxis*sina;
		m(0, 2) = m(0, 2) * cosa - m(0, 1) * sina;
		m(1, 2) = m(1, 2) * cosa - m(1, 1) * sina;
		m(2, 2) = m(2, 2) * cosa - m(2, 1) * sina;

		// zAxis.normalize();
		T len = sqrt(m(0, 2) * m(0, 2)
			+ m(1, 2) * m(1, 2) + m(2, 2) * m(2, 2));
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
		TMat3& m = *this;
		// NOTE: See the clean code from < r664
		T sina, cosa;
		sinCos(rad, sina, cosa);

		// zAxis = zAxis*cosa + xAxis*sina;
		m(0, 2) = m(0, 2) * cosa + m(0, 0) * sina;
		m(1, 2) = m(1, 2) * cosa + m(1, 0) * sina;
		m(2, 2) = m(2, 2) * cosa + m(2, 0) * sina;

		// zAxis.normalize();
		T len = sqrt(m(0, 2) * m(0, 2)
			+ m(1, 2) * m(1, 2) + m(2, 2) * m(2, 2));
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
		TMat3& m = *this;
		// NOTE: See the clean code from < r664
		T sina, cosa;
		sinCos(rad, sina, cosa);

		// xAxis = xAxis*cosa + yAxis*sina;
		m(0, 0) = m(0, 0) * cosa + m(0, 1) * sina;
		m(1, 0) = m(1, 0) * cosa + m(1, 1) * sina;
		m(2, 0) = m(2, 0) * cosa + m(2, 1) * sina;

		// xAxis.normalize();
		T len = sqrt(m(0, 0) * m(0, 0)
			+ m(1, 0) * m(1, 0) + m(2, 0) * m(2, 0));
		m(0, 0) /= len;
		m(1, 0) /= len;
		m(2, 0) /= len;

		// yAxis = zAxis*xAxis;
		m(0, 1) = m(1, 2) * m(2, 0) - m(2, 2) * m(1, 0);
		m(1, 1) = m(2, 2) * m(0, 0) - m(0, 2) * m(2, 0);
		m(2, 1) = m(0, 2) * m(1, 0) - m(1, 2) * m(0, 0);
	}

	void transpose()
	{
		TMat3& m = *this;
		T temp = m(0, 1);
		m(0, 1) = m(1, 0);
		m(1, 0) = temp;
		temp = m(0, 2);
		m(0, 2) = m(2, 0);
		m(2, 0) = temp;
		temp = m(1, 2);
		m(1, 2) = m(2, 1);
		m(2, 1) = temp;
		
	}

	TMat3 getTransposed() const
	{
		const TMat3& m = *this;
		TMat3 m3;
		for(U i = 0; i < 3; i++)
		{
			for(U j = 0; j < 3; j++)
			{
				m3(i, j) = m(j, i);
			}
		}
		return m3;
	}

	void reorthogonalize()
	{
		// There are 2 methods, the standard and the Gram-Schmidt method with a 
		// twist for zAxis. This uses the 2nd. For the first see < r664
		TVec3<T> xAxis, yAxis, zAxis;
		getColumns(xAxis, yAxis, zAxis);

		xAxis.normalize();

		yAxis = yAxis - (xAxis * xAxis.dot(yAxis));
		yAxis.normalize();

		zAxis = xAxis.cross(yAxis);

		setColumns(xAxis, yAxis, zAxis);
	}

	T getDet() const
	{
		const TMat3& m = *this;
		// For the accurate method see < r664
		return m(0, 0) * (m(1, 1) * m(2, 2)
			- m(1, 2) * m(2, 1)) - m(0, 1) * (m(1, 0)
			* m(2, 2) - m(1, 2) * m(2, 0)) + m(0, 2)
			* (m(0, 1) * m(2, 1) - m(1, 1) * m(2, 0));
	}

	TMat3 getInverse() const
	{
		// Using Gramer's method Inv(A) = (1 / getDet(A)) * Adj(A)
		const TMat3& m = *this;
		TMat3 r;

		// compute determinant
		T cofactor0 = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1);
		T cofactor3 = m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2);
		T cofactor6 = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);
		T det = m(0, 0) * cofactor0 + m(1, 0) * cofactor3
			+ m(2, 0) * cofactor6;

		ANKI_ASSERT(!isZero<T>(det)); // Cannot invert det == 0

		// create adjoint matrix and multiply by 1/det to get inverse
		T invDet = 1.0 / det;
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

	void invert()
	{
		(*this) = getInverse();
	}

	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TMat3& getZero()
	{
		static const TMat3 zero(0.0);
		return zero;
	}

	static const TMat3& getIdentity()
	{
		static const TMat3 ident(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
		return ident;
	}
	/// @}

	/// @name Friends
	/// @{
	friend TMat3 operator+(T f, const TMat3& m3)
	{
		return m3 + f;
	}

	friend TMat3 operator-(T f, const TMat3& m3)
	{
		TMat3 out;
		for(U i = 0; i < 9; i++)
		{
			out[i] = f - m3[i];
		}
		return out;
	}

	friend TMat3 operator*(T f, const TMat3& m3)
	{
		return m3 * f;
	}

	friend TMat3 operator/(T f, const TMat3& m3)
	{
		TMat3 out;
		for(U i = 0; i < 9; i++)
		{
			out[i] = f / m3[i];
		}
		return out;
	}
	/// @}

private:
	/// @name Data members
	/// @{
	union
	{
		Array<T, 9> arr1;
		Array<Array<T, 3>, 3> arr2;
		T carr1[9]; ///< For easier debugging with gdb
		T carr2[3][3]; ///< For easier debugging with gdb
	};
	/// @}
};

/// F32 3x3 matrix
typedef TMat3<F32> Mat3;

static_assert(sizeof(Mat3) == sizeof(F32) * 3 * 3, "Incorrect size");

/// @}

} // end namespace anki

#endif
