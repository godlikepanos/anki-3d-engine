#ifndef ANKI_MATH_QUAT_H
#define ANKI_MATH_QUAT_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Quaternion. Used in rotations
template<typename T>
ANKI_ATTRIBUTE_ALIGNED(class, 16) TQuat
{
public:
	/// @name Constructors
	/// @{
	explicit TQuat()
	{
		x() = y() = z() = w() = 0.0;
	}

	explicit TQuat(const T f)
	{
		x() = y() = z() = w() = f;
	}

	explicit TQuat(const T x_, const T y_, const T z_, const T w_)
	{
		x() = x_;
		y() = y_;
		z() = z_;
		w() = w_;
	}

	explicit TQuat(const TVec2<T>& v2, const T z_, const T w_)
	{
		x() = v2.x();
		y() = v2.y();
		z() = z_;
		w() = w_;
	}

	explicit TQuat(const TVec3<T>& v3, const T w_)
	{
		x() = v3.x();
		y() = v3.y();
		z() = v3.z();
		w() = w_;
	}

	explicit TQuat(const TVec4<T>& v4)
	{
		x() = v4.x();
		y() = v4.y();
		z() = v4.z();
		w() = v4.w();
	}
	
	TQuat(const TQuat& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
		w() = b.w();
	}

	explicit TQuat(const TMat3<T>& m3)
	{
		T trace = m3(0, 0) + m3(1, 1) + m3(2, 2) + 1.0;
		if(trace > getEpsilon<T>())
		{
			T s = 0.5 / sqrt(trace);
			w() = 0.25 / s;
			x() = (m3(2, 1) - m3(1, 2)) * s;
			y() = (m3(0, 2) - m3(2, 0)) * s;
			z() = (m3(1, 0) - m3(0, 1)) * s;
		}
		else
		{
			if(m3(0, 0) > m3(1, 1) && m3(0, 0) > m3(2, 2))
			{
				T s = 0.5 / sqrt(1.0 + m3(0, 0) - m3(1, 1) - m3(2, 2));
				w() = (m3(1, 2) - m3(2, 1)) * s;
				x() = 0.25 / s;
				y() = (m3(0, 1) + m3(1, 0)) * s;
				z() = (m3(0, 2) + m3(2, 0)) * s;
			}
			else if(m3(1, 1) > m3(2, 2))
			{
				T s = 0.5 / sqrt(1.0 + m3(1, 1) - m3(0, 0) - m3(2, 2));
				w() = (m3(0, 2) - m3(2, 0)) * s;
				x() = (m3(0, 1) + m3(1, 0)) * s;
				y() = 0.25 / s;
				z() = (m3(1, 2) + m3(2, 1)) * s;
			}
			else
			{
				T s = 0.5 / sqrt(1.0 + m3(2, 2) - m3(0, 0) - m3(1, 1));
				w() = (m3(0, 1) - m3(1, 0)) * s;
				x() = (m3(0, 2) + m3(2, 0)) * s;
				y() = (m3(1, 2) + m3(2, 1)) * s;
				z() = 0.25 / s;
			}
		}
	}

	explicit TQuat(const TEuler<T>& eu)
	{
		T cx, sx;
		sinCos(eu.y() * 0.5, sx, cx);

		T cy, sy;
		sinCos(eu.z() * 0.5, sy, cy);

		T cz, sz;
		sinCos(eu.x() * 0.5, sz, cz);

		T cxcy = cx * cy;
		T sxsy = sx * sy;
		x() = cxcy * sz + sxsy * cz;
		y() = sx * cy * cz + cx * sy * sz;
		z() = cx * sy * cz - sx * cy * sz;
		w() = cxcy * cz - sxsy * sz;
	}

	explicit TQuat(const TAxisang<T>& axisang)
	{
		T lengthsq = axisang.getAxis().getLengthSquared();
		if(isZero<T>(lengthsq))
		{
			(*this) = getIdentity();
			return;
		}

		T rad = axisang.getAngle() * 0.5;

		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		T scalefactor = sintheta / sqrt(lengthsq);

		x() = scalefactor * axisang.getAxis().x();
		y() = scalefactor * axisang.getAxis().y();
		z() = scalefactor * axisang.getAxis().z();
		w() = costheta;
	}
	/// @}

	/// @name Accessors
	/// @{
	T x() const
	{
		return vec.x;
	}

	T& x()
	{
		return vec.x;
	}

	T y() const
	{
		return vec.y;
	}

	T& y()
	{
		return vec.y;
	}

	T z() const
	{
		return vec.z;
	}

	T& z()
	{
		return vec.z;
	}

	T w() const
	{
		return vec.w;
	}

	T& w()
	{
		return vec.w;
	}
	/// @}

	/// Operators with same type
	/// @{
	TQuat& operator=(const TQuat& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
		w() = b.w();
		return *this;
	}

	/// 16 muls, 12 adds
	TQuat operator*(const TQuat& b) const
	{
		// XXX See if this can be optimized
		TQuat out;
		out.vec.x = x() * b.w() + y() * b.z() - z() * b.y() + w() * b.x();
		out.vec.y = -x() * b.z() + y() * b.w() + z() * b.x() + w() * b.y();
		out.vec.z = x() * b.y() - y() * b.x() + z() * b.w() + w() * b.z();
		out.vec.w = -x() * b.x() - y() * b.y() - z() * b.z() + w() * b.w();
		return out;
	}

	TQuat& operator*=(const TQuat& b)
	{
		(*this) = (*this) * b;
		return (*this);
	}

	Bool operator==(const TQuat& b) const
	{
		return isZero<T>(x() - b.x()) &&
			isZero<T>(y() - b.y()) &&
			isZero<T>(z() - b.z()) &&
			isZero<T>(w() - b.w());
	}

	Bool operator!=(const TQuat& b) const
	{
		return !((*this) == b);
	}
	/// @}

	/// @name Other
	/// @{

	/// Calculates the rotation from TVec3 "from" to "to"
	void setFrom2Vec3(const TVec3<T>& from, const TVec3<T>& to)
	{
		TVec3<T> axis(from.cross(to));
		(*this) = TQuat(axis.x(), axis.y(), axis.z(), from.dot(to));
		normalize();
		w() += 1.0;

		if(w() <= getEpsilon<T>())
		{
			if(from.z() * from.z() > from.x() * from.x())
			{
				(*this) = TQuat(0.0, from.z(), -from.y(), 0.0);
			}
			else
			{
				(*this) = TQuat(from.y(), -from.x(), 0.0, 0.0);
			}
		}
		normalize();
	}

	T getLength() const
	{
		return sqrt(w() * w() + x() * x() + y() * y() + z() * z());
	}

	TQuat getInverted() const
	{
		T norm = w() * w() + x() * x() + y() * y() + z() * z();

		ANKI_ASSERT(!isZero<T>(norm)); // Norm is zero

		T normi = 1.0 / norm;
		return TQuat(-normi * x(), -normi * y(), -normi * z(), normi * w());
	}

	void invert()
	{
		(*this) = getInverted();
	}
	
	void conjugate()
	{
		x() = -x();
		y() = -y();
		z() = -z();
	}

	TQuat getConjugated() const
	{
		return TQuat(-x(), -y(), -z(), w());
	}

	void normalize()
	{
		(*this) = getNormalized();
	}

	TQuat getNormalized() const
	{
		return TQuat(TVec4<T>(*this).getNormalized());
	}

	T dot(const TQuat& b) const
	{
		return w() * b.w() + x() * b.x() + y() * b.y() + z() * b.z();
	}

	/// Returns slerp(this, q1, t)
	TQuat slerp(const TQuat& q1_, const T t) const
	{
		const TQuat& q0 = *this;
		TQuat q1(q1_);
		T cosHalfTheta = q0.w() * q1.w() + q0.x() * q1.x() + q0.y() * q1.y() 
			+ q0.z() * q1.z();
		if(cosHalfTheta < 0.0)
		{
			q1 = TQuat(-TVec4<T>(q1)); // quat changes
			cosHalfTheta = -cosHalfTheta;
		}

		if(fabs(cosHalfTheta) >= 1.0f)
		{
			return TQuat(q0);
		}

		T halfTheta = acos<T>(cosHalfTheta);
		T sinHalfTheta = sqrt<T>(1.0 - cosHalfTheta * cosHalfTheta);

		if(fabs(sinHalfTheta) < 0.001)
		{
			return TQuat((TVec4<T>(q0) + TVec4<T>(q1)) * 0.5);
		}
		T ratioA = sin<T>((1.0 - t) * halfTheta) / sinHalfTheta;
		T ratio_b = sin<T>(t * halfTheta) / sinHalfTheta;
		TVec4<T> tmp, tmp1, sum;
		tmp = TVec4<T>(q0) * ratioA;
		tmp1 = TVec4<T>(q1) * ratio_b;
		sum = tmp + tmp1;
		sum.normalize();
		return TQuat(sum);
	}

	/// The same as TQuat * TQuat
	TQuat getRotated(const TQuat& b) const 
	{
		return (*this) * b;
	}

	/// @see getRotated
	void rotate(const TQuat& b)
	{
		(*this) = getRotated(b);
	}

	void setIdentity()
	{
		x() = y() = z() = 0.0;
		w() = 1.0;
	}

	static const TQuat& getIdentity()
	{
		static TQuat ident(0.0, 0.0, 0.0, 1.0);
		return ident;
	}

	std::string toString() const
	{
		return std::to_string(x()) + " " + std::to_string(y()) + " "
			+ std::to_string(z()) + " " + std::to_string(w());
	}
	/// @}

private:
	/// @name Data
	/// @{
	struct
	{
		T x, y, z, w;
	} vec;
	/// @}
};

/// F32 quaternion
typedef TQuat<F32> Quat;

/// @}

} // end namespace anki

#endif
