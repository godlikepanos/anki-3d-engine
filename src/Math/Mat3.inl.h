#include "MathCommon.inl.h"


#define SELF (*this)


namespace M {

//======================================================================================================================
// Accessor                                                                                                            =
//======================================================================================================================

inline float& Mat3::operator()(const uint i, const uint j)
{
	return arr2[i][j];
}

inline const float& Mat3::operator()(const uint i, const uint j) const
{
	return arr2[i][j]; 
}

inline float& Mat3::operator[](const uint i)
{
	return arr1[i];
}

inline const float& Mat3::operator[](const uint i) const
{
	return arr1[i];
}


//======================================================================================================================
// Constructors                                                                                                        =
//======================================================================================================================

// constructor [float]
inline Mat3::Mat3(float f)
{
	for(int i = 0; i < 9; i++)
	{
		SELF[i] = f;
	}
}

// float[]
inline Mat3::Mat3(float arr [])
{
	for(int i = 0; i < 9; i++)
	{
		SELF[i] = arr[i];
	}
}

// many floats
inline Mat3::Mat3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
{
	SELF(0, 0) = m00;
	SELF(0, 1) = m01;
	SELF(0, 2) = m02;
	SELF(1, 0) = m10;
	SELF(1, 1) = m11;
	SELF(1, 2) = m12;
	SELF(2, 0) = m20;
	SELF(2, 1) = m21;
	SELF(2, 2) = m22;
}

// Copy
inline Mat3::Mat3(const Mat3& b)
{
	for(int i = 0; i < 9; i++)
	{
		SELF[i] = b[i];
	}
}

// Quat
inline Mat3::Mat3(const Quat& q)
{
	RASSERT_THROW_EXCEPTION(fabs(1.0 - q.getLength()) > 0.002); // If length is > 1 + 0.002 or < 1 - 0.002 then not normalized quat

	float xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

	xs = q.x + q.x;
	ys = q.y + q.y;
	zs = q.z + q.z;
	wx = q.w * xs;
	wy = q.w * ys;
	wz = q.w * zs;
	xx = q.x * xs;
	xy = q.x * ys;
	xz = q.x * zs;
	yy = q.y * ys;
	yz = q.y * zs;
	zz = q.z * zs;

	SELF(0, 0) = 1.0 - (yy + zz);
	SELF(0, 1) = xy - wz;
	SELF(0, 2) = xz + wy;

	SELF(1, 0) = xy + wz;
	SELF(1, 1) = 1.0 - (xx + zz);
	SELF(1, 2) = yz - wx;

	SELF(2, 0) = xz - wy;
	SELF(2, 1) = yz + wx;
	SELF(2, 2) = 1.0 - (xx + yy);
}

// Euler
inline Mat3::Mat3(const Euler& e)
{
	float ch, sh, ca, sa, cb, sb;
  sinCos(e.getHeading(), sh, ch);
  sinCos(e.getAttitude(), sa, ca);
  sinCos(e.getBank(), sb, cb);

  SELF(0, 0) = ch * ca;
  SELF(0, 1) = sh * sb - ch * sa * cb;
  SELF(0, 2) = ch * sa * sb + sh * cb;
  SELF(1, 0) = sa;
  SELF(1, 1) = ca * cb;
  SELF(1, 2) = -ca * sb;
  SELF(2, 0) = -sh * ca;
  SELF(2, 1) = sh * sa * cb + ch * sb;
  SELF(2, 2) = -sh * sa * sb + ch * cb;
}

// Axisang
inline Mat3::Mat3(const Axisang& axisang)
{
	RASSERT_THROW_EXCEPTION(!isZero(1.0 - axisang.axis.getLength())); // Not normalized axis

	float c, s;
	sinCos(axisang.ang, s, c);
	float t = 1.0 - c;

	const Vec3& axis = axisang.axis;
	SELF(0, 0) = c + axis.x() * axis.x() * t;
	SELF(1, 1) = c + axis.y() * axis.y() * t;
	SELF(2, 2) = c + axis.z() * axis.z() * t;

	float tmp1 = axis.x() * axis.y() * t;
	float tmp2 = axis.z() * s;
	SELF(1, 0) = tmp1 + tmp2;
	SELF(0, 1) = tmp1 - tmp2;
	tmp1 = axis.x() * axis.z() * t;
	tmp2 = axis.y() * s;
	SELF(2, 0) = tmp1 - tmp2;
	SELF(0, 2) = tmp1 + tmp2;    tmp1 = axis.y() * axis.z() * t;
	tmp2 = axis.x() * s;
	SELF(2, 1) = tmp1 + tmp2;
	SELF(1, 2) = tmp1 - tmp2;
}


//======================================================================================================================
// Operators with same                                                                                                 =
//======================================================================================================================

// +
inline Mat3 Mat3::operator+(const Mat3& b) const
{
	Mat3 c;
	for(int i = 0; i < 9; i++)
	{
		c[i] = SELF[i] + b[i];
	}
	return c;
}

// +=
inline Mat3& Mat3::operator+=(const Mat3& b)
{
	for(int i = 0; i < 9; i++)
	{
		SELF[i] += b[i];
	}
	return SELF;
}

// -
inline Mat3 Mat3::operator-(const Mat3& b) const
{
	Mat3 c;
	for(int i = 0; i < 9; i++)
	{
		c[i] = SELF[i] - b[i];
	}
	return c;
}

// -=
inline Mat3& Mat3::operator-=(const Mat3& b)
{
	for(int i = 0; i < 9; i++)
	{
		SELF[i] -= b[i];
	}
	return SELF;
}

// *
inline Mat3 Mat3::operator*(const Mat3& b) const
{
	Mat3 c;
	c(0, 0) = SELF(0, 0) * b(0, 0) + SELF(0, 1) * b(1, 0) + SELF(0, 2) * b(2, 0);
	c(0, 1) = SELF(0, 0) * b(0, 1) + SELF(0, 1) * b(1, 1) + SELF(0, 2) * b(2, 1);
	c(0, 2) = SELF(0, 0) * b(0, 2) + SELF(0, 1) * b(1, 2) + SELF(0, 2) * b(2, 2);
	c(1, 0) = SELF(1, 0) * b(0, 0) + SELF(1, 1) * b(1, 0) + SELF(1, 2) * b(2, 0);
	c(1, 1) = SELF(1, 0) * b(0, 1) + SELF(1, 1) * b(1, 1) + SELF(1, 2) * b(2, 1);
	c(1, 2) = SELF(1, 0) * b(0, 2) + SELF(1, 1) * b(1, 2) + SELF(1, 2) * b(2, 2);
	c(2, 0) = SELF(2, 0) * b(0, 0) + SELF(2, 1) * b(1, 0) + SELF(2, 2) * b(2, 0);
	c(2, 1) = SELF(2, 0) * b(0, 1) + SELF(2, 1) * b(1, 1) + SELF(2, 2) * b(2, 1);
	c(2, 2) = SELF(2, 0) * b(0, 2) + SELF(2, 1) * b(1, 2) + SELF(2, 2) * b(2, 2);
	return c;
}

// *=
inline Mat3& Mat3::operator*=(const Mat3& b)
{
	SELF = SELF * b;
	return SELF;
}

// ==
inline bool Mat3::operator==(const Mat3& b) const
{
	for(int i = 0; i < 9; i++)
	{
		if(!isZero(SELF[i]-b[i]))
		{
			return false;
		}
	}
	return true;
}

// !=
inline bool Mat3::operator!=(const Mat3& b) const
{
	for(int i = 0; i < 9; i++)
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

// 3x3 + float
inline Mat3 Mat3::operator+(float f) const
{
	Mat3 c;
	for(uint i = 0; i < 9; i++)
	{
		c[i] = SELF[i] + f;
	}
	return c;
}

// float + 3x3
inline Mat3 operator+(float f, const Mat3& m3)
{
	return m3 + f;
}

// 3x3 += float
inline Mat3& Mat3::operator+=(float f)
{
	for(uint i = 0; i < 9; i++)
	{
		SELF[i] += f;
	}
	return SELF;
}

// 3x3 - float
inline Mat3 Mat3::operator-(float f) const
{
	Mat3 c;
	for(uint i = 0; i < 9; i++)
	{
		c[i] = SELF[i] - f;
	}
	return c;
}

// float - 3x3
inline Mat3 operator-(float f, const Mat3& m3)
{
	Mat3 out;
	for(uint i = 0; i < 9; i++)
	{
		out[i] = f - m3[i];
	}
	return out;
}

// 3x3 -= float
inline Mat3& Mat3::operator-=(float f)
{
	for(uint i = 0; i < 9; i++)
	{
		SELF[i] -= f;
	}
	return SELF;
}

// 3x3 * float
inline Mat3 Mat3::operator*(float f) const
{
	Mat3 c;
	for(uint i = 0; i < 9; i++)
	{
		c[i] = SELF[i] * f;
	}
	return c;
}

// float * 3x3
inline Mat3 operator*(float f, const Mat3& m3)
{
	Mat3 out;
	for(uint i = 0; i < 9; i++)
	{
		out[i] = f * m3[i];
	}
	return out;
}

// 3x3 *= float
inline Mat3& Mat3::operator*=(float f)
{
	for(uint i = 0; i < 9; i++)
	{
		SELF[i] *= f;
	}
	return SELF;
}

// 3x3 / float
inline Mat3 Mat3::operator/(float f) const
{
	Mat3 c;
	for(uint i = 0; i < 9; i++)
	{
		c[i] = SELF[i] / f;
	}
	return c;
}

// float / 3x3
inline Mat3 operator/(float f, const Mat3& m3)
{
	Mat3 out;
	for(uint i = 0; i < 9; i++)
	{
		out[i] = f / m3[i];
	}
	return out;
}

// 3x3 / float (self)
inline Mat3& Mat3::operator/=(float f)
{
	for(uint i = 0; i < 9; i++)
	{
		SELF[i] /= f;
	}
	return SELF;
}


//======================================================================================================================
// Operators with other                                                                                                =
//======================================================================================================================

// 3x3 * vec3 (cross products with rows)
inline Vec3 Mat3::operator*(const Vec3& b) const
{
	return Vec3(
		SELF(0, 0) * b.x() + SELF(0, 1) * b.y() + SELF(0, 2) * b.z(),
		SELF(1, 0) * b.x() + SELF(1, 1) * b.y() + SELF(1, 2) * b.z(),
		SELF(2, 0) * b.x() + SELF(2, 1) * b.y() + SELF(2, 2) * b.z()
	);
}


//======================================================================================================================
// Other                                                                                                               =
//======================================================================================================================

// setRows
inline void Mat3::setRows(const Vec3& a, const Vec3& b, const Vec3& c)
{
	SELF(0, 0) = a.x();
	SELF(0, 1) = a.y();
	SELF(0, 2) = a.z();
	SELF(1, 0) = b.x();
	SELF(1, 1) = b.y();
	SELF(1, 2) = b.z();
	SELF(2, 0) = c.x();
	SELF(2, 1) = c.y();
	SELF(2, 2) = c.z();
}

// setColumns
inline void Mat3::setColumns(const Vec3& a, const Vec3& b, const Vec3& c)
{
	SELF(0, 0) = a.x();
	SELF(1, 0) = a.y();
	SELF(2, 0) = a.z();
	SELF(0, 1) = b.x();
	SELF(1, 1) = b.y();
	SELF(2, 1) = b.z();
	SELF(0, 2) = c.x();
	SELF(1, 2) = c.y();
	SELF(2, 2) = c.z();
}

// getRows
inline void Mat3::getRows(Vec3& a, Vec3& b, Vec3& c) const
{
	a.x() = SELF(0, 0);
	a.y() = SELF(0, 1);
	a.z() = SELF(0, 2);
	b.x() = SELF(1, 0);
	b.y() = SELF(1, 1);
	b.z() = SELF(1, 2);
	c.x() = SELF(2, 0);
	c.y() = SELF(2, 1);
	c.z() = SELF(2, 2);
}

// getColumns
inline void Mat3::getColumns(Vec3& a, Vec3& b, Vec3& c) const
{
	a.x() = SELF(0, 0);
	a.y() = SELF(1, 0);
	a.z() = SELF(2, 0);
	b.x() = SELF(0, 1);
	b.y() = SELF(1, 1);
	b.z() = SELF(2, 1);
	c.x() = SELF(0, 2);
	c.y() = SELF(1, 2);
	c.z() = SELF(2, 2);
}

// setRow
inline void Mat3::setRow(const uint i, const Vec3& v)
{
	SELF(i, 0) = v.x();
	SELF(i, 1) = v.y();
	SELF(i, 2) = v.z();
}

// getRow
inline Vec3 Mat3::getRow(const uint i) const
{
	return Vec3(SELF(i, 0), SELF(i, 1), SELF(i, 2));
}

// setColumn
inline void Mat3::setColumn(const uint i, const Vec3& v)
{
	SELF(0, i) = v.x();
	SELF(1, i) = v.y();
	SELF(2, i) = v.z();
}

// getColumn
inline Vec3 Mat3::getColumn(const uint i) const
{
	return Vec3(SELF(0,i), SELF(1,i), SELF(2,i));
}

// getXAxis
inline Vec3 Mat3::getXAxis() const
{
	return getColumn(0);
}

// getYAxis
inline Vec3 Mat3::getYAxis() const
{
	return getColumn(1);
}

// getZAxis
inline Vec3 Mat3::getZAxis() const
{
	return getColumn(2);
}

// setXAxis
inline void Mat3::setXAxis(const Vec3& v3)
{
	setColumn(0, v3);
}

// setYAxis
inline void Mat3::setYAxis(const Vec3& v3)
{
	setColumn(1, v3);
}

// setZAxis
inline void Mat3::setZAxis(const Vec3& v3)
{
	setColumn(2, v3);
}

// setRotationX
inline void Mat3::setRotationX(float rad)
{
	float sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	SELF(0, 0) = 1.0;
	SELF(0, 1) = 0.0;
	SELF(0, 2) = 0.0;
	SELF(1, 0) = 0.0;
	SELF(1, 1) = costheta;
	SELF(1, 2) = -sintheta;
	SELF(2, 0) = 0.0;
	SELF(2, 1) = sintheta;
	SELF(2, 2) = costheta;
}

// setRotationY
inline void Mat3::setRotationY(float rad)
{
	float sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	SELF(0, 0) = costheta;
	SELF(0, 1) = 0.0;
	SELF(0, 2) = sintheta;
	SELF(1, 0) = 0.0;
	SELF(1, 1) = 1.0;
	SELF(1, 2) = 0.0;
	SELF(2, 0) = -sintheta;
	SELF(2, 1) = 0.0;
	SELF(2, 2) = costheta;
}

// loadRotationZ
inline void Mat3::setRotationZ(float rad)
{
	float sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	SELF(0, 0) = costheta;
	SELF(0, 1) = -sintheta;
	SELF(0, 2) = 0.0;
	SELF(1, 0) = sintheta;
	SELF(1, 1) = costheta;
	SELF(1, 2) = 0.0;
	SELF(2, 0) = 0.0;
	SELF(2, 1) = 0.0;
	SELF(2, 2) = 1.0;
}

// rotateXAxis
/*
 * the slow code is in comments and above the comments the optimized one
 * If we analize the mat3 we can extract the 3 unit vectors rotated by the mat3. The 3 rotated vectors are in mat's
 * columns. This means that: mat3.colomn[0] == i*mat3. rotateXAxis() rotates rad angle not from i vector (aka x axis)
 * but from the vector from colomn 0
 */
inline void Mat3::rotateXAxis(float rad)
{
	float sina, cosa;
	sinCos(rad, sina, cosa);

	/*Vec3 xAxis, yAxis, zAxis;
	getColumns(xAxis, yAxis, zAxis);*/

	// zAxis = zAxis*cosa - yAxis*sina;
	SELF(0, 2) = SELF(0, 2) * cosa - SELF(0, 1) * sina;
	SELF(1, 2) = SELF(1, 2) * cosa - SELF(1, 1) * sina;
	SELF(2, 2) = SELF(2, 2) * cosa - SELF(2, 1) * sina;

	// zAxis.normalize();
	float len = sqrt(SELF(0, 2) * SELF(0, 2) + SELF(1, 2) * SELF(1, 2) + SELF(2, 2) * SELF(2, 2));
	SELF(0, 2) /= len;
	SELF(1, 2) /= len;
	SELF(2, 2) /= len;

	// yAxis = zAxis * xAxis;
	SELF(0, 1) = SELF(1, 2) * SELF(2, 0) - SELF(2, 2) * SELF(1, 0);
	SELF(1, 1) = SELF(2, 2) * SELF(0, 0) - SELF(0, 2) * SELF(2, 0);
	SELF(2, 1) = SELF(0, 2) * SELF(1, 0) - SELF(1, 2) * SELF(0, 0);

	// yAxis.normalize();
	/*len = invSqrt(SELF(0, 1) * SELF(0, 1) + SELF(1, 1) * SELF(1, 1) + SELF(2, 1) * SELF(2, 1));
	SELF(0, 1) *= len;
	SELF(1, 1) *= len;
	SELF(2, 1) *= len;*/

	// setColumns(xAxis, yAxis, zAxis);

}

// rotateYAxis
inline void Mat3::rotateYAxis(float rad)
{
	float sina, cosa;
	sinCos(rad, sina, cosa);

	/*Vec3 xAxis, yAxis, zAxis;
	getColumns(xAxis, yAxis, zAxis);*/

	// zAxis = zAxis*cosa + xAxis*sina;
	SELF(0, 2) = SELF(0, 2)*cosa + SELF(0, 0)*sina;
	SELF(1, 2) = SELF(1, 2)*cosa + SELF(1, 0)*sina;
	SELF(2, 2) = SELF(2, 2)*cosa + SELF(2, 0)*sina;

	// zAxis.normalize();
	float len = sqrt(SELF(0, 2) * SELF(0, 2) + SELF(1, 2) * SELF(1, 2) + SELF(2, 2) * SELF(2, 2));
	SELF(0, 2) /= len;
	SELF(1, 2) /= len;
	SELF(2, 2) /= len;

	// xAxis = (zAxis*yAxis) * -1.0f;
	SELF(0, 0) = SELF(2, 2) * SELF(1, 1) - SELF(1, 2) * SELF(2, 1);
	SELF(1, 0) = SELF(0, 2) * SELF(2, 1) - SELF(2, 2) * SELF(0, 1);
	SELF(2, 0) = SELF(1, 2) * SELF(0, 1) - SELF(0, 2) * SELF(1, 1);

	// xAxis.normalize();
	/*len = invSqrt(SELF(0, 0) * SELF(0, 0) + SELF(1, 0) * SELF(1, 0) + SELF(2, 0) * SELF(2, 0));
	SELF(0, 0) *= len;
	SELF(1, 0) *= len;
	SELF(2, 0) *= len;*/

	// setColumns(xAxis, yAxis, zAxis);
}


// rotateZAxis
inline void Mat3::rotateZAxis(float rad)
{
	float sina, cosa;
	sinCos(rad, sina, cosa);

	/*Vec3 xAxis, yAxis, zAxis;
	getColumns(xAxis, yAxis, zAxis);*/

	// xAxis = xAxis*cosa + yAxis*sina;
	SELF(0, 0) = SELF(0, 0)*cosa + SELF(0, 1)*sina;
	SELF(1, 0) = SELF(1, 0)*cosa + SELF(1, 1)*sina;
	SELF(2, 0) = SELF(2, 0)*cosa + SELF(2, 1)*sina;

	// xAxis.normalize();
	float len = sqrt(SELF(0, 0) * SELF(0, 0) + SELF(1, 0) * SELF(1, 0) + SELF(2, 0) * SELF(2, 0));
	SELF(0, 0) /= len;
	SELF(1, 0) /= len;
	SELF(2, 0) /= len;

	// yAxis = zAxis*xAxis;
	SELF(0, 1) = SELF(1, 2) * SELF(2, 0) - SELF(2, 2) * SELF(1, 0);
	SELF(1, 1) = SELF(2, 2) * SELF(0, 0) - SELF(0, 2) * SELF(2, 0);
	SELF(2, 1) = SELF(0, 2) * SELF(1, 0) - SELF(1, 2) * SELF(0, 0);

	// yAxis.normalize();
	/*len = invSqrt(SELF(0, 1) * SELF(0, 1) + SELF(1, 1) * SELF(1, 1) + SELF(2, 1) * SELF(2, 1));
	SELF(0, 1) *= len;
	SELF(1, 1) *= len;
	SELF(2, 1) *= len;*/

	//setColumns(xAxis, yAxis, zAxis);
}

// transpose
inline void Mat3::transpose()
{
	float temp = SELF(0, 1);
	SELF(0, 1) = SELF(1, 0);
	SELF(1, 0) = temp;
	temp = SELF(0, 2);
	SELF(0, 2) = SELF(2, 0);
	SELF(2, 0) = temp;
	temp = SELF(1, 2);
	SELF(1, 2) = SELF(2, 1);
	SELF(2, 1) = temp;
}

// transposed
inline Mat3 Mat3::getTransposed() const
{
	Mat3 m3;
	m3[0] = SELF[0];
	m3[1] = SELF[3];
	m3[2] = SELF[6];
	m3[3] = SELF[1];
	m3[4] = SELF[4];
	m3[5] = SELF[7];
	m3[6] = SELF[2];
	m3[7] = SELF[5];
	m3[8] = SELF[8];
	return m3;
}

// reorthogonalize
inline void Mat3::reorthogonalize()
{
	// method 1: standard orthogonalization method
	/*Mat3 correction_m3 =
	(
		(Mat3::ident * 3.0f) -
		(SELF * SELF.transposed())
	) * 0.5f;

	SELF = correction_m3 * SELF;*/

	// method 2: Gram-Schmidt method with a twist for zAxis
	Vec3 xAxis, yAxis, zAxis;
	getColumns(xAxis, yAxis, zAxis);

	xAxis.normalize();

	yAxis = yAxis - (xAxis * xAxis.dot(yAxis));
	yAxis.normalize();

	zAxis = xAxis.cross(yAxis);

	setColumns(xAxis, yAxis, zAxis);
}

// Determinant
inline float Mat3::getDet() const
{
	/* accurate method:
	return SELF(0, 0) * SELF(1, 1) * SELF(2, 2) + SELF(0, 1) * SELF(1, 2) * SELF(2, 0) + SELF(0, 2) * SELF(1, 0) * SELF(2, 1)
	- SELF(0, 0) * SELF(1, 2) * SELF(2, 1) - SELF(0, 1) * SELF(1, 0) * SELF(2, 2) - SELF(0, 2) * SELF(1, 1) * SELF(2, 0);*/
	return SELF(0, 0)*(SELF(1, 1) * SELF(2, 2) - SELF(1, 2) * SELF(2, 1)) -
	SELF(0, 1)*(SELF(1, 0) * SELF(2, 2) - SELF(1, 2) * SELF(2, 0)) +
	SELF(0, 2)*(SELF(0, 1) * SELF(2, 1) - SELF(1, 1) * SELF(2, 0));
}

// getInverse
// using Gramer's method (Inv(A) = (1/getDet(A)) * Adj(A))
inline Mat3 Mat3::getInverse() const
{
	Mat3 r;

	// compute determinant
	float cofactor0 = SELF(1, 1) * SELF(2, 2) - SELF(1, 2) * SELF(2, 1);
	float cofactor3 = SELF(0, 2) * SELF(2, 1) - SELF(0, 1) * SELF(2, 2);
	float cofactor6 = SELF(0, 1) * SELF(1, 2) - SELF(0, 2) * SELF(1, 1);
	float det = SELF(0, 0) * cofactor0 + SELF(1, 0) * cofactor3 + SELF(2, 0) * cofactor6;

	RASSERT_THROW_EXCEPTION(isZero(det)); // Cannot invert det == 0

	// create adjoint matrix and multiply by 1/det to get inverse
	float invDet = 1.0 / det;
	r(0, 0) = invDet * cofactor0;
	r(0, 1) = invDet * cofactor3;
	r(0, 2) = invDet * cofactor6;

	r(1, 0) = invDet * (SELF(1, 2) * SELF(2, 0) - SELF(1, 0) * SELF(2, 2));
	r(1, 1) = invDet * (SELF(0, 0) * SELF(2, 2) - SELF(0, 2) * SELF(2, 0));
	r(1, 2) = invDet * (SELF(0, 2) * SELF(1, 0) - SELF(0, 0) * SELF(1, 2));

	r(2, 0) = invDet * (SELF(1, 0) * SELF(2, 1) - SELF(1, 1) * SELF(2, 0));
	r(2, 1) = invDet * (SELF(0, 1) * SELF(2, 0) - SELF(0, 0) * SELF(2, 1));
	r(2, 2) = invDet * (SELF(0, 0) * SELF(1, 1) - SELF(0, 1) * SELF(1, 0));

	return r;
}

// setIdentity
inline void Mat3::setIdentity()
{
	SELF = getIdentity();
}

// invert
// see above
inline void Mat3::invert()
{
	SELF = getInverse();
}

// getZero
inline const Mat3& Mat3::getZero()
{
	static Mat3 zero(0.0);
	return zero;
}

// getIdentity
inline const Mat3& Mat3::getIdentity()
{
	static Mat3 ident(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	return ident;
}

// print
inline std::ostream& operator<<(std::ostream& s, const Mat3& m)
{
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
			s << m(i, j) << ' ';

		if(i != 2)
			s << "\n";
	}
	return s;
}

} // end namespace
