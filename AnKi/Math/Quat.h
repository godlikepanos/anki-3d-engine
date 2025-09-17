// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math/Common.h>
#include <AnKi/Math/Vec.h>

namespace anki {

/// @addtogroup math
/// @{

/// Quaternion. Used in rotations
template<typename T>
class alignas(16) TQuat
{
public:
	static constexpr Bool kSimdEnabled = std::is_same<T, F32>::value && ANKI_ENABLE_SIMD;
	static constexpr Bool kSseEnabled = kSimdEnabled && ANKI_SIMD_SSE;

	/// @name Constructors
	/// @{
	TQuat()
		: m_value(T(0), T(0), T(0), T(1))
	{
	}

	TQuat(const TQuat& b)
		: m_value(b.m_value)
	{
	}

	explicit TQuat(const T x_, const T y_, const T z_, const T w_)
		: m_value(x_, y_, z_, w_)
	{
	}

	explicit TQuat(const T arr[])
		: m_value(arr)
	{
	}

	explicit TQuat(const TVec<T, 2>& v, const T z_, const T w_)
		: m_value(v, z_, w_)
	{
	}

	explicit TQuat(const TVec<T, 3>& v, const T w_)
		: m_value(v, w_)
	{
	}

	explicit TQuat(const TVec<T, 4>& v)
		: m_value(v)
	{
	}

	/// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
	explicit TQuat(const TMat<T, 3, 3>& m)
	{
		const T tr = m(0, 0) + m(1, 1) + m(2, 2);

		if(tr > T(0))
		{
			const T S = sqrt<T>(tr + T(1)) * T(2); // S=4*qw
			w() = T(0.25) * S;
			x() = (m(2, 1) - m(1, 2)) / S;
			y() = (m(0, 2) - m(2, 0)) / S;
			z() = (m(1, 0) - m(0, 1)) / S;
		}
		else if((m(0, 0) > m(1, 1)) & (m(0, 0) > m(2, 2)))
		{
			const T S = sqrt<T>(T(1) + m(0, 0) - m(1, 1) - m(2, 2)) * T(2); // S=4*qx
			w() = (m(2, 1) - m(1, 2)) / S;
			x() = T(0.25) * S;
			y() = (m(0, 1) + m(1, 0)) / S;
			z() = (m(0, 2) + m(2, 0)) / S;
		}
		else if(m(1, 1) > m(2, 2))
		{
			const T S = sqrt<T>(T(1) + m(1, 1) - m(0, 0) - m(2, 2)) * T(2); // S=4*qy
			w() = (m(0, 2) - m(2, 0)) / S;
			x() = (m(0, 1) + m(1, 0)) / S;
			y() = T(0.25) * S;
			z() = (m(1, 2) + m(2, 1)) / S;
		}
		else
		{
			const T S = sqrt<T>(T(1) + m(2, 2) - m(0, 0) - m(1, 1)) * T(2); // S=4*qz
			w() = (m(1, 0) - m(0, 1)) / S;
			x() = (m(0, 2) + m(2, 0)) / S;
			y() = (m(1, 2) + m(2, 1)) / S;
			z() = T(0.25) * S;
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
		sinCos(eu.y() * T(0.5), sx, cx);

		T cy, sy;
		sinCos(eu.z() * T(0.5), sy, cy);

		T cz, sz;
		sinCos(eu.x() * T(0.5), sz, cz);

		const T cxcy = cx * cy;
		const T sxsy = sx * sy;
		x() = cxcy * sz + sxsy * cz;
		y() = sx * cy * cz + cx * sy * sz;
		z() = cx * sy * cz - sx * cy * sz;
		w() = cxcy * cz - sxsy * sz;
	}

	explicit TQuat(const TAxisang<T>& axisang)
	{
		const T lengthsq = axisang.getAxis().lengthSquared();
		if(isZero<T>(lengthsq))
		{
			(*this) = getIdentity();
			return;
		}

		const T rad = axisang.getAngle() * T(0.5);

		T sintheta, costheta;
		sinCos(rad, sintheta, costheta);

		const T scalefactor = sintheta / sqrt(lengthsq);

		m_value = TVec<T, 4>(axisang.getAxis(), costheta) * TVec<T, 4>(scalefactor, scalefactor, scalefactor, T(1));
	}
	/// @}

	/// @name Operators with same type
	/// @{

	/// Copy.
	TQuat& operator=(const TQuat& b)
	{
		m_value = b.m_value;
		return *this;
	}

	Bool operator==(const TQuat& b) const
	{
		return m_value == b.m_value;
	}

	Bool operator!=(const TQuat& b) const
	{
		return m_value != b.m_value;
	}

#if ANKI_SIMD_SSE
	/// Combine rotations (SIMD version)
	TQuat operator*(const TQuat& b) requires(kSseEnabled)
	{
		// Taken from: http://momchil-velikov.blogspot.nl/2013/10/fast-sse-quternion-multiplication.html
		const __m128 abcd = m_value.getSimd();
		const __m128 xyzw = b.m_value.getSimd();

		const __m128 t0 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(3, 3, 3, 3));
		const __m128 t1 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(2, 3, 0, 1));

