#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// constructor [F32]
inline Mat3::Mat3(const F32 f)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] = f;
	}
}

// F32[]
inline Mat3::Mat3(const F32 arr [])
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] = arr[i];
	}
}

// many F32s
inline Mat3::Mat3(const F32 m00, const F32 m01, const F32 m02,
	const F32 m10, const F32 m11, const F32 m12,
	const F32 m20, const F32 m21, const F32 m22)
{
	(*this)(0, 0) = m00;
	(*this)(0, 1) = m01;
	(*this)(0, 2) = m02;
	(*this)(1, 0) = m10;
	(*this)(1, 1) = m11;
	(*this)(1, 2) = m12;
	(*this)(2, 0) = m20;
	(*this)(2, 1) = m21;
	(*this)(2, 2) = m22;
}

// Copy
inline Mat3::Mat3(const Mat3& b)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] = b[i];
	}
}

// Quat
inline Mat3::Mat3(const Quat& q)
{
	// If length is > 1 + 0.002 or < 1 - 0.002 then not normalized quat
	ANKI_ASSERT(fabs(1.0 - q.getLength()) <= 0.002);

	F32 xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

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

	(*this)(0, 0) = 1.0 - (yy + zz);
	(*this)(0, 1) = xy - wz;
	(*this)(0, 2) = xz + wy;

	(*this)(1, 0) = xy + wz;
	(*this)(1, 1) = 1.0 - (xx + zz);
	(*this)(1, 2) = yz - wx;

	(*this)(2, 0) = xz - wy;
	(*this)(2, 1) = yz + wx;
	(*this)(2, 2) = 1.0 - (xx + yy);
}

// Euler
inline Mat3::Mat3(const Euler& e)
{
	F32 ch, sh, ca, sa, cb, sb;
	sinCos(e.y(), sh, ch);
	sinCos(e.z(), sa, ca);
	sinCos(e.x(), sb, cb);

	(*this)(0, 0) = ch * ca;
	(*this)(0, 1) = sh * sb - ch * sa * cb;
	(*this)(0, 2) = ch * sa * sb + sh * cb;
	(*this)(1, 0) = sa;
	(*this)(1, 1) = ca * cb;
	(*this)(1, 2) = -ca * sb;
	(*this)(2, 0) = -sh * ca;
	(*this)(2, 1) = sh * sa * cb + ch * sb;
	(*this)(2, 2) = -sh * sa * sb + ch * cb;
}

// Axisang
inline Mat3::Mat3(const Axisang& axisang)
{
	// Not normalized axis
	ANKI_ASSERT(isZero(1.0 - axisang.getAxis().getLength()));

	F32 c, s;
	sinCos(axisang.getAngle(), s, c);
	F32 t = 1.0 - c;

	const Vec3& axis = axisang.getAxis();
	(*this)(0, 0) = c + axis.x() * axis.x() * t;
	(*this)(1, 1) = c + axis.y() * axis.y() * t;
	(*this)(2, 2) = c + axis.z() * axis.z() * t;

	F32 tmp1 = axis.x() * axis.y() * t;
	F32 tmp2 = axis.z() * s;
	(*this)(1, 0) = tmp1 + tmp2;
	(*this)(0, 1) = tmp1 - tmp2;
	tmp1 = axis.x() * axis.z() * t;
	tmp2 = axis.y() * s;
	(*this)(2, 0) = tmp1 - tmp2;
	(*this)(0, 2) = tmp1 + tmp2;
	tmp1 = axis.y() * axis.z() * t;
	tmp2 = axis.x() * s;
	(*this)(2, 1) = tmp1 + tmp2;
	(*this)(1, 2) = tmp1 - tmp2;
}

//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline F32& Mat3::operator()(const U i, const U j)
{
	return arr2[i][j];
}

inline const F32& Mat3::operator()(const U i, const U j) const
{
	return arr2[i][j];
}

inline F32& Mat3::operator[](const U i)
{
	return arr1[i];
}

