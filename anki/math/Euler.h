// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/Common.h>

namespace anki
{

/// @addtogroup math
/// @{

/// Euler angles. Used for rotations. It cannot describe a rotation accurately though.
/// The 'x' denotes a rotation around x axis, 'y' around y axis and 'z' around z axis.
template<typename T>
class TEuler
{
public:
	/// @name Constructors
	/// @{
	TEuler()
	{
	}

	TEuler(const T x_, const T y_, const T z_)
	{
		x() = x_;
		y() = y_;
		z() = z_;
	}

	TEuler(const TEuler& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
	}

	explicit TEuler(const TQuat<T>& q)
	{
		const T test = q.x() * q.y() + q.z() * q.w();
		if(test > T(0.499))
		{
			y() = T(2) * atan2<T>(q.x(), q.w());
			z() = PI / T(2);
			x() = T(0);
		}
		else if(test < T(-0.499))
		{
			y() = -T(2) * atan2<T>(q.x(), q.w());
			z() = -PI / T(2);
			x() = T(0);
		}
		else
		{
			const T sqx = q.x() * q.x();
			const T sqy = q.y() * q.y();
			const T sqz = q.z() * q.z();
			y() = atan2<T>(T(2) * q.y() * q.w() - T(2) * q.x() * q.z(), T(1) - T(2) * sqy - T(2) * sqz);
			z() = asin<T>(T(2) * test);
			x() = atan2<T>(T(2) * q.x() * q.w() - T(2) * q.y() * q.z(), T(1) - T(2) * sqx - T(2) * sqz);
		}
	}

	explicit TEuler(const TMat<T, 3, 3>& m3)
	{
		if(m3(1, 0) > T(0.998))
		{
			// Singularity at north pole
			y() = atan2(m3(0, 2), m3(2, 2));
			z() = PI / T(2);
			x() = T(0);
		}
		else if(m3(1, 0) < T(-0.998))
		{
			// Singularity at south pole
			y() = atan2(m3(0, 2), m3(2, 2));
			z() = -PI / T(2);
			x() = T(0);
		}
		else
		{
			y() = atan2(-m3(2, 0), m3(0, 0));
			z() = asin(m3(1, 0));
			x() = atan2(-m3(1, 2), m3(1, 1));
		}
	}
	/// @}

	/// @name Accessors
	/// @{
	T& operator[](const U i)
	{
		return m_arr[i];
	}

	T operator[](const U i) const
	{
		return m_arr[i];
	}

	T& x()
	{
		return m_vec.m_x;
	}

	T x() const
	{
		return m_vec.m_x;
	}

	T& y()
	{
		return m_vec.m_y;
	}

	T y() const
	{
		return m_vec.m_y;
	}

	T& z()
	{
		return m_vec.m_z;
	}

	T z() const
	{
		return m_vec.m_z;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TEuler& operator=(const TEuler& b)
	{
		x() = b.x();
		y() = b.y();
		z() = b.z();
		return *this;
	}
	/// @}

	/// @name Other
	/// @{
	ANKI_ENABLE_METHOD(std::is_floating_point<T>::value)
	void toString(StringAuto& str) const
	{
		str.sprintf("%f %f %f", m_vec.m_x, m_vec.m_y, m_vec.m_z);
	}
	/// @}

private:
	/// @name Data
	/// @{
	struct Vec
	{
		T m_x, m_y, m_z;
	};

	union
	{
		Vec m_vec;
		Array<T, 3> m_arr;
	};
	/// @}
};

/// F32 Euler angles
using Euler = TEuler<F32>;

/// F64 Euler angles
using DEuler = TEuler<F64>;
/// @}

} // end namespace anki
