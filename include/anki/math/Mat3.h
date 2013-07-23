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
		arr2[0][0] = m00;
		arr2[0][1] = m01;
		arr2[0][2] = m02;
		arr2[1][0] = m10;
		arr2[1][1] = m11;
		arr2[1][2] = m12;
		arr2[2][0] = m20;
		arr2[2][1] = m21;
		arr2[2][2] = m22;
	}

	explicit TMat3(const T arr[])
	{
		for(U i = 0; i < 9; i++)
		{
			(*this)[i] = arr[i];
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

		arr2[0][0] = T(1) - (yy + zz);
		arr2[0][1] = xy - wz;
		arr2[0][2] = xz + wy;

		arr2[1][0] = xy + wz;
		arr2[1][1] = T(1) - (xx + zz);
		arr2[1][2] = yz - wx;

		arr2[2][0] = xz - wy;
		arr2[2][1] = yz + wx;
		arr2[2][2] = T(1) - (xx + yy);
	}

	explicit TMat3(const TEuler<T>& e)
	{
		T ch, sh, ca, sa, cb, sb;
		sinCos(e.y(), sh, ch);
		sinCos(e.z(), sa, ca);
		sinCos(e.x(), sb, cb);

		arr2[0][0] = ch * ca;
		arr2[0][1] = sh * sb - ch * sa * cb;
		arr2[0][2] = ch * sa * sb + sh * cb;
		arr2[1][0] = sa;
		arr2[1][1] = ca * cb;
		arr2[1][2] = -ca * sb;
		arr2[2][0] = -sh * ca;
		arr2[2][1] = sh * sa * cb + ch * sb;
		arr2[2][2] = -sh * sa * sb + ch * cb;
	}

	explicit TMat3(const TAxisang<T>& axisang)
	{
		// Not normalized axis
		ANKI_ASSERT(isZero(1.0 - axisang.getAxis().getLength()));

		T c, s;
		sinCos(axisang.getAngle(), s, c);
		T t = 1.0 - c;

		const TVec3<T>& axis = axisang.getAxis();
		arr2[0][0] = c + axis.x() * axis.x() * t;
		arr2[1][1] = c + axis.y() * axis.y() * t;
		arr2[2][2] = c + axis.z() * axis.z() * t;

		T tmp1 = axis.x() * axis.y() * t;
		T tmp2 = axis.z() * s;
		arr2[1][0] = tmp1 + tmp2;
		arr2[0][1] = tmp1 - tmp2;
		tmp1 = axis.x() * axis.z() * t;
		tmp2 = axis.y() * s;
		arr2[2][0] = tmp1 - tmp2;
		arr2[0][2] = tmp1 + tmp2;
		tmp1 = axis.y() * axis.z() * t;
		tmp2 = axis.x() * s;
		arr2[2][1] = tmp1 + tmp2;
		arr2[1][2] = tmp1 - tmp2;
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
		c(0, 0) = arr2[0][0] * b(0, 0) + arr2[0][1] * b(1, 0)
			+ arr2[0][2] * b(2, 0);
		c(0, 1) = arr2[0][0] * b(0, 1) + arr2[0][1] * b(1, 1)
			+ arr2[0][2] * b(2, 1);
		c(0, 2) = arr2[0][0] * b(0, 2) + arr2[0][1] * b(1, 2)
			+ arr2[0][2] * b(2, 2);
		c(1, 0) = arr2[1][0] * b(0, 0) + arr2[1][1] * b(1, 0)
			+ arr2[1][2] * b(2, 0);
		c(1, 1) = arr2[1][0] * b(0, 1) + arr2[1][1] * b(1, 1)
			+ arr2[1][2] * b(2, 1);
		c(1, 2) = arr2[1][0] * b(0, 2) + arr2[1][1] * b(1, 2)
			+ arr2[1][2] * b(2, 2);
		c(2, 0) = arr2[2][0] * b(0, 0) + arr2[2][1] * b(1, 0)
			+ arr2[2][2] * b(2, 0);
		c(2, 1) = arr2[2][0] * b(0, 1) + arr2[2][1] * b(1, 1)
			+ arr2[2][2] * b(2, 1);
		c(2, 2) = arr2[2][0] * b(0, 2) + arr2[2][1] * b(1, 2)
			+ arr2[2][2] * b(2, 2);
		
		return c;
	}

	TMat3& operator*=(const TMat3& b)
	{
		(*this) = (*this) * b;
		return (*this);
	}

	TMat3 operator/(const TMat3& b) const;
	TMat3& operator/=(const TMat3& b);

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
		return TVec3<T>(
			arr2[0][0] * b.x() + arr2[0][1] * b.y() + arr2[0][2] * b.z(),
			arr2[1][0] * b.x() + arr2[1][1] * b.y() + arr2[1][2] * b.z(),
			arr2[2][0] * b.x() + arr2[2][1] * b.y() + arr2[2][2] * b.z());
	}
	/// @}

	/// @name Other
	/// @{
	void setRows(const TVec3<T>& a, const TVec3<T>& b, const TVec3<T>& c)
	{
		arr2[0][0] = a.x();
		arr2[0][1] = a.y();
		arr2[0][2] = a.z();
		arr2[1][0] = b.x();
		arr2[1][1] = b.y();
		arr2[1][2] = b.z();
		arr2[2][0] = c.x();
		arr2[2][1] = c.y();
		arr2[2][2] = c.z();
	}

	void setRow(const U i, const TVec3<T>& v)
	{
		arr2[i][0] = v.x();
		arr2[i][1] = v.y();
		arr2[i][2] = v.z();
	}

	void getRows(TVec3<T>& a, TVec3<T>& b, TVec3<T>& c) const
	{
		a.x() = arr2[0][0];
		a.y() = arr2[0][1];
		a.z() = arr2[0][2];
		b.x() = arr2[1][0];
		b.y() = arr2[1][1];
		b.z() = arr2[1][2];
		c.x() = arr2[2][0];
		c.y() = arr2[2][1];
		c.z() = arr2[2][2];
	}

	TVec3<T> getRow(const U i) const
	{
		return TVec3<T>(arr2[i][0], arr2[i][1], arr2[i][2]);
	}

	void setColumns(const TVec3<T>& a, const TVec3<T>& b, const TVec3<T>& c)
	{
		arr2[0][0] = a.x();
		arr2[1][0] = a.y();
		arr2[2][0] = a.z();
		arr2[0][1] = b.x();
		arr2[1][1] = b.y();
		arr2[2][1] = b.z();
		arr2[0][2] = c.x();
		arr2[1][2] = c.y();
		arr2[2][2] = c.z();
	}

	void setColumn(const U i, const TVec3<T>& v)
	{
		arr2[0][i] = v.x();
		arr2[1][i] = v.y();
		arr2[2][i] = v.z();
	}

	void getColumns(TVec3<T>& a, TVec3<T>& b, TVec3<T>& c) const
	{
		a.x() = arr2[0][0];
		a.y() = arr2[1][0];
		a.z() = arr2[2][0];
		b.x() = arr2[0][1];
		b.y() = arr2[1][1];
		b.z() = arr2[2][1];
		c.x() = arr2[0][2];
		c.y() = arr2[1][2];
		c.z() = arr2[2][2];
	}

	TVec3<T> getColumn(const U i) const
	{
		return TVec3<T>(arr2[0][i], arr2[1][i], arr2[2][i]);
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
		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		arr2[0][0] = 1.0;
		arr2[0][1] = 0.0;
		arr2[0][2] = 0.0;
		arr2[1][0] = 0.0;
		arr2[1][1] = costheta;
		arr2[1][2] = -sintheta;
		arr2[2][0] = 0.0;
		arr2[2][1] = sintheta;
		arr2[2][2] = costheta;
	}

	void setRotationY(const T rad)
	{
		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		arr2[0][0] = costheta;
		arr2[0][1] = 0.0;
		arr2[0][2] = sintheta;
		arr2[1][0] = 0.0;
		arr2[1][1] = 1.0;
		arr2[1][2] = 0.0;
		arr2[2][0] = -sintheta;
		arr2[2][1] = 0.0;
		arr2[2][2] = costheta;
	}

	void setRotationZ(const T rad)
	{
		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		arr2[0][0] = costheta;
		arr2[0][1] = -sintheta;
		arr2[0][2] = 0.0;
		arr2[1][0] = sintheta;
		arr2[1][1] = costheta;
		arr2[1][2] = 0.0;
		arr2[2][0] = 0.0;
		arr2[2][1] = 0.0;
		arr2[2][2] = 1.0;
	}

	/// It rotates "this" in the axis defined by the rotation AND not the
	/// world axis
	void rotateXAxis(const T rad)
	{
		// If we analize the mat3 we can extract the 3 unit vectors rotated by 
		// the mat3. The 3 rotated vectors are in mat's columns. This means 
		// that: mat3.colomn[0] == i * mat3. rotateXAxis() rotates rad angle 
		// not from i vector (aka x axis) but from the vector from colomn 0
		// NOTE: See the clean code from < r664

		T sina, cosa;
		sinCos(rad, sina, cosa);

		// zAxis = zAxis*cosa - yAxis*sina;
		arr2[0][2] = arr2[0][2] * cosa - arr2[0][1] * sina;
		arr2[1][2] = arr2[1][2] * cosa - arr2[1][1] * sina;
		arr2[2][2] = arr2[2][2] * cosa - arr2[2][1] * sina;

		// zAxis.normalize();
		F32 len = sqrt(arr2[0][2] * arr2[0][2]
			+ arr2[1][2] * arr2[1][2] + arr2[2][2] * arr2[2][2]);
		arr2[0][2] /= len;
		arr2[1][2] /= len;
		arr2[2][2] /= len;

		// yAxis = zAxis * xAxis;
		arr2[0][1] = arr2[1][2] * arr2[2][0]
			- arr2[2][2] * arr2[1][0];
		arr2[1][1] = arr2[2][2] * arr2[0][0]
			- arr2[0][2] * arr2[2][0];
		arr2[2][1] = arr2[0][2] * arr2[1][0]
			- arr2[1][2] * arr2[0][0];

		// yAxis.normalize();
	}

	/// @copybrief rotateXAxis
	void rotateYAxis(const T rad)
	{
		// NOTE: See the clean code from < r664
		T sina, cosa;
		sinCos(rad, sina, cosa);

		// zAxis = zAxis*cosa + xAxis*sina;
		arr2[0][2] = arr2[0][2] * cosa + arr2[0][0] * sina;
		arr2[1][2] = arr2[1][2] * cosa + arr2[1][0] * sina;
		arr2[2][2] = arr2[2][2] * cosa + arr2[2][0] * sina;

		// zAxis.normalize();
		F32 len = sqrt(arr2[0][2] * arr2[0][2]
			+ arr2[1][2] * arr2[1][2] + arr2[2][2] * arr2[2][2]);
		arr2[0][2] /= len;
		arr2[1][2] /= len;
		arr2[2][2] /= len;

		// xAxis = (zAxis*yAxis) * -1.0f;
		arr2[0][0] = arr2[2][2] * arr2[1][1]
			- arr2[1][2] * arr2[2][1];
		arr2[1][0] = arr2[0][2] * arr2[2][1]
			- arr2[2][2] * arr2[0][1];
		arr2[2][0] = arr2[1][2] * arr2[0][1]
			- arr2[0][2] * arr2[1][1];
			
	}

	/// @copybrief rotateXAxis
	void rotateZAxis(const T rad)
	{
		// NOTE: See the clean code from < r664
		T sina, cosa;
		sinCos(rad, sina, cosa);

		// xAxis = xAxis*cosa + yAxis*sina;
		arr2[0][0] = arr2[0][0] * cosa + arr2[0][1] * sina;
		arr2[1][0] = arr2[1][0] * cosa + arr2[1][1] * sina;
		arr2[2][0] = arr2[2][0] * cosa + arr2[2][1] * sina;

		// xAxis.normalize();
		T len = sqrt(arr2[0][0] * arr2[0][0]
			+ arr2[1][0] * arr2[1][0] + arr2[2][0] * arr2[2][0]);
		arr2[0][0] /= len;
		arr2[1][0] /= len;
		arr2[2][0] /= len;

		// yAxis = zAxis*xAxis;
		arr2[0][1] = arr2[1][2] * arr2[2][0] - arr2[2][2] * arr2[1][0];
		arr2[1][1] = arr2[2][2] * arr2[0][0] - arr2[0][2] * arr2[2][0];
		arr2[2][1] = arr2[0][2] * arr2[1][0] - arr2[1][2] * arr2[0][0];
	}

	void transpose()
	{
		T temp = arr2[0][1];
		arr2[0][1] = arr2[1][0];
		arr2[1][0] = temp;
		temp = arr2[0][2];
		arr2[0][2] = arr2[2][0];
		arr2[2][0] = temp;
		temp = arr2[1][2];
		arr2[1][2] = arr2[2][1];
		arr2[2][1] = temp;
		
	}

	TMat3 getTransposed() const
	{
		TMat3 m3;
		for(U i = 0; i < 3; i++)
		{
			for(U j = 0; j < 3; j++)
			{
				m3[i][j] = arr2[j][i];
			}
		}
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
		// For the accurate method see < r664
		return arr2[0][0] * (arr2[1][1] * arr2[2][2]
			- arr2[1][2] * arr2[2][1]) - arr2[0][1] * (arr2[1][0]
			* arr2[2][2] - arr2[1][2] * arr2[2][0]) + arr2[0][2]
			* (arr2[0][1] * arr2[2][1] - arr2[1][1] * arr2[2][0]);
	}

	TMat3 getInverse() const
	{
		// Using Gramer's method Inv(A) = (1 / getDet(A)) * Adj(A)
		TMat3 r;

		// compute determinant
		T cofactor0 = arr2[1][1] * arr2[2][2] - arr2[1][2] * arr2[2][1];
		T cofactor3 = arr2[0][2] * arr2[2][1] - arr2[0][1] * arr2[2][2];
		T cofactor6 = arr2[0][1] * arr2[1][2] - arr2[0][2] * arr2[1][1];
		T det = arr2[0][0] * cofactor0 + arr2[1][0] * cofactor3
			+ arr2[2][0] * cofactor6;

		ANKI_ASSERT(!isZero<T>(det)); // Cannot invert det == 0

		// create adjoint matrix and multiply by 1/det to get inverse
		T invDet = 1.0 / det;
		r(0, 0) = invDet * cofactor0;
		r(0, 1) = invDet * cofactor3;
		r(0, 2) = invDet * cofactor6;

		r(1, 0) = invDet * (arr2[1][2] * arr2[2][0] - arr2[1][0] * arr2[2][2]);
		r(1, 1) = invDet * (arr2[0][0] * arr2[2][2] - arr2[0][2] * arr2[2][0]);
		r(1, 2) = invDet * (arr2[0][2] * arr2[1][0] - arr2[0][0] * arr2[1][2]);

		r(2, 0) = invDet * (arr2[1][0] * arr2[2][1] - arr2[1][1] * arr2[2][0]);
		r(2, 1) = invDet * (arr2[0][1] * arr2[2][0] - arr2[0][0] * arr2[2][1]);
		r(2, 2) = invDet * (arr2[0][0] * arr2[1][1] - arr2[0][1] * arr2[1][0]);

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

	friend std::ostream& operator<<(std::ostream& s, const TMat3& m)
	{
		for(U i = 0; i < 3; i++)
		{
			for(U j = 0; j < 3; j++)
			{
				s << m(i, j) << ' ';
			}

			if(i != 2)
			{
				//s << "\n";
			}
		}
		return s;
	}
	/// @}

private:
	/// @name Data members
	/// @{
	union
	{
		Array<T, 9> arr1;
		Array<Array<T, 3>, 3> arr2;
		T carr1[9]; ///< For easy debugging with gdb
		T carr2[3][3]; ///< For easy debugging with gdb
	};
	/// @}
};

