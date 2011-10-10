#ifndef AXISANG_H
#define AXISANG_H

#include "Vec3.h"
#include "MathCommonIncludes.h"


/// @addtogroup Math
/// @{

/// Axis angles. Used for rotations
class Axisang
{
	public:
		/// @name Constructors
		/// @{
		explicit Axisang();
		         Axisang(const Axisang& b);
		explicit Axisang(const float rad, const Vec3& axis_);
		explicit Axisang(const Quat& q);
		explicit Axisang(const Mat3& m3);
		/// @}
		
		/// @name Accessors
		/// @{
		float getAngle() const;
		float& getAngle();
		void setAngle(const float a);
		
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
		float ang;
		Vec3 axis;
		/// @}
};
/// @}


#include "Axisang.inl.h"


#endif