inline const F32& Mat3::operator[](const U i) const
{
	return arr1[i];
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Mat3& Mat3::operator=(const Mat3& b)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] = b[i];
	}
	return (*this);
}

// +
inline Mat3 Mat3::operator+(const Mat3& b) const
{
	Mat3 c;
	for(U i = 0; i < 9; i++)
	{
		c[i] = (*this)[i] + b[i];
	}
	return c;
}

// +=
inline Mat3& Mat3::operator+=(const Mat3& b)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] += b[i];
	}
	return (*this);
}

// -
inline Mat3 Mat3::operator-(const Mat3& b) const
{
	Mat3 c;
	for(U i = 0; i < 9; i++)
	{
		c[i] = (*this)[i] - b[i];
	}
	return c;
}

// -=
inline Mat3& Mat3::operator-=(const Mat3& b)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] -= b[i];
	}
	return (*this);
}

// *
inline Mat3 Mat3::operator*(const Mat3& b) const
{
	Mat3 c;
	c(0, 0) = (*this)(0, 0) * b(0, 0) + (*this)(0, 1) * b(1, 0)
		+ (*this)(0, 2) * b(2, 0);
	c(0, 1) = (*this)(0, 0) * b(0, 1) + (*this)(0, 1) * b(1, 1)
		+ (*this)(0, 2) * b(2, 1);
	c(0, 2) = (*this)(0, 0) * b(0, 2) + (*this)(0, 1) * b(1, 2)
		+ (*this)(0, 2) * b(2, 2);
	c(1, 0) = (*this)(1, 0) * b(0, 0) + (*this)(1, 1) * b(1, 0)
		+ (*this)(1, 2) * b(2, 0);
	c(1, 1) = (*this)(1, 0) * b(0, 1) + (*this)(1, 1) * b(1, 1)
		+ (*this)(1, 2) * b(2, 1);
	c(1, 2) = (*this)(1, 0) * b(0, 2) + (*this)(1, 1) * b(1, 2)
		+ (*this)(1, 2) * b(2, 2);
	c(2, 0) = (*this)(2, 0) * b(0, 0) + (*this)(2, 1) * b(1, 0)
		+ (*this)(2, 2) * b(2, 0);
	c(2, 1) = (*this)(2, 0) * b(0, 1) + (*this)(2, 1) * b(1, 1)
		+ (*this)(2, 2) * b(2, 1);
	c(2, 2) = (*this)(2, 0) * b(0, 2) + (*this)(2, 1) * b(1, 2)
		+ (*this)(2, 2) * b(2, 2);
	return c;
}

// *=
inline Mat3& Mat3::operator*=(const Mat3& b)
{
	(*this) = (*this) * b;
	return (*this);
}

// ==
inline Bool Mat3::operator==(const Mat3& b) const
{
	for(U i = 0; i < 9; i++)
	{
		if(!isZero((*this)[i] - b[i]))
		{
			return false;
		}
	}
	return true;
}

// !=
inline Bool Mat3::operator!=(const Mat3& b) const
{
	for(U i = 0; i < 9; i++)
	{
		if(!isZero((*this)[i] - b[i]))
		{
			return true;
		}
	}
	return false;
}

//==============================================================================
// Operators with F32                                                        =
//==============================================================================

// 3x3 + F32
inline Mat3 Mat3::operator+(const F32 f) const
{
	Mat3 c;
	for(U i = 0; i < 9; i++)
	{
		c[i] = (*this)[i] + f;
	}
	return c;
}

// 3x3 += F32
inline Mat3& Mat3::operator+=(const F32 f)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] += f;
	}
	return (*this);
}

// 3x3 - F32
inline Mat3 Mat3::operator-(const F32 f) const
{
	Mat3 c;
	for(U i = 0; i < 9; i++)
	{
		c[i] = (*this)[i] - f;
	}
	return c;
}

// 3x3 -= F32
inline Mat3& Mat3::operator-=(const F32 f)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] -= f;
	}
	return (*this);
}

