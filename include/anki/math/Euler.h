#ifndef ANKI_MATH_EULER_H
#define ANKI_MATH_EULER_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
///

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