		const __m128 t3 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(0, 0, 0, 0));
		const __m128 t4 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(1, 0, 3, 2));

		const __m128 t5 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(1, 1, 1, 1));
		const __m128 t6 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(2, 0, 3, 1));

		// [d,d,d,d] * [z,w,x,y] = [dz,dw,dx,dy]
		const __m128 m0 = _mm_mul_ps(t0, t1);

		// [a,a,a,a] * [y,x,w,z] = [ay,ax,aw,az]
		const __m128 m1 = _mm_mul_ps(t3, t4);

		// [b,b,b,b] * [z,x,w,y] = [bz,bx,bw,by]
		const __m128 m2 = _mm_mul_ps(t5, t6);

		// [c,c,c,c] * [w,z,x,y] = [cw,cz,cx,cy]
		const __m128 t7 = _mm_shuffle_ps(abcd, abcd, _MM_SHUFFLE(2, 2, 2, 2));
		const __m128 t8 = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(3, 2, 0, 1));
		const __m128 m3 = _mm_mul_ps(t7, t8);

		// [dz,dw,dx,dy] + -[ay,ax,aw,az] = [dz+ay,dw-ax,dx+aw,dy-az]
		__m128 e = _mm_addsub_ps(m0, m1);

		// [dx+aw,dz+ay,dy-az,dw-ax]
		e = _mm_shuffle_ps(e, e, _MM_SHUFFLE(1, 3, 0, 2));

		// [dx+aw,dz+ay,dy-az,dw-ax] + -[bz,bx,bw,by] = [dx+aw+bz,dz+ay-bx,dy-az+bw,dw-ax-by]
		e = _mm_addsub_ps(e, m2);

		// [dz+ay-bx,dw-ax-by,dy-az+bw,dx+aw+bz]
		e = _mm_shuffle_ps(e, e, _MM_SHUFFLE(2, 0, 1, 3));

		// [dz+ay-bx,dw-ax-by,dy-az+bw,dx+aw+bz] + -[cw,cz,cx,cy] = [dz+ay-bx+cw,dw-ax-by-cz,dy-az+bw+cx,dx+aw+bz-cy]
		e = _mm_addsub_ps(e, m3);

		// [dw-ax-by-cz,dz+ay-bx+cw,dy-az+bw+cx,dx+aw+bz-cy]
		return TQuat(TVec<T, 4>(_mm_shuffle_ps(e, e, _MM_SHUFFLE(2, 3, 1, 0))));
	}
