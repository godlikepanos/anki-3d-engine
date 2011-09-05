#ifndef EULER_H
#define EULER_H

#include "MathCommonIncludes.h"


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
		explicit Euler(const float x, const float y, const float z);
		         Euler(const Euler& b);
		explicit Euler(const Quat& q);
		explicit Euler(const Mat3& m3);
		/// @}

		/// @name Accessors
		/// @{
		float& operator [](const size_t i);
		float operator [](const size_t i) const;
		float& x();
		float x() const;
		float& y();
		float y() const;
		float& z();
		float z() const;
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
				float x, y, z;
			} vec;

			boost::array<float, 3> arr;
		};
		/// @}
};
/// @}


#include "Euler.inl.h"


#endif
