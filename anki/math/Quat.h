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

/// Quaternion. Used in rotations
template<typename T>
class alignas(16) TQuat : public TVec<T, 4>
{
public:
	using Base = TVec<T, 4>;

	using Base::x;
	using Base::y;
	using Base::z;
	using Base::w;
	using Base::normalize; // Shortcut
	using Base::getLengthSquared; // Shortcut
	using Base::operator-;
	using Base::operator=;

	/// @name Constructors
	/// @{
	TQuat()
		: Base()
	{
	}

	TQuat(const TQuat& b)
		: Base(b)
	{
	}

	TQuat(const T x_, const T y_, const T z_, const T w_)
		: Base(x_, y_, z_, w_)
	{
	}

	explicit TQuat(const T f)
		: Base(f)
	{
	}

	explicit TQuat(const T arr[])
		: Base(arr)
	{
	}

	explicit TQuat(const typename Base::Simd& simd)
		: Base(simd)
	{
	}

	TQuat(const TVec<T, 2>& v, const T z_, const T w_)
		: Base(v, z_, w_)
	{
	}

	TQuat(const TVec<T, 3>& v, const T w_)
		: Base(v, w_)
	{
	}

	TQuat(const Base& v)
		: Base(v.x(), v.y(), v.z(), v.w())
	{
	}

	explicit TQuat(const TMat<T, 3, 3>& m3)
	{
		const T trace = m3(0, 0) + m3(1, 1) + m3(2, 2) + T(1.0);
		if(trace > EPSILON)
		{
			T s = T(0.5) / sqrt<T>(trace);
			w() = T(0.25) / s;
			x() = (m3(2, 1) - m3(1, 2)) * s;
			y() = (m3(0, 2) - m3(2, 0)) * s;
			z() = (m3(1, 0) - m3(0, 1)) * s;
		}
		else
		{
			if(m3(0, 0) > m3(1, 1) && m3(0, 0) > m3(2, 2))
			{
				T s = T(0.5) / sqrt<T>(T(1.0) + m3(0, 0) - m3(1, 1) - m3(2, 2));
				w() = (m3(1, 2) - m3(2, 1)) * s;
				x() = T(0.25) / s;
				y() = (m3(0, 1) + m3(1, 0)) * s;
				z() = (m3(0, 2) + m3(2, 0)) * s;
			}
			else if(m3(1, 1) > m3(2, 2))
			{
				T s = T(0.5) / sqrt<T>(T(1.0) + m3(1, 1) - m3(0, 0) - m3(2, 2));
				w() = (m3(0, 2) - m3(2, 0)) * s;
				x() = (m3(0, 1) + m3(1, 0)) * s;
				y() = T(0.25) / s;
				z() = (m3(1, 2) + m3(2, 1)) * s;
			}
			else
			{
				T s = T(0.5) / sqrt<T>(T(1.0) + m3(2, 2) - m3(0, 0) - m3(1, 1));
				w() = (m3(0, 1) - m3(1, 0)) * s;
				x() = (m3(0, 2) + m3(2, 0)) * s;
				y() = (m3(1, 2) + m3(2, 1)) * s;
				z() = T(0.25) / s;
			}
		}
	}

	explicit TQuat(const TMat<T, 3, 4>& m)
		: TQuat(m.getRotationPart())
	{
		ANKI_ASSERT(isZero<T>(m(0, 3)) && isZero<T>(m(1, 3)) && isZero<T>(m(2, 3)));
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
		const T lengthsq = axisang.getAxis().getLengthSquared();
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

	/// @name Operators with same type
	/// @{
	TQuat& operator=(const TQuat& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
		w() = b.w();
		return *this;
	}
	/// @}

	/// @name Other
	/// @{

	/// Calculates the rotation from TVec3 "from" to "to"
	void setFrom2Vec3(const TVec<T, 3>& from, const TVec<T, 3>& to)
	{
		TVec<T, 3> axis(from.cross(to));
		*this = TQuat(axis.x(), axis.y(), axis.z(), from.dot(to));
		normalize();
		w() += 1.0;

		if(w() <= EPSILON)
		{
			if(from.z() * from.z() > from.x() * from.x())
			{
				*this = TQuat(0.0, from.z(), -from.y(), 0.0);
			}
			else
			{
				*this = TQuat(from.y(), -from.x(), 0.0, 0.0);
			}
		}
		normalize();
	}

	TQuat getInverted() const
	{
		T norm = getLengthSquared();

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

	/// Returns slerp(this, q1, t)
	TQuat slerp(const TQuat& q1_, const T t) const
	{
		TQuat q1 = q1_;
		const TQuat& q0 = *this;
		T cosHalfTheta = q0.dot(q1);
		if(cosHalfTheta < 0.0)
		{
			q1 = -q1; // quat changes
			cosHalfTheta = -cosHalfTheta;
		}

		if(absolute<T>(cosHalfTheta) >= 1.0)
		{
			return TQuat(q0);
		}

		const T halfTheta = acos<T>(cosHalfTheta);
		const T sinHalfTheta = sqrt<T>(T(1) - cosHalfTheta * cosHalfTheta);

		if(absolute<T>(sinHalfTheta) < 0.001)
		{
			return TQuat((q0 + q1) * 0.5);
		}

		const T ratioA = sin<T>((T(1) - t) * halfTheta) / sinHalfTheta;
		const T ratioB = sin<T>(t * halfTheta) / sinHalfTheta;
		TQuat tmp, tmp1, sum;
		tmp = q0 * ratioA;
		tmp1 = q1 * ratioB;
		sum = tmp + tmp1;
		sum.normalize();
		return TQuat(sum);
	}

	/// @note 16 muls, 12 adds
	TQuat combineRotations(const TQuat& b) const
	{
		// XXX See if this can be optimized
		TQuat out;
		out.x() = x() * b.w() + y() * b.z() - z() * b.y() + w() * b.x();
		out.y() = -x() * b.z() + y() * b.w() + z() * b.x() + w() * b.y();
		out.z() = x() * b.y() - y() * b.x() + z() * b.w() + w() * b.z();
		out.w() = -x() * b.x() - y() * b.y() - z() * b.z() + w() * b.w();
		return out;
	}

	/// Returns q * this * q.Conjucated() aka returns a rotated this. 18 muls, 12 adds
	TVec<T, 3> rotate(const TVec<T, 3>& v) const
	{
		ANKI_ASSERT(isZero<T>(1.0 - Base::getLength())); // Not normalized quat
		TVec<T, 3> qXyz(Base::xyz());
		return v + qXyz.cross(qXyz.cross(v) + v * Base::w()) * 2.0;
	}

	void setIdentity()
	{
		*this = getIdentity();
	}

	static TQuat getIdentity()
	{
		return TQuat(0.0, 0.0, 0.0, 1.0);
	}
	/// @}
};

/// F32 quaternion
using Quat = TQuat<F32>;

/// F64 quaternion
using DQuat = TQuat<F64>;
/// @}

} // end namespace anki
