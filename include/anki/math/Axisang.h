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
		: ang(0), axis(0)
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
		if(isZero<T>(length))
		{
			axis = TVec3<T>(0.0);
		}
		else
		{
			length = 1.0 / length;
			axis = TVec3<T>(q.x() * length, q.y() * length, q.z() * length);
		}
	}

	explicit TAxisang(const TMat3<T>& m3)
	{
		if((fabs(m3(0, 1) - m3(1, 0)) < getEpsilon<T>()) 
			&&(fabs(m3(0, 2) - m3(2, 0)) < getEpsilon<T>()) 
			&& (fabs(m3(1, 2) - m3(2, 1)) < getEpsilon<T>()))
		{

			if((fabs(m3(0, 1) + m3(1, 0)) < 0.1) 
				&& (fabs(m3(0, 2) + m3(2, 0)) < 0.1) 
				&& (fabs(m3(1, 2) + m3(2, 1)) < 0.1) 
				&& (fabs(m3(0, 0) + m3(1, 1) + m3(2, 2)) - 3) < 0.1)
			{
				axis = TVec3<T>(1.0, 0.0, 0.0);
				ang = 0.0;
				return;
			}

			ang = getPi<T>();
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

			Bool xZero = (fabs(axis.x()) < getEpsilon<T>());
			Bool yZero = (fabs(axis.y()) < getEpsilon<T>());
			Bool zZero = (fabs(axis.z()) < getEpsilon<T>());
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

		ang = acos((m3(0, 0) + m3(1, 1) + m3(2, 2) - 1) / 2);
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
	std::string toString()
	{
		std::string s;
		//std::string s = << "axis: " << a.getAxis() << ", angle: " << a.getAngle();
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

/// Axis angles. Used for rotations
class Axisang
{
public:
	/// @name Constructors
	/// @{
	explicit Axisang();
	         Axisang(const Axisang& b);
	explicit Axisang(const F32 rad, const Vec3& axis_);
	explicit Axisang(const Quat& q);
	explicit Axisang(const Mat3& m3);
	/// @}

	/// @name Accessors
	/// @{
	F32 getAngle() const;
	F32& getAngle();
	void setAngle(const F32 a);

	const Vec3& getAxis() const;
	Vec3& getAxis();
	void setAxis(const Vec3& a);
	/// @}

	/// @name Operators with same type
	/// @{
	Axisang& operator=(const Axisang& b);
	/// @}

	/// @name Friends
	/// @{
	friend std::ostream& operator<<(std::ostream& s, const Axisang& a);
	/// @}

private:
	/// @name Data
	/// @{
	F32 ang;
	Vec3 axis;
	/// @}
};
/// @}

} // end namespace

#include "anki/math/Axisang.inl.h"

#endif
