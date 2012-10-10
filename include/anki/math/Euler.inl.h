#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Euler::Euler()
{
	x() = y() = z() = 0.0;
}

// F32, F32, F32
inline Euler::Euler(const F32 x_, const F32 y_, const F32 z_)
{
	x() = x_;
	y() = y_;
	z() = z_;
}

// Copy
inline Euler::Euler(const Euler& b)
{
	x() = b.x();
	y() = b.y();
	z() = b.z();
}

// Quat
inline Euler::Euler(const Quat& q)
{
	F32 test = q.x() * q.y() + q.z() * q.w();
	if(test > 0.499)
	{
		y() = 2.0 * atan2(q.x(), q.w());
		z() = getPi<F32>() / 2.0;
		x() = 0.0;
		return;
	}
	if(test < -0.499)
	{
		y() = -2.0 * atan2(q.x(), q.w());
		z() = -getPi<F32>() / 2.0;
		x() = 0.0;
		return;
	}

	F32 sqx = q.x() * q.x();
	F32 sqy = q.y() * q.y();
	F32 sqz = q.z() * q.z();
	y() = atan2(2.0 * q.y() * q.w() - 2.0 * q.x() * q.z(),
		1.0 - 2.0 * sqy - 2.0 * sqz);
	z() = asin(2.0 * test);
	x() = atan2(2.0 * q.x() * q.w() - 2.0 * q.y() * q.z(),
	    1.0 - 2.0 * sqx - 2.0 * sqz);
}

// mat3
inline Euler::Euler(const Mat3& m3)
{
	F32 cx, sx;
	F32 cy, sy;
	F32 cz, sz;

	sy = m3(0, 2);
	cy = sqrt(1.0 - sy * sy);
	// normal case
	if (!isZero(cy))
	{
		F32 factor = 1.0/cy;
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

	z() = atan2f(sz, cz);
	y() = atan2f(sy, cy);
	x() = atan2f(sx, cx);
}

//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline F32& Euler::operator [](const U i)
{
	return arr[i];
}

inline F32 Euler::operator [](const U i) const
{
	return arr[i];
}

inline F32& Euler::x()
{
	return vec.x;
}

inline F32 Euler::x() const
{
	return vec.x;
}

inline F32& Euler::y()
{
	return vec.y;
}

inline F32 Euler::y() const
{
	return vec.y;
}

inline F32& Euler::z()
{
	return vec.z;
}

inline F32 Euler::z() const
{
	return vec.z;
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Euler& Euler::operator=(const Euler& b)
{
	x() = b.x();
	y() = b.y();
	z() = b.z();
	return *this;
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

inline std::ostream& operator<<(std::ostream& s, const Euler& e)
{
	s << e.x() << ' ' << e.y() << ' ' << e.z();
	return s;
}

} // end namespace