// 3x3 * F32
inline Mat3 Mat3::operator*(const F32 f) const
{
	Mat3 c;
	for(U i = 0; i < 9; i++)
	{
		c[i] = (*this)[i] * f;
	}
	return c;
}

// 3x3 *= F32
inline Mat3& Mat3::operator*=(const F32 f)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] *= f;
	}
	return (*this);
}

// 3x3 / F32
inline Mat3 Mat3::operator/(const F32 f) const
{
	Mat3 c;
	for(U i = 0; i < 9; i++)
	{
		c[i] = (*this)[i] / f;
	}
	return c;
}

// 3x3 / F32 (self)
inline Mat3& Mat3::operator/=(const F32 f)
{
	for(U i = 0; i < 9; i++)
	{
		(*this)[i] /= f;
	}
	return (*this);
}

//==============================================================================
// Operators with other                                                        =
//==============================================================================

// 3x3 * vec3 (cross products with rows)
inline Vec3 Mat3::operator*(const Vec3& b) const
{
	return Vec3(
		(*this)(0, 0) * b.x() + (*this)(0, 1) * b.y() + (*this)(0, 2) * b.z(),
		(*this)(1, 0) * b.x() + (*this)(1, 1) * b.y() + (*this)(1, 2) * b.z(),
		(*this)(2, 0) * b.x() + (*this)(2, 1) * b.y() + (*this)(2, 2) * b.z());
}

//==============================================================================
// Other                                                                       =
//==============================================================================

// setRows
inline void Mat3::setRows(const Vec3& a, const Vec3& b, const Vec3& c)
{
	(*this)(0, 0) = a.x();
	(*this)(0, 1) = a.y();
	(*this)(0, 2) = a.z();
	(*this)(1, 0) = b.x();
	(*this)(1, 1) = b.y();
	(*this)(1, 2) = b.z();
	(*this)(2, 0) = c.x();
	(*this)(2, 1) = c.y();
	(*this)(2, 2) = c.z();
}

// setColumns
inline void Mat3::setColumns(const Vec3& a, const Vec3& b, const Vec3& c)
{
	(*this)(0, 0) = a.x();
	(*this)(1, 0) = a.y();
	(*this)(2, 0) = a.z();
	(*this)(0, 1) = b.x();
	(*this)(1, 1) = b.y();
	(*this)(2, 1) = b.z();
	(*this)(0, 2) = c.x();
	(*this)(1, 2) = c.y();
	(*this)(2, 2) = c.z();
}

// getRows
inline void Mat3::getRows(Vec3& a, Vec3& b, Vec3& c) const
{
	a.x() = (*this)(0, 0);
	a.y() = (*this)(0, 1);
	a.z() = (*this)(0, 2);
	b.x() = (*this)(1, 0);
	b.y() = (*this)(1, 1);
	b.z() = (*this)(1, 2);
	c.x() = (*this)(2, 0);
	c.y() = (*this)(2, 1);
	c.z() = (*this)(2, 2);
}

// getColumns
inline void Mat3::getColumns(Vec3& a, Vec3& b, Vec3& c) const
{
	a.x() = (*this)(0, 0);
	a.y() = (*this)(1, 0);
	a.z() = (*this)(2, 0);
	b.x() = (*this)(0, 1);
	b.y() = (*this)(1, 1);
	b.z() = (*this)(2, 1);
	c.x() = (*this)(0, 2);
	c.y() = (*this)(1, 2);
	c.z() = (*this)(2, 2);
}

// setRow
inline void Mat3::setRow(const U i, const Vec3& v)
{
	(*this)(i, 0) = v.x();
	(*this)(i, 1) = v.y();
	(*this)(i, 2) = v.z();
}

// getRow
inline Vec3 Mat3::getRow(const U i) const
{
	return Vec3((*this)(i, 0), (*this)(i, 1), (*this)(i, 2));
}

// setColumn
inline void Mat3::setColumn(const U i, const Vec3& v)
{
	(*this)(0, i) = v.x();
	(*this)(1, i) = v.y();
	(*this)(2, i) = v.z();
}