#endif

	/// Combine rotations (non-SIMD version)
	TQuat operator*(const TQuat& b) requires(!kSseEnabled)
	{
		const T lx = m_value.x();
		const T ly = m_value.y();
		const T lz = m_value.z();
		const T lw = m_value.w();

		const T rx = b.m_value.x();
		const T ry = b.m_value.y();
		const T rz = b.m_value.z();
		const T rw = b.m_value.w();

		const T x = lw * rx + lx * rw + ly * rz - lz * ry;
		const T y = lw * ry - lx * rz + ly * rw + lz * rx;
		const T z = lw * rz + lx * ry - ly * rx + lz * rw;
		const T w = lw * rw - lx * rx - ly * ry - lz * rz;

		return Quat(x, y, z, w);
	}

	/// Convert to Vec4.
	explicit operator TVec<T, 4>() const
	{
		return m_value;
	}
	/// @}

	/// @name Operators with other types
	/// @{

	T operator[](const U32 i) const
	{
		return m_value[i];
	}

	T& operator[](const U32 i)
	{
		return m_value[i];
	}

	/// Rotate a vector by this quat.
	TVec<T, 3> operator*(const TVec<T, 3>& inValue) const
	{
		// Rotating a vector by a quaternion is done by: p' = q * p * q^-1 (q^-1 = conjugated(q) for a unit quaternion)
		ANKI_ASSERT(isZero<T>(T(1) - m_value.getLength()));
		return TVec<T, 3>((*this * TQuat(TVec<T, 4>(inValue, T(0))) * conjugated()).m_value.xyz());
	}
	/// @}

	/// @name Accessors
	/// @{
	T x() const
	{
		return m_value.x();
	}

	T& x()
	{
		return m_value.x();
	}

	T y() const
	{
		return m_value.y();
	}

	T& y()
	{
		return m_value.y();
	}

	T z() const
	{
		return m_value.z();
	}

	T& z()
	{
		return m_value.z();
	}

	T w() const
	{
		return m_value.w();
	}

	T& w()
	{
		return m_value.w();
	}
	/// @}

	/// @name Other
	/// @{

	[[nodiscard]] T length() const
	{
		return m_value.length();
	}

	/// Calculates the rotation from vector "from" to "to".
	static TQuat fromPointToPoint(const TVec<T, 3>& from, const TVec<T, 3>& to)
	{
		const TVec<T, 3> axis(from.cross(to));
		TVec<T, 4> quat = TVec4<T, 4>(axis.x(), axis.y(), axis.z(), from.dot(to));
		quat = quat.normalize();
		quat.w() += T(1);

		if(quat.w() <= T(0.0001))
		{
			if(from.z() * from.z() > from.x() * from.x())
			{
				quat = TVec<T, 4>(T(0), from.z(), -from.y(), T(0));
			}
			else
			{
				quat = TVec<T, 4>(from.y(), -from.x(), T(0), T(0));
			}
		}

		quat = quat.normalize();
		return TQuat(quat);
	}

	[[nodiscard]] TQuat invert() const
	{
		const T len = m_value.length();
		ANKI_ASSERT(!isZero<T>(len));
		return conjugated() / len;
	}

	[[nodiscard]] TQuat conjugated() const
	{
		return TQuat(m_value * TVec<T, 4>(T(-1), T(-1), T(-1), T(1)));
	}

	/// Returns slerp(this, q1, t)
	[[nodiscard]] TQuat slerp(const TQuat& destination, const T t) const
	{
		// Difference at which to LERP instead of SLERP
		const T delta = T(0.0001);

		// Calc cosine
		T sinScale1 = T(1);
		T cosComega = m_value.dot(destination.m_value);

		// Adjust signs (if necessary)
		if(cosComega < T(0))
		{
			cosComega = -cosComega;
			sinScale1 = T(-1);
		}

		// Calculate coefficients
		T scale0, scale1;
		if(T(1) - cosComega > delta)
		{
			// Standard case (slerp)
			const F32 omega = acos<T>(cosComega);
			const F32 sinOmega = sin<T>(omega);
			scale0 = sin<T>((T(1) - t) * omega) / sinOmega;
			scale1 = sinScale1 * sin<T>(t * omega) / sinOmega;
		}
		else
		{
			// Quaternions are very close so we can do a linear interpolation
			scale0 = T(1) - t;
			scale1 = sinScale1 * t;
		}

		// Interpolate between the two quaternions
		const TVec<T, 4> v = TVec<T, 4>(scale0) * m_value + TVec<T, 4>(scale1) * destination.m_value;
		return TQuat(v.normalize());
	}

	[[nodiscard]] TQuat rotateXAxis(const T rad) const
	{
		const TQuat r(Axisang<T>(rad, TVec<T, 3>(T(1), T(0), T(0))));
		return r * (*this);
	}

	[[nodiscard]] TQuat rotateYAxis(const T rad) const
	{
		const TQuat r(Axisang<T>(rad, TVec<T, 3>(T(0), T(1), T(0))));
		return r * (*this);
	}

	[[nodiscard]] TQuat rotateZAxis(const T rad) const
	{
		const TQuat r(Axisang<T>(rad, TVec<T, 3>(T(0), T(0), T(1))));
		return r * (*this);
	}

	[[nodiscard]] TQuat normalize() const
	{
		return TQuat(m_value.normalize());
	}

	void setIdentity()
	{
		*this = getIdentity();
	}

	static TQuat getIdentity()
	{
		return TQuat();
	}

	static constexpr U32 getSize()
	{
		return 4;
	}

	String toString() const requires(std::is_floating_point<T>::value)
	{
		return m_value.toString();
	}
	/// @}

private:
	TVec<T, 4> m_value;
};

/// F32 quaternion
using Quat = TQuat<F32>;

/// F64 quaternion
using DQuat = TQuat<F64>;
/// @}

} // end namespace anki
