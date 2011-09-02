#include "MathCommonSrc.h"


//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Quat::Quat()
{
	x() = y() = z() = w() = 0.0;
}


// float
inline Quat::Quat(const float f)
{
	x() = y() = z() = w() = f;
}


// float, float, float, float
inline Quat::Quat(const float x_, const float y_, const float z_,
	const float w_)
{
	x() = x_;
	y() = y_;
	z() = z_;
	w() = w_;
}


// constructor [vec2, float, float]
inline Quat::Quat(const Vec2& v, const float z_, const float w_)
{
	x() = v.x();
	y() = v.y();
	z() = z_;
	w() = w_;
}


// constructor [vec3, float]
inline Quat::Quat(const Vec3& v, const float w_)
{
	x() = v.x();
	y() = v.y();
	z() = v.z();
	w() = w_;
}


// constructor [vec4]
inline Quat::Quat(const Vec4& v)
{
	x() = v.x();
	y() = v.y();
	z() = v.z();
	w() = v.w();
}


// Copy
inline Quat::Quat(const Quat& b)
{
	x() = b.x();
	y() = b.y();
	z() = b.z();
	w() = b.w();
}


// mat3
inline Quat::Quat(const Mat3& m3)
{
	float trace = m3(0, 0) + m3(1, 1) + m3(2, 2) + 1.0;
	if(trace > Math::EPSILON)
	{
		float s = 0.5 / sqrt(trace);
		w() = 0.25 / s;
		x() = (m3(2, 1) - m3(1, 2)) * s;
		y() = (m3(0, 2) - m3(2, 0)) * s;
		z() = (m3(1, 0) - m3(0, 1)) * s;
	}
	else
	{
		if(m3(0, 0) > m3(1, 1) && m3(0, 0) > m3(2, 2))
		{
			float s = 0.5 / sqrt(1.0 + m3(0, 0) - m3(1, 1) - m3(2, 2));
			w() = (m3(1, 2) - m3(2, 1)) * s;
			x() = 0.25 / s;
			y() = (m3(0, 1) + m3(1, 0)) * s;
			z() = (m3(0, 2) + m3(2, 0)) * s;
		}
		else if(m3(1, 1) > m3(2, 2))
		{
			float s = 0.5 / sqrt(1.0 + m3(1, 1) - m3(0, 0) - m3(2, 2));
			w() = (m3(0, 2) - m3(2, 0)) * s;
			x() = (m3(0, 1) + m3(1, 0)) * s;
			y() = 0.25 / s;
			z() = (m3(1, 2) + m3(2, 1)) * s;
		}
		else
		{
			float s = 0.5 / sqrt(1.0 + m3(2, 2) - m3(0, 0) - m3(1, 1));
			w() = (m3(0, 1) - m3(1, 0)) * s;
			x() = (m3(0, 2) + m3(2, 0)) * s;
			y() = (m3(1, 2) + m3(2, 1)) * s;
			z() = 0.25 / s;
		}
	}
}


// euler
inline Quat::Quat(const Euler& eu)
{
	float cx, sx;
	Math::sinCos(eu.y() * 0.5, sx, cx);

	float cy, sy;
	Math::sinCos(eu.z() * 0.5, sy, cy);

	float cz, sz;
	Math::sinCos(eu.x() * 0.5, sz, cz);

	float cxcy = cx * cy;
	float sxsy = sx * sy;
	x() = cxcy * sz + sxsy * cz;
	y() = sx * cy * cz + cx * sy * sz;
	z() = cx * sy * cz - sx * cy * sz;
	w() = cxcy * cz - sxsy * sz;
}


// euler
inline Quat::Quat(const Axisang& axisang)
{
	float lengthsq = axisang.getAxis().getLengthSquared();
	if(Math::isZero(lengthsq))
	{
		(*this) = getIdentity();
		return;
	}

	float rad = axisang.getAngle() * 0.5;

	float sintheta, costheta;
	Math::sinCos(rad, sintheta, costheta);

	float scalefactor = sintheta / sqrt(lengthsq);

	x() = scalefactor * axisang.getAxis().x();
	y() = scalefactor * axisang.getAxis().y();
	z() = scalefactor * axisang.getAxis().z();
	w() = costheta;
}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline float Quat::x() const
{
	return vec.x;
}


inline float& Quat::x()
{
	return vec.x;
}


inline float Quat::y() const
{
	return vec.y;
}


inline float& Quat::y()
{
	return vec.y;
}