/// 3x3 Matrix. Mainly used for rotations. It includes many helpful member
/// functions. Its row major. The columns are the x,y,z axis
class Mat3
{
public:
	/// @name Constructors
	/// @{
	explicit Mat3() {};
	explicit Mat3(const F32 f);
	explicit Mat3(const F32 m00, const F32 m01, const F32 m02,
		const F32 m10, const F32 m11, const F32 m12,
		const F32 m20, const F32 m21, const F32 m22);
	explicit Mat3(const F32 arr[]);
	Mat3(const Mat3& b);
	explicit Mat3(const Quat& q); ///< Quat to Mat3. 12 muls, 12 adds
	explicit Mat3(const Euler& eu);
	explicit Mat3(const Axisang& axisang);
	/// @}

	/// @name Accessors
	/// @{
	F32& operator()(const U i, const U j);
	const F32& operator()(const U i, const U j) const;
	F32& operator[](const U i);
	const F32& operator[](const U i) const;
	/// @}

	/// @name Operators with same type
	/// @{
	Mat3& operator=(const Mat3& b);
	Mat3 operator+(const Mat3& b) const;
	Mat3& operator+=(const Mat3& b);
	Mat3 operator-(const Mat3& b) const;
	Mat3& operator-=(const Mat3& b);
	Mat3 operator*(const Mat3& b) const; ///< 27 muls, 18 adds
	Mat3& operator*=(const Mat3& b);
	Mat3 operator/(const Mat3& b) const;
	Mat3& operator/=(const Mat3& b);
	Bool operator==(const Mat3& b) const;
	Bool operator!=(const Mat3& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Mat3 operator+(const F32 f) const;
	Mat3& operator+=(const F32 f);
	Mat3 operator-(const F32 f) const;
	Mat3& operator-=(const F32 f);
	Mat3 operator*(const F32 f) const;
	Mat3& operator*=(const F32 f);
	Mat3 operator/(const F32 f) const;
	Mat3& operator/=(const F32 f);
	/// @}

	/// @name Operators with others
	/// @{

	/// Vec3(dot(row0 * b), dot(row1 * b), dot(row2 * b)). 9 muls, 6 adds
	Vec3 operator*(const Vec3& b) const;
	/// @}

	/// @name Other
	/// @{
	void setRows(const Vec3& a, const Vec3& b, const Vec3& c);
	void setRow(const U i, const Vec3& v);
	void getRows(Vec3& a, Vec3& b, Vec3& c) const;
	Vec3 getRow(const U i) const;
	void setColumns(const Vec3& a, const Vec3& b, const Vec3& c);
	void setColumn(const U i, const Vec3& v);
	void getColumns(Vec3& a, Vec3& b, Vec3& c) const;
	Vec3 getColumn(const U i) const;
	Vec3 getXAxis() const; ///< Get 1st column
	Vec3 getYAxis() const; ///< Get 2nd column
	Vec3 getZAxis() const; ///< Get 3rd column
	void setXAxis(const Vec3& v3); ///< Set 1st column
	void setYAxis(const Vec3& v3); ///< Set 2nd column
	void setZAxis(const Vec3& v3); ///< Set 3rd column
	void setRotationX(const F32 rad);
	void setRotationY(const F32 rad);
	void setRotationZ(const F32 rad);
	/// It rotates "this" in the axis defined by the rotation AND not the
	/// world axis
	void rotateXAxis(const F32 rad);
	void rotateYAxis(const F32 rad); ///< @copybrief rotateXAxis
	void rotateZAxis(const F32 rad); ///< @copybrief rotateXAxis
	void transpose();
	Mat3 getTransposed() const;
	void reorthogonalize();
	F32 getDet() const;
	void invert();
	Mat3 getInverse() const;
	void setIdentity();
	static const Mat3& getZero();
	static const Mat3& getIdentity();
	/// @}

	/// @name Friends
	/// @{
	friend Mat3 operator+(F32 f, const Mat3& m3);
	friend Mat3 operator-(F32 f, const Mat3& m3);
	friend Mat3 operator*(F32 f, const Mat3& m3);
	friend Mat3 operator/(F32 f, const Mat3& m3);
	friend std::ostream& operator<<(std::ostream& s, const Mat3& m);
	/// @}

private:
	/// @name Data members
	/// @{
	union
	{
		Array<F32, 9> arr1;
		Array<Array<F32, 3>, 3> arr2;
		F32 carr1[9]; ///< For gdb
		F32 carr2[3][3]; ///< For gdb
	};
	/// @}
};
/// @}

static_assert(sizeof(Mat3) == sizeof(F32) * 3 * 3, "Incorrect size");

} // end namespace anki

#include "anki/math/Mat3.inl.h"

#endif