// getColumn
inline Vec3 Mat3::getColumn(const U i) const
{
	return Vec3((*this)(0, i), (*this)(1, i), (*this)(2, i));
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
inline void Mat3::setRotationX(const F32 rad)
{
	F32 sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	(*this)(0, 0) = 1.0;
	(*this)(0, 1) = 0.0;
	(*this)(0, 2) = 0.0;
	(*this)(1, 0) = 0.0;
	(*this)(1, 1) = costheta;
	(*this)(1, 2) = -sintheta;
	(*this)(2, 0) = 0.0;
	(*this)(2, 1) = sintheta;
	(*this)(2, 2) = costheta;
}

// setRotationY
inline void Mat3::setRotationY(const F32 rad)
{
	F32 sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	(*this)(0, 0) = costheta;
	(*this)(0, 1) = 0.0;
	(*this)(0, 2) = sintheta;
	(*this)(1, 0) = 0.0;
	(*this)(1, 1) = 1.0;
	(*this)(1, 2) = 0.0;
	(*this)(2, 0) = -sintheta;
	(*this)(2, 1) = 0.0;
	(*this)(2, 2) = costheta;
}

// loadRotationZ
inline void Mat3::setRotationZ(const F32 rad)
{
	F32 sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	(*this)(0, 0) = costheta;
	(*this)(0, 1) = -sintheta;
	(*this)(0, 2) = 0.0;
	(*this)(1, 0) = sintheta;
	(*this)(1, 1) = costheta;
	(*this)(1, 2) = 0.0;
	(*this)(2, 0) = 0.0;
	(*this)(2, 1) = 0.0;
	(*this)(2, 2) = 1.0;
}

// rotateXAxis
inline void Mat3::rotateXAxis(const F32 rad)
{
	// If we analize the mat3 we can extract the 3 unit vectors rotated by the 
	// mat3. The 3 rotated vectors are in mat's columns. This means that:
	// mat3.colomn[0] == i * mat3. rotateXAxis() rotates rad angle not from i 
	// vector (aka x axis) but from the vector from colomn 0
	// NOTE: See the clean code from < r664

	F32 sina, cosa;
	sinCos(rad, sina, cosa);

	// zAxis = zAxis*cosa - yAxis*sina;
	(*this)(0, 2) = (*this)(0, 2) * cosa - (*this)(0, 1) * sina;
	(*this)(1, 2) = (*this)(1, 2) * cosa - (*this)(1, 1) * sina;
	(*this)(2, 2) = (*this)(2, 2) * cosa - (*this)(2, 1) * sina;

	// zAxis.normalize();
	F32 len = sqrt((*this)(0, 2) * (*this)(0, 2)
		+ (*this)(1, 2) * (*this)(1, 2) + (*this)(2, 2) * (*this)(2, 2));
	(*this)(0, 2) /= len;
	(*this)(1, 2) /= len;
	(*this)(2, 2) /= len;

	// yAxis = zAxis * xAxis;
	(*this)(0, 1) = (*this)(1, 2) * (*this)(2, 0)
		- (*this)(2, 2) * (*this)(1, 0);
	(*this)(1, 1) = (*this)(2, 2) * (*this)(0, 0)
		- (*this)(0, 2) * (*this)(2, 0);
	(*this)(2, 1) = (*this)(0, 2) * (*this)(1, 0)
		- (*this)(1, 2) * (*this)(0, 0);

	// yAxis.normalize();
}

// rotateYAxis
inline void Mat3::rotateYAxis(const F32 rad)
{
	// NOTE: See the clean code from < r664
	F32 sina, cosa;
	sinCos(rad, sina, cosa);

	// zAxis = zAxis*cosa + xAxis*sina;
	(*this)(0, 2) = (*this)(0, 2) * cosa + (*this)(0, 0) * sina;
	(*this)(1, 2) = (*this)(1, 2) * cosa + (*this)(1, 0) * sina;
	(*this)(2, 2) = (*this)(2, 2) * cosa + (*this)(2, 0) * sina;

	// zAxis.normalize();
	F32 len = sqrt((*this)(0, 2) * (*this)(0, 2)
		+ (*this)(1, 2) * (*this)(1, 2) + (*this)(2, 2) * (*this)(2, 2));
	(*this)(0, 2) /= len;
	(*this)(1, 2) /= len;
	(*this)(2, 2) /= len;

	// xAxis = (zAxis*yAxis) * -1.0f;
	(*this)(0, 0) = (*this)(2, 2) * (*this)(1, 1)
		- (*this)(1, 2) * (*this)(2, 1);
	(*this)(1, 0) = (*this)(0, 2) * (*this)(2, 1)
		- (*this)(2, 2) * (*this)(0, 1);
	(*this)(2, 0) = (*this)(1, 2) * (*this)(0, 1)
		- (*this)(0, 2) * (*this)(1, 1);
}

// rotateZAxis
inline void Mat3::rotateZAxis(const F32 rad)
{
	// NOTE: See the clean code from < r664
	F32 sina, cosa;
	sinCos(rad, sina, cosa);

	// xAxis = xAxis*cosa + yAxis*sina;
	(*this)(0, 0) = (*this)(0, 0) * cosa + (*this)(0, 1) * sina;
	(*this)(1, 0) = (*this)(1, 0) * cosa + (*this)(1, 1) * sina;
	(*this)(2, 0) = (*this)(2, 0) * cosa + (*this)(2, 1) * sina;

	// xAxis.normalize();
	F32 len = sqrt((*this)(0, 0) * (*this)(0, 0)
		+ (*this)(1, 0) * (*this)(1, 0) + (*this)(2, 0) * (*this)(2, 0));
	(*this)(0, 0) /= len;
	(*this)(1, 0) /= len;
	(*this)(2, 0) /= len;

	// yAxis = zAxis*xAxis;
	(*this)(0, 1) = (*this)(1, 2) * (*this)(2, 0)
		- (*this)(2, 2) * (*this)(1, 0);
	(*this)(1, 1) = (*this)(2, 2) * (*this)(0, 0)
		- (*this)(0, 2) * (*this)(2, 0);
	(*this)(2, 1) = (*this)(0, 2) * (*this)(1, 0)
		- (*this)(1, 2) * (*this)(0, 0);
}

// transpose
inline void Mat3::transpose()
{
	F32 temp = (*this)(0, 1);
	(*this)(0, 1) = (*this)(1, 0);
	(*this)(1, 0) = temp;
	temp = (*this)(0, 2);
	(*this)(0, 2) = (*this)(2, 0);
	(*this)(2, 0) = temp;
	temp = (*this)(1, 2);
	(*this)(1, 2) = (*this)(2, 1);
	(*this)(2, 1) = temp;
}

// transposed
inline Mat3 Mat3::getTransposed() const
{
	Mat3 m3;
	m3[0] = (*this)[0];
	m3[1] = (*this)[3];
	m3[2] = (*this)[6];
	m3[3] = (*this)[1];
	m3[4] = (*this)[4];
	m3[5] = (*this)[7];
	m3[6] = (*this)[2];
	m3[7] = (*this)[5];
	m3[8] = (*this)[8];
	return m3;
}

// reorthogonalize
inline void Mat3::reorthogonalize()
{
	// There are 2 methods, the standard and the Gram-Schmidt method with a 
	// twist for zAxis. This uses the 2nd. For the first see < r664
	Vec3 xAxis, yAxis, zAxis;
	getColumns(xAxis, yAxis, zAxis);

	xAxis.normalize();

	yAxis = yAxis - (xAxis * xAxis.dot(yAxis));
	yAxis.normalize();

	zAxis = xAxis.cross(yAxis);

	setColumns(xAxis, yAxis, zAxis);
}

// Determinant
inline F32 Mat3::getDet() const
{
	// For the accurate method see < r664
	return (*this)(0, 0) * ((*this)(1, 1) * (*this)(2, 2)
		- (*this)(1, 2) * (*this)(2, 1)) - (*this)(0, 1) * ((*this)(1, 0)
		* (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 0)) + (*this)(0, 2)
		* ((*this)(0, 1) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0));
}

