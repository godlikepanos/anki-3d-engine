#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Axisang::Axisang()
	: ang(0.0), axis(0.0)
{}

// Copy
inline Axisang::Axisang(const Axisang& b)
	: ang(b.ang), axis(b.axis)
{}

// F32, axis
inline Axisang::Axisang(const F32 rad, const Vec3& axis_)
	: ang(rad), axis(axis_)
{}

// Quat
inline Axisang::Axisang(const Quat& q)
{
	ang = 2.0 * acos(q.w());
	F32 length = sqrt(1.0 - q.w() * q.w());
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
	if((fabs(m3(0, 1) - m3(1, 0)) < getEpsilon<F32>()) 
		&&(fabs(m3(0, 2) - m3(2, 0)) < getEpsilon<F32>()) 
		&& (fabs(m3(1, 2) - m3(2, 1)) < getEpsilon<F32>()))
	{

		if((fabs(m3(0, 1) + m3(1, 0)) < 0.1) 
			&& (fabs(m3(0, 2) + m3(2, 0)) < 0.1) 
			&& (fabs(m3(1, 2) + m3(2, 1)) < 0.1) 
			&& (fabs(m3(0, 0) + m3(1, 1) + m3(2, 2)) - 3) < 0.1)
		{
			axis = Vec3(1.0, 0.0, 0.0);
			ang = 0.0;
			return;
		}

		ang = getPi<F32>();
		axis.x() = (m3(0, 0)+1) / 2.0;
		if(axis.x() > 0.0)
		{
			axis.x() = sqrt(axis.x());
		}
		else
		{
			axis.x() = 0;
		}
		axis.y() = (m3(1, 1)+1)/2;
		if(axis.y() > 0)
		{
			axis.y() = sqrt(axis.y());
		}
		else
		{
			axis.y() = 0;
		}

		axis.z() = (m3(2, 2)+1)/2;
		if(axis.z() > 0)
		{
			axis.z() = sqrt(axis.z());
		}
		else
		{
			axis.z() = 0.0;
		}

		Bool xZero = (fabs(axis.x()) < getEpsilon<F32>());
		Bool yZero = (fabs(axis.y()) < getEpsilon<F32>());
		Bool zZero = (fabs(axis.z()) < getEpsilon<F32>());
		Bool xyPositive = (m3(0, 1) > 0);
		Bool xzPositive = (m3(0, 2) > 0);
		Bool yzPositive = (m3(1, 2) > 0);
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

	F32 s = sqrt((m3(2, 1) - m3(1, 2)) * (m3(2, 1) - m3(1, 2)) 
		+ (m3(0, 2) - m3(2, 0)) * (m3(0, 2) - m3(2, 0)) 
		+ (m3(1, 0) - m3(0, 1)) * (m3(1, 0) - m3(0, 1)));

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

inline F32 Axisang::getAngle() const
{
	return ang;
}

inline F32& Axisang::getAngle()
{
	return ang;
}

inline void Axisang::setAngle(F32 a)
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

//==============================================================================
// Friends                                                                     =
//==============================================================================

// Print
inline std::ostream& operator<<(std::ostream& s, const Axisang& a)
{
	s << "axis: " << a.getAxis() << ", angle: " << a.getAngle();
	return s;
}

} // end namespace
