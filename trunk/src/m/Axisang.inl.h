#include "Common.inl.h"


namespace m {


//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Axisang::Axisang()
:	ang(0.0),
	axis(0.0)
{}

// Copy
inline Axisang::Axisang(const Axisang& b)
:	ang(b.ang),
	axis(b.axis)
{}

// float, axis
inline Axisang::Axisang(float rad, const Vec3& axis_)
:	ang(rad),
	axis(axis_)
{}

// Quat
inline Axisang::Axisang(const Quat& q)
{
	ang = 2.0 * acos(q.w());
	float length = m::sqrt(1.0 - q.w() * q.w());
	if(isZero(length))
	{
		axis = Vec3(0.0);
	}
	else
	{
		length = 1.0 / length;
		axis = Vec3(q.x() * length, q.y() * length, q.z() * length);
	}
}

// constructor [mat3]
inline Axisang::Axisang(const Mat3& m3)
{
	if((fabs(m3(0, 1) - m3(1, 0)) < EPSILON) &&
		(fabs(m3(0, 2) - m3(2, 0)) < EPSILON) &&
		(fabs(m3(1, 2) - m3(2, 1)) < EPSILON))
	{

		if((fabs(m3(0, 1) + m3(1, 0)) < 0.1) &&
			(fabs(m3(0, 2) + m3(2, 0)) < 0.1) &&
			(fabs(m3(1, 2) + m3(2, 1)) < 0.1) &&
			(fabs(m3(0, 0) + m3(1, 1) + m3(2, 2)) - 3) < 0.1)
		{
			axis = Vec3(1.0, 0.0, 0.0);
			ang = 0.0;
			return;
		}

		ang = PI;
		axis.x() = (m3(0, 0)+1) / 2.0;
		if(axis.x() > 0.0)
		{
			axis.x() = m::sqrt(axis.x());
		}
		else
		{
			axis.x() = 0;
		}
		axis.y() = (m3(1, 1)+1)/2;
		if(axis.y() > 0)
		{
			axis.y() = m::sqrt(axis.y());
		}
		else
		{
			axis.y() = 0;
		}

		axis.z() = (m3(2, 2)+1)/2;
		if(axis.z() > 0)
		{
			axis.z() = m::sqrt(axis.z());
		}
		else
		{
			axis.z() = 0.0;
		}

		bool xZero = (fabs(axis.x()) < EPSILON);
		bool yZero = (fabs(axis.y()) < EPSILON);
		bool zZero = (fabs(axis.z()) < EPSILON);
		bool xyPositive = (m3(0, 1) > 0);
		bool xzPositive = (m3(0, 2) > 0);
		bool yzPositive = (m3(1, 2) > 0);
		if(xZero && !yZero && !zZero)
		{
			if(!yzPositive)
			{
				axis.y() = -axis.y();
			}
		}
		else if(yZero && !zZero)
		{
			if(!xzPositive)
			{
				axis.z() = -axis.z();
			}
		}
		else if(zZero)
		{
			if(!xyPositive)
			{
				axis.x() = -axis.x();
			}
		}

		return;
	}

	float s = m::sqrt((m3(2, 1) - m3(1, 2)) * (m3(2, 1) - m3(1, 2)) +
		(m3(0, 2) - m3(2, 0)) * (m3(0, 2) - m3(2, 0)) +
		(m3(1, 0) - m3(0, 1)) * (m3(1, 0) - m3(0, 1)));

	if(fabs(s) < 0.001)
	{
		s = 1;
	}

	ang = acos((m3(0, 0) + m3(1, 1) + m3(2, 2) - 1) / 2);
	axis.x() = (m3(2, 1) - m3(1, 2)) / s;
	axis.y() = (m3(0, 2) - m3(2, 0)) / s;
	axis.z() = (m3(1, 0) - m3(0, 1)) / s;
}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline float Axisang::getAngle() const
{
	return ang;
}

inline float& Axisang::getAngle()
{
	return ang;
}

inline void Axisang::setAngle(float a)
{
	ang = a;
}

inline const Vec3& Axisang::getAxis() const
{
	return axis;
}

inline Vec3& Axisang::getAxis()
{
	return axis;
}

inline void Axisang::setAxis(const Vec3& a)
{
	axis = a;
}


//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Axisang& Axisang::operator=(const Axisang& b)
{
	ang = b.ang;
	axis = b.axis;
	return *this;
}


} // end namaspace
