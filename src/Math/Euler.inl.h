#include "Common.inl.h"


namespace M {


//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Euler::Euler()
{
	x() = y() = z() = 0.0;
}

// float, float, float
inline Euler::Euler(float x_, float y_, float z_)
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
	float test = q.x() * q.y() + q.z() * q.w();
	if(test > 0.499)
	{
		y() = 2.0 * atan2(q.x(), q.w());
		z() = PI / 2.0;
		x() = 0.0;
		return;
	}
	if(test < -0.499)
	{
		y() = -2.0 * atan2(q.x(), q.w());
		z() = -PI / 2.0;
		x() = 0.0;
		return;
	}

	float sqx = q.x() * q.x();
	float sqy = q.y() * q.y();
	float sqz = q.z() * q.z();
	y() = atan2(2.0 * q.y() * q.w() - 2.0 * q.x() * q.z(), 1.0 - 2.0 * sqy - 2.0 * sqz);
	z() = asin(2.0 * test);
	x() = atan2(2.0 * q.x() * q.w() - 2.0 * q.y() * q.z(), 1.0 - 2.0 * sqx - 2.0 * sqz);
}

// mat3
inline Euler::Euler(const Mat3& m3)
{
	float cx, sx;
	float cy, sy;
	float cz, sz;

	sy = m3(0, 2);
	cy = M::sqrt(1.0 - sy*sy);
	// normal case
	if (!isZero(cy))
	{
		float factor = 1.0/cy;
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

inline float& Euler::operator [](uint i)
{
	return arr[i];
}

inline float Euler::operator [](uint i) const
{
	return arr[i];
}

inline float& Euler::x()
{
	return vec.x;
}

inline float Euler::x() const
{
	return vec.x;
}

inline float& Euler::y()
{
	return vec.y;
}

inline float Euler::y() const
{
	return vec.y;
}

inline float& Euler::z()
{
	return vec.z;
}

inline float Euler::z() const
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
	return SELF;
}


//==============================================================================
// Print                                                                       =
//==============================================================================

inline std::ostream& operator<<(std::ostream& s, const Euler& e)
{
	s << e.x() << ' ' << e.y() << ' ' << e.z();
	return s;
}


} // end namespace