// getInverse
inline Mat3 Mat3::getInverse() const
{
	// Using Gramer's method Inv(A) = (1 / getDet(A)) * Adj(A)
	Mat3 r;

	// compute determinant
	F32 cofactor0 = (*this)(1, 1) * (*this)(2, 2)
		- (*this)(1, 2) * (*this)(2, 1);
	F32 cofactor3 = (*this)(0, 2) * (*this)(2, 1)
		- (*this)(0, 1) * (*this)(2, 2);
	F32 cofactor6 = (*this)(0, 1) * (*this)(1, 2)
		- (*this)(0, 2) * (*this)(1, 1);
	F32 det = (*this)(0, 0) * cofactor0 + (*this)(1, 0) * cofactor3
		+ (*this)(2, 0) * cofactor6;

	ANKI_ASSERT(!isZero(det)); // Cannot invert det == 0

	// create adjoint matrix and multiply by 1/det to get inverse
	F32 invDet = 1.0 / det;
	r(0, 0) = invDet * cofactor0;
	r(0, 1) = invDet * cofactor3;
	r(0, 2) = invDet * cofactor6;

	r(1, 0) = invDet * ((*this)(1, 2) * (*this)(2, 0)
		- (*this)(1, 0) * (*this)(2, 2));
	r(1, 1) = invDet * ((*this)(0, 0) * (*this)(2, 2)
		- (*this)(0, 2) * (*this)(2, 0));
	r(1, 2) = invDet * ((*this)(0, 2) * (*this)(1, 0)
		- (*this)(0, 0) * (*this)(1, 2));

	r(2, 0) = invDet * ((*this)(1, 0) * (*this)(2, 1)
		- (*this)(1, 1) * (*this)(2, 0));
	r(2, 1) = invDet * ((*this)(0, 1) * (*this)(2, 0)
		- (*this)(0, 0) * (*this)(2, 1));
	r(2, 2) = invDet * ((*this)(0, 0) * (*this)(1, 1)
		- (*this)(0, 1) * (*this)(1, 0));

	return r;
}

