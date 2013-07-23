#ifndef ANKI_MATH_EULER_H
#define ANKI_MATH_EULER_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Euler angles. Used for rotations. It cannot describe a rotation
/// accurately though
template<typename T>
class TEuler
{
public:
	/// @name Constructors
	/// @{
	explicit TEuler()
	{
		x() = y() = z() = 0.0;
	}

	explicit TEuler(const T x_, const T y_, const T z_)
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
		T test = q.x() * q.y() + q.z() * q.w();
		if(test > 0.499)
		{
			y() = 2.0 * atan2(q.x(), q.w());
			z() = getPi<T>() / 2.0;
			x() = 0.0;
			return;
		}
		if(test < -0.499)
		{
			y() = -2.0 * atan2(q.x(), q.w());
			z() = -getPi<T>() / 2.0;
			x() = 0.0;
			return;
		}

		T sqx = q.x() * q.x();
		T sqy = q.y() * q.y();
		T sqz = q.z() * q.z();
		y() = atan2(2.0 * q.y() * q.w() - 2.0 * q.x() * q.z(),
			1.0 - 2.0 * sqy - 2.0 * sqz);
		z() = asin(2.0 * test);
		x() = atan2(2.0 * q.x() * q.w() - 2.0 * q.y() * q.z(),
			1.0 - 2.0 * sqx - 2.0 * sqz);
	}

	explicit TEuler(const TMat3<T>& m3)
	{
		T cx, sx;
		T cy, sy;
		T cz, sz;

		sy = m3(0, 2);
		cy = sqrt(1.0 - sy * sy);
		// normal case
		if (!isZero<T>(cy))
		{
			T factor = 1.0 / cy;
			sx = -m3(1, 2) * factor;
			cx = m3(2, 2) * factor;
			sz = -m3(0, 1) * factor;
			cz = m3(0, 0) * factor;
		}
		// x and z axes aligned
		else
		{
			sz = 0.0;
			cz = 1.0;
			sx = m3(2, 1);
			cx = m3(1, 1);
		}

		z() = atan2(sz, cz);
		y() = atan2(sy, cy);
		x() = atan2(sx, cx);
	}
	/// @}

	/// @name Accessors
	/// @{
	T& operator [](const U i)
	{
		return arr[i];
	}

	T operator [](const U i) const
	{
		return arr[i];
	}

	T& x()
	{
		return vec.x;
	}

	T x() const
	{
		return vec.x;
	}

	T& y()
	{
		return vec.y;
	}

	T y() const
	{
		return vec.y;
	}

	T& z()
	{
		return vec.z;
	}

	T z() const
	{
		return vec.z;
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
	std::string toString();
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			T x, y, z;
		} vec;

		Array<T, 3> arr;
	};
	/// @}
};

/// Euler angles. Used for rotations. It cannot describe a rotation
/// accurately though
class Euler
{
public:
	/// @name Constructors
	/// @{
	explicit Euler();
	explicit Euler(const F32 x, const F32 y, const F32 z);
			 Euler(const Euler& b);
	explicit Euler(const Quat& q);
	explicit Euler(const Mat3& m3);
	/// @}

	/// @name Accessors
	/// @{
	F32& operator [](const U i);
	F32 operator [](const U i) const;
	F32& x();
	F32 x() const;
	F32& y();
	F32 y() const;
	F32& z();
	F32 z() const;
	/// @}

	/// @name Operators with same type
	/// @{
	Euler& operator=(const Euler& b);
	/// @}

	/// @name Friends
	/// @{
	friend std::ostream& operator<<(std::ostream& s, const Euler& e);
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			F32 x, y, z;
		} vec;

		Array<F32, 3> arr;
	};
	/// @}
};
/// @}

} // end namespace

#include "anki/math/Euler.inl.h"

#endif
