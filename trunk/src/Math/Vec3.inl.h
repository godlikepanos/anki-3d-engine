#include "MathCommon.inl.h"


#define ME (*this)


namespace M {


// accessors
inline float& Vec3::operator [](uint i)
{
	return (&x)[i];
}

inline float Vec3::operator [](uint i) const
{
	return (&x)[i];
}

// constructor []
inline Vec3::Vec3()
	: x(0.0), y(0.0), z(0.0)
{}

// constructor [float, float, float]
inline Vec3::Vec3(float x_, float y_, float z_)
	: x(x_), y(y_), z(z_)
{}

// constructor [float]
inline Vec3::Vec3(float f)
	: x(f), y(f), z(f)
{}

// constructor [float[]]
inline Vec3::Vec3(float arr[])
{
	x = arr[0];
	y = arr[1];
	z = arr[2];
}

// constructor [Vec3]
inline Vec3::Vec3(const Vec3& b)
	: x(b.x), y(b.y), z(b.z)
{}

// constructor [vec2, float]
inline Vec3::Vec3(const Vec2& v2, float z_)
	: x(v2.x()), y(v2.y()), z(z_)
{}

// constructor [vec4]
inline Vec3::Vec3(const Vec4& v4)
	: x(v4.x()), y(v4.y()), z(v4.z())
{}

// constructor [quat]
inline Vec3::Vec3(const Quat& q)
	: x(q.x), y(q.y), z(q.z)
{}

// +
inline Vec3 Vec3::operator +(const Vec3& b) const
{
	return Vec3(x+b.x, y+b.y, z+b.z);
}

// +=
inline Vec3& Vec3::operator +=(const Vec3& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
	return ME;
}

// -
inline Vec3 Vec3::operator -(const Vec3& b) const
{
	return Vec3(x-b.x, y-b.y, z-b.z);
}

// -=
inline Vec3& Vec3::operator -=(const Vec3& b)
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	return ME;
}

// *
inline Vec3 Vec3::operator *(const Vec3& b) const
{
	return Vec3(x*b.x, y*b.y, z*b.z);
}

// *=
inline Vec3& Vec3::operator *=(const Vec3& b)
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	return ME;
}

// /
inline Vec3 Vec3::operator /(const Vec3& b) const
{
	return Vec3(x/b.x, y/b.y, z/b.z);
}

// /=
inline Vec3& Vec3::operator /=(const Vec3& b)
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	return ME;
}

// negative
inline Vec3 Vec3::operator -() const
{
	return Vec3(-x, -y, -z);
}

// ==
inline bool Vec3::operator ==(const Vec3& b) const
{
	return (isZero(x-b.x) && isZero(y-b.y) && isZero(z-b.z)) ? true : false;
}

// !=
inline bool Vec3::operator !=(const Vec3& b) const
{
	return (isZero(x-b.x) && isZero(y-b.y) && isZero(z-b.z)) ? false : true;
}

// Vec3 + float
inline Vec3 Vec3::operator +(float f) const
{
	return ME + Vec3(f);
}

// float + Vec3
inline Vec3 operator +(float f, const Vec3& v)
{
	return v+f;
}

// Vec3 += float
inline Vec3& Vec3::operator +=(float f)
{
	ME += Vec3(f);
	return ME;
}

// Vec3 - float
inline Vec3 Vec3::operator -(float f) const
{
	return ME - Vec3(f);
}

// float - Vec3
inline Vec3 operator -(float f, const Vec3& v)
{
	return Vec3(f-v.x, f-v.y, f-v.z);
}

// Vec3 -= float
inline Vec3& Vec3::operator -=(float f)
{
	ME -= Vec3(f);
	return ME;
}

// Vec3 * float
inline Vec3 Vec3::operator *(float f) const
{
	return ME * Vec3(f);
}

// float * Vec3
inline Vec3 operator *(float f, const Vec3& v)
{
	return v*f;
}

// Vec3 *= float
inline Vec3& Vec3::operator *=(float f)
{
	ME *= Vec3(f);
	return ME;
}

// Vec3 / float
inline Vec3 Vec3::operator /(float f) const
{
	return ME / Vec3(f);
}

// float / Vec3
inline Vec3 operator /(float f, const Vec3& v)
{
	return Vec3(f/v.x, f/v.y, f/v.z);
}

