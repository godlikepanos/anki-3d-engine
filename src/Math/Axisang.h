#ifndef M_AXISANG_H
#define M_AXISANG_H

#include "Common.h"


namespace M {


/// Used for rotations
class Axisang
{
	public:
		/// @name Constructors
		/// @{
		explicit Axisang();
		         Axisang(const Axisang& b);
		explicit Axisang(float rad, const Vec3& axis_);
		explicit Axisang(const Quat& q);
		explicit Axisang(const Mat3& m3);
		/// @}
		
		/// @name Accessors
		/// @{
		float getAngle() const;
		float& getAngle();
		void setAngle(float a);
		
		const Vec3& getAxis() const;
		Vec3& getAxis();
		void setAxis(const Vec3& a);
		/// @}
		
		/// @name Operators with same
		/// @{
		Axisang& operator=(const Axisang& b);
		/// @}

	private:
		/// @name Data
		/// @{
		float ang;
		Vec3 axis;
		/// @}
};


} // end namespace


#include "Axisang.inl.h"


#endif
