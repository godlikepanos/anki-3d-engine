#include <iostream>
#include "MathDfltHeader.h"


#define ME (*this)

namespace M {


// constructor []
inline Quat::Quat()
	: x(0.0), y(0.0), z(0.0), w(1.0)
{}

// constructor [float]
inline Quat::Quat(float f)
	: x(f), y(f), z(f), w(f)
{}

// constructor [float, float, float, float]
inline Quat::Quat(float x_, float y_, float z_, float w_)
	: x(x_), y(y_), z(z_), w(w_)
{}

// constructor [vec2, float, float]
inline Quat::Quat(const Vec2& v2, float z_, float w_)
	: x(v2.x), y(v2.y), z(z_), w(w_)
{}

// constructor [vec3, float]
inline Quat::Quat(const Vec3& v3, float w_)
	: x(v3.x), y(v3.y), z(v3.z), w(w_)
{}

// constructor [vec4]
inline Quat::Quat(const Vec4& v4)
	: x(v4.x), y(v4.y), z(v4.z), w(v4.w)
{}

// constructor [quat]
inline Quat::Quat(const Quat& b)
	: x(b.x), y(b.y), z(b.z), w(b.w)
{}

// constructor [mat3]
inline Quat::Quat(const Mat3& m3)
{
	float trace = m3(0, 0) + m3(1, 1) + m3(2, 2) + 1.0;
	if(trace > EPSILON)
	{
		float s = 0.5 * invSqrt(trace);
		w = 0.25 / s;
		x = (m3(2, 1) - m3(1, 2)) * s;
		y = (m3(0, 2) - m3(2, 0)) * s;
		z = (m3(1, 0) - m3(0, 1)) * s;
	}
	else
	{
		if(m3(0, 0) > m3(1, 1) && m3(0, 0) > m3(2, 2))
		{
			float s = 0.5 * invSqrt(1.0 + m3(0, 0) - m3(1, 1) - m3(2, 2));
			w = (m3(1, 2) - m3(2, 1)) * s;
			x = 0.25 / s;
			y = (m3(0, 1) + m3(1, 0)) * s;
			z = (m3(0, 2) + m3(2, 0)) * s;
		}
		else if(m3(1, 1) > m3(2, 2))
		{
			float s = 0.5 * invSqrt(1.0 + m3(1, 1) - m3(0, 0) - m3(2, 2));
			w = (m3(0, 2) - m3(2, 0)) * s;
			x = (m3(0, 1) + m3(1, 0)) * s;
			y = 0.25 / s;
			z = (m3(1, 2) + m3(2, 1)) * s;
		}
		else
		{
			float s = 0.5 * invSqrt(1.0 + m3(2, 2) - m3(0, 0) - m3(1, 1));
			w = (m3(0, 1) - m3(1, 0)) * s;
			x = (m3(0, 2) + m3(2, 0)) * s;
			y = (m3(1, 2) + m3(2, 1)) * s;
			z = 0.25 / s;
		}
	}
}

// constructor [euler]
inline Quat::Quat(const Euler& eu)
{
	float cx, sx;
	sinCos(eu.heading()*0.5, sx, cx);

	float cy, sy;
	sinCos(eu.attitude()*0.5, sy, cy);

	float cz, sz;
	sinCos(eu.bank()*0.5, sz, cz);

	float cxcy = cx*cy;
	float sxsy = sx*sy;
	x = cxcy*sz + sxsy*cz;
	y = sx*cy*cz + cx*sy*sz;
	z = cx*sy*cz - sx*cy*sz;
	w = cxcy*cz - sxsy*sz;
}

// constructor [euler]
inline Quat::Quat(const Axisang& axisang)
{
	float lengthsq = axisang.axis.getLengthSquared();
	if(isZero(lengthsq))
	{
		ME = getIdentity();
		return;
	}

	float rad = axisang.ang * 0.5;

	float sintheta, costheta;
	sinCos(rad, sintheta, costheta);

	float scalefactor = sintheta * invSqrt(lengthsq);

	x = scalefactor * axisang.axis.x;
	y = scalefactor * axisang.axis.y;
	z = scalefactor * axisang.axis.z;
	w = costheta;
}

// *
inline Quat Quat::operator *(const Quat& b) const
{
	return Quat(
		 x * b.w + y * b.z - z * b.y + w * b.x,
		-x * b.z + y * b.w + z * b.x + w * b.y,
		 x * b.y - y * b.x + z * b.w + w * b.z,
		-x * b.x - y * b.y - z * b.z + w * b.w
	);
}

// *=
inline Quat& Quat::operator *=(const Quat& b)
{
	ME = ME * b;
	return ME;
}

// ==
inline bool Quat::operator ==(const Quat& b) const
{
	return (isZero(x-b.x) && isZero(y-b.y) && isZero(z-b.z) && isZero(w-b.w)) ? true : false;
}

// !=
inline bool Quat::operator !=(const Quat& b) const
{
	return (isZero(x-b.x) && isZero(y-b.y) && isZero(z-b.z) && isZero(w-b.w)) ? false : true;
}

// conjugate
inline void Quat::conjugate()
{
	x = -x;
	y = -y;
	z = -z;
}

// getConjugated
inline Quat Quat::getConjugated() const
{
	return Quat(-x, -y, -z, w);
}

// Normalized
inline Quat Quat::getNormalized() const
{
	return Quat(Vec4(ME).getNormalized());
}

// normalize
inline void Quat::normalize()
{
	ME = getNormalized();
}

// getLength
inline float Quat::getLength() const
{
	return M::sqrt(w*w + x*x + y*y + z*z);
}


// getInverted
inline Quat Quat::getInverted() const
{
	float norm = w*w + x*x + y*y + z*z;

	DEBUG_ERR(isZero(norm)); // Norm is zero

	float normi = 1.0 / norm;
	return Quat(-normi*x, -normi*y, -normi*z, normi*w);
}

// invert
inline void Quat::invert()
{
	ME = getInverted();
}

// CalcFromVecVec
inline void Quat::setFrom2Vec3(const Vec3& from, const Vec3& to)
{
	Vec3 axis(from.cross(to));
	ME = Quat(axis.x, axis.y, axis.z, from.dot(to));
	normalize();
	w += 1.0;

	if(w <= EPSILON)
	{
		if(from.z*from.z > from.x*from.x)
			ME = Quat(0.0, from.z, -from.y, 0.0);
		else
			ME = Quat(from.y, -from.x, 0.0, 0.0);
	}
	normalize();
}

// getRotated
inline Quat Quat::getRotated(const Quat& b) const
{
	return ME * b;
}

// rotate
inline void Quat::rotate(const Quat& b)
{
	ME = getRotated(b);
}

// dot
inline float Quat::dot(const Quat& b) const
{
	return w*b.w + x*b.x + y*b.y + z*b.z;
}

// SLERP
inline Quat Quat::slerp(const Quat& q1_, float t) const
{
	const Quat& q0 = ME;
	Quat q1(q1_);
	float cosHalfTheta = q0.w*q1.w + q0.x*q1.x + q0.y*q1.y + q0.z*q1.z;
	if(cosHalfTheta < 0.0)
	{
		q1 = Quat(-Vec4(q1)); // quat changes
		cosHalfTheta = -cosHalfTheta;
	}
	if(fabs(cosHalfTheta) >= 1.0f)
	{
		return Quat(q0);
	}

	float halfTheta = acos(cosHalfTheta);
	float sinHalfTheta = M::sqrt(1.0 - cosHalfTheta*cosHalfTheta);

	if(fabs(sinHalfTheta) < 0.001)
	{
		return Quat((Vec4(q0) + Vec4(q1))*0.5);
	}
	float ratioA = sin((1.0 - t) * halfTheta) / sinHalfTheta;
	float ratio_b = sin(t * halfTheta) / sinHalfTheta;
	Vec4 tmp, tmp1, sum;
	tmp = Vec4(q0)*ratioA;
	tmp1 = Vec4(q1)*ratio_b;
	sum = tmp + tmp1;
	sum.normalize();
	return Quat(sum);
}

// setIdentity
inline void Quat::setIdentity()
{
	x = y = z = 0.0;
	w = 1.0;
}

// getIdentity
inline const Quat::Quat& getIdentity()
{
	static Quat ident(0.0, 0.0, 0.0, 1.0);
	return ident;
}

// print
inline ostream& operator<<(ostream& s, const Quat& q)
{
	s << q.w << ' ' << q.x << ' ' << q.y << ' ' << q.z;
	return s;
}

} // end namespace