// Vec3 /= float
inline Vec3& Vec3::operator /=(float f)
{
	ME /= Vec3(f);
	return ME;
}

// dot
inline float Vec3::dot(const Vec3& b) const
{
	return x*b.x + y*b.y + z*b.z;
}

// cross prod
inline Vec3 Vec3::cross(const Vec3& b) const
{
	return Vec3(y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x);
}

// getLength
inline float Vec3::getLength() const
{
	return M::sqrt(x*x + y*y + z*z);
}

// getLengthSquared
inline float Vec3::getLengthSquared() const
{
	return x*x + y*y + z*z;
}

// getDistanceSquared
inline float Vec3::getDistanceSquared(const Vec3& b) const
{
	return (ME-b).getLengthSquared();
}

// normalize
inline void Vec3::normalize()
{
	ME /= getLength();
}

// Normalized (return the normalized)
inline Vec3 Vec3::getNormalized() const
{
	return ME / getLength();
}

// getProjection
inline Vec3 Vec3::getProjection(const Vec3& toThis) const
{
	return toThis * (ME.dot(toThis)/(toThis.dot(toThis)));
}

// Rotated
inline Vec3 Vec3::getRotated(const Quat& q) const
{
	RASSERT_THROW_EXCEPTION(!isZero(1.0-q.getLength())); // Not normalized quat

	/*float vmult = 2.0f*(q.x*x + q.y*y + q.z*z);
	float crossmult = 2.0*q.w;
	float pmult = crossmult*q.w - 1.0;

	return Vec3(pmult*x + vmult*q.x + crossmult*(q.y*z - q.z*y),
							   pmult*y + vmult*q.y + crossmult*(q.z*x - q.x*z),
	               pmult*z + vmult*q.z + crossmult*(q.x*y - q.y*x));*/
	Vec3 qXyz(q);
	return ME + qXyz.cross(qXyz.cross(ME) + ME*q.w) * 2.0;
}

// rotate
inline void Vec3::rotate(const Quat& q)
{
	ME = getRotated(q);
}

// lerp
inline Vec3 Vec3::lerp(const Vec3& v1, float t) const
{
	return (ME*(1.0f-t))+(v1*t);
}

// getTransformed [mat3]
inline Vec3 Vec3::getTransformed(const Vec3& translate, const Mat3& rotate, float scale) const
{
	return (rotate * (ME * scale)) + translate;
}

// transform [mat3]
inline void Vec3::transform(const Vec3& translate, const Mat3& rotate, float scale)
{
	ME = getTransformed(translate, rotate, scale);
}

// getTransformed [mat3] no scale
inline Vec3 Vec3::getTransformed(const Vec3& translate, const Mat3& rotate) const
{
	return (rotate * ME) + translate;
}

// transform [mat3] no scale
inline void Vec3::transform(const Vec3& translate, const Mat3& rotate)
{
	ME = getTransformed(translate, rotate);
}

// getTransformed [quat]
inline Vec3 Vec3::getTransformed(const Vec3& translate, const Quat& rotate, float scale) const
{
	return (ME * scale).getRotated(rotate) + translate;
}

// transform [quat3] no scale
inline void Vec3::transform(const Vec3& translate, const Quat& rotate, float scale)
{
	ME = getTransformed(translate, rotate, scale);
}

// getTransformed [mat4]
inline Vec3 Vec3::getTransformed(const Mat4& transform) const
{
	return Vec3(
		transform(0, 0)*x + transform(0, 1)*y + transform(0, 2)*z + transform(0, 3),
		transform(1, 0)*x + transform(1, 1)*y + transform(1, 2)*z + transform(1, 3),
		transform(2, 0)*x + transform(2, 1)*y + transform(2, 2)*z + transform(2, 3)
	);
}

// getTransformed [mat4]
inline void Vec3::transform(const Mat4& transform)
{
	ME = getTransformed(transform);
}

// getTransformed [Transform]
inline Vec3 Vec3::getTransformed(const Transform& transform) const
{
	return (transform.rotation * (ME * transform.scale)) + transform.origin;
}

// getTransformed [Transform]
inline void Vec3::transform(const Transform& transform)
{
	ME = getTransformed(transform);
}

// print
inline std::ostream& operator<<(std::ostream& s, const Vec3& v)
{
	s << v.x << ' ' << v.y << ' ' << v.z;
	return s;
}

} // end namespace