// setIdentity
inline void Mat3::setIdentity()
{
	(*this) = getIdentity();
}

// invert
// see above
inline void Mat3::invert()
{
	(*this) = getInverse();
}

// getZero
inline const Mat3& Mat3::getZero()
{
	static const Mat3 zero(0.0);
	return zero;
}

// getIdentity
inline const Mat3& Mat3::getIdentity()
{
	static const Mat3 ident(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	return ident;
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

// F32 + 3x3
inline Mat3 operator+(const F32 f, const Mat3& m3)
{
	return m3 + f;
}

// F32 - 3x3
inline Mat3 operator-(const F32 f, const Mat3& m3)
{
	Mat3 out;
	for(U i = 0; i < 9; i++)
	{
		out[i] = f - m3[i];
	}
	return out;
}

// F32 * 3x3
inline Mat3 operator*(const F32 f, const Mat3& m3)
{
	Mat3 out;
	for(U i = 0; i < 9; i++)
	{
		out[i] = f * m3[i];
	}
	return out;
}

// F32 / 3x3
inline Mat3 operator/(const F32 f, const Mat3& m3)
{
	Mat3 out;
	for(U i = 0; i < 9; i++)
	{
		out[i] = f / m3[i];
	}
	return out;
}

// Print
inline std::ostream& operator<<(std::ostream& s, const Mat3& m)
{
	for(U i = 0; i < 3; i++)
	{
		for(U j = 0; j < 3; j++)
		{
			s << m(i, j) << ' ';
		}

		if(i != 2)
		{
			s << "\n";
		}
	}
	return s;
}

} // end namespace
