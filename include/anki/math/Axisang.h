#ifndef ANKI_MATH_AXISANG_H
#define ANKI_MATH_AXISANG_H

#include "anki/math/Vec3.h"
#include "anki/math/CommonIncludes.h"

namespace anki {

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
