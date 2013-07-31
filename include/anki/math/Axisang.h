#ifndef ANKI_MATH_AXISANG_H
#define ANKI_MATH_AXISANG_H

#include "anki/math/Vec3.h"
#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Axis angles. Used for rotations
template<typename T>
class TAxisang
{
public:
	/// @name Constructors
	/// @{
	explicit TAxisang()
		: ang(0.0), axis(0.0)
	{}
	
	TAxisang(const TAxisang& b)
		: ang(b.ang), axis(b.axis)
	{}

	explicit TAxisang(const T rad, const TVec3<T>& axis_)
		: ang(rad), axis(axis_)
	{}

	explicit TAxisang(const TQuat<T>& q)
	{
		ang = 2.0 * acos(q.w());
		T length = sqrt(1.0 - q.w() * q.w());
		if(!isZero<T>(length))
		{
			length = 1.0 / length;
			axis = TVec3<T>(q.x() * length, q.y() * length, q.z() * length);
		}
		else
		{
			axis = TVec3<T>(0.0);
		}
	}

	explicit TAxisang(const TMat3<T>& m3)
	{
		if(isZero<T>(m3(0, 1) - m3(1, 0))
			&& isZero<T>(m3(0, 2) - m3(2, 0)) 
			&& isZero<T>(m3(1, 2) - m3(2, 1)))
		{

			if((fabs<T>(m3(0, 1) + m3(1, 0)) < 0.1) 
				&& (fabs<T>(m3(0, 2) + m3(2, 0)) < 0.1) 
				&& (fabs<T>(m3(1, 2) + m3(2, 1)) < 0.1) 
				&& (fabs<T>(m3(0, 0) + m3(1, 1) + m3(2, 2)) - 3) < 0.1)
			{
				axis = TVec3<T>(1.0, 0.0, 0.0);
				ang = 0.0;
				return;
			}

			ang = getPi<T>();
			axis.x() = (m3(0, 0) + 1.0) / 2.0;
			if(axis.x() > 0.0)
			{
				axis.x() = sqrt(axis.x());
			}
			else
			{
				axis.x() = 0.0;
			}
			axis.y() = (m3(1, 1) + 1.0) / 2.0;
			if(axis.y() > 0.0)
			{
				axis.y() = sqrt(axis.y());
			}
			else
			{
				axis.y() = 0.0;
			}

			axis.z() = (m3(2, 2) + 1.0) / 2.0;
			if(axis.z() > 0.0)
			{
				axis.z() = sqrt(axis.z());
			}
			else
			{
				axis.z() = 0.0;
			}

			Bool xZero = isZero<T>(axis.x());
			Bool yZero = isZero<T>(axis.y());
			Bool zZero = isZero<T>(axis.z());
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

		T s = sqrt((m3(2, 1) - m3(1, 2)) * (m3(2, 1) - m3(1, 2)) 
			+ (m3(0, 2) - m3(2, 0)) * (m3(0, 2) - m3(2, 0)) 
			+ (m3(1, 0) - m3(0, 1)) * (m3(1, 0) - m3(0, 1)));

		if(fabs(s) < 0.001)
		{
			s = 1;
		}

		ang = acos((m3(0, 0) + m3(1, 1) + m3(2, 2) - 1.0) / 2.0);
		axis.x() = (m3(2, 1) - m3(1, 2)) / s;
		axis.y() = (m3(0, 2) - m3(2, 0)) / s;
		axis.z() = (m3(1, 0) - m3(0, 1)) / s;		
	}
	/// @}

	/// @name Accessors
	/// @{
	T getAngle() const
	{
		return ang;
	}

	T& getAngle()
	{
		return ang;
	}

	void setAngle(const T a)
	{
		ang = a;
	}

	const TVec3<T>& getAxis() const
	{
		return axis;
	}

	TVec3<T>& getAxis()
	{
		return axis;
	}

	void setAxis(const TVec3<T>& a)
	{
		axis = a;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TAxisang& operator=(const TAxisang& b)
	{
		ang = b.ang;
		axis = b.axis;
		return *this;
	}
	/// @}

	/// @name Other
	/// @{
	std::string toString() const
	{
		std::string s = "axis: " + axis.toString() 
			+ ", angle: " + std::to_string(ang);
		return s;
	}
	/// @}

private:
	/// @name Data
	/// @{
	T ang;
	TVec3<T> axis;
	/// @}
};

/// F32 Axisang
typedef TAxisang<F32> Axisang;

/// @}

} // end namespace

#endif