inline float Quat::z() const
{
	return vec.z;
}


inline float& Quat::z()
{
	return vec.z;
}


inline float Quat::w() const
{
	return vec.w;
}


inline float& Quat::w()
{
	return vec.w;
}


//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Quat& Quat::operator=(const Quat& b)
{
	x() = b.x();
	y() = b.y();
	z() = b.z();
	w() = b.w();
	return *this;
}


// *
inline Quat Quat::operator *(const Quat& b) const
{
	return Quat(x() * b.w() + y() * b.z() - z() * b.y() + w() * b.x(),
		-x() * b.z() + y() * b.w() + z() * b.x() + w() * b.y(),
		x() * b.y() - y() * b.x() + z() * b.w() + w() * b.z(),
		-x() * b.x() - y() * b.y() - z() * b.z() + w() * b.w()
	);
}


// *=
inline Quat& Quat::operator *=(const Quat& b)
{
	(*this) = (*this) * b;
	return (*this);
}


// ==
inline bool Quat::operator ==(const Quat& b) const
{
	return Math::isZero(x() - b.x()) &&
		Math::isZero(y() - b.y()) &&
		Math::isZero(z() - b.z()) &&
		Math::isZero(w() - b.w());
}


// !=
inline bool Quat::operator !=(const Quat& b) const
{
	return !(Math::isZero(x() - b.x()) &&
		Math::isZero(y() - b.y()) &&
		Math::isZero(z() - b.z()) &&
		Math::isZero(w() - b.w()));
}


//==============================================================================
// Other                                                                       =
//==============================================================================

// conjugate
inline void Quat::conjugate()
{
	x() = -x();
	y() = -y();
	z() = -z();
}


// getConjugated
inline Quat Quat::getConjugated() const
{
	return Quat(-x(), -y(), -z(), w());
}


// Normalized
inline Quat Quat::getNormalized() const
{
	return Quat(Vec4((*this)).getNormalized());
}


// normalize
inline void Quat::normalize()
{
	(*this) = getNormalized();
}


// getLength
inline float Quat::getLength() const
{
	return Math::sqrt(w() * w() + x() * x() + y() * y() + z() * z());
}


// getInverted
inline Quat Quat::getInverted() const
{
	float norm = w() * w() + x() * x() + y() * y() + z() * z();

	ASSERT(!Math::isZero(norm)); // Norm is zero

	float normi = 1.0 / norm;
	return Quat(-normi * x(), -normi * y(), -normi * z(), normi * w());
}


// invert
inline void Quat::invert()
{
	(*this) = getInverted();
}


// CalcFromVecVec
inline void Quat::setFrom2Vec3(const Vec3& from, const Vec3& to)
{
	Vec3 axis(from.cross(to));
	(*this) = Quat(axis.x(), axis.y(), axis.z(), from.dot(to));
	normalize();
	w() += 1.0;

	if(w() <= Math::EPSILON)
	{
		if(from.z() * from.z() > from.x() * from.x())
		{
			(*this) = Quat(0.0, from.z(), -from.y(), 0.0);
		}
		else
		{
			(*this) = Quat(from.y(), -from.x(), 0.0, 0.0);
		}
	}
	normalize();
}


// getRotated
inline Quat Quat::getRotated(const Quat& b) const
{
	return (*this) * b;
}


// rotate
inline void Quat::rotate(const Quat& b)
{
	(*this) = getRotated(b);
}


// dot
inline float Quat::dot(const Quat& b) const
{
	return w() * b.w() + x() * b.x() + y() * b.y() + z() * b.z();
}


// SLERP
inline Quat Quat::slerp(const Quat& q1_, const float t) const
{
	const Quat& q0 = (*this);
	Quat q1(q1_);
	float cosHalfTheta = q0.w() * q1.w() + q0.x() * q1.x() + q0.y() * q1.y() +
		q0.z() * q1.z();
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
	float sinHalfTheta = Math::sqrt(1.0 - cosHalfTheta*cosHalfTheta);

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
	x() = y() = z() = 0.0;
	w() = 1.0;
}


// getIdentity
inline const Quat& Quat::getIdentity()
{
	static Quat ident(0.0, 0.0, 0.0, 1.0);
	return ident;
}


//==============================================================================
// Friends                                                                     =
//==============================================================================

// print
inline std::ostream& operator<<(std::ostream& s, const Quat& q)
{
	s << q.w() << ' ' << q.x() << ' ' << q.y() << ' ' << q.z();
	return s;
}
