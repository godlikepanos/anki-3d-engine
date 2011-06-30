#ifndef EULER_H
#define EULER_H

#include "Common.h"


namespace M {


/// Used for rotations. It cannot describe a rotation accurately though
class Euler
{
	public:
		/// @name Constructors
		/// @{
		explicit Euler();
		explicit Euler(float x, float y, float z);
		         Euler(const Euler& b);
		explicit Euler(const Quat& q);
		explicit Euler(const Mat3& m3);
		/// @}

		/// @name Accessors
		/// @{
		float& operator [](uint i);
		float operator [](uint i) const;
		float& x();
		float x() const;
		float& y();
		float y() const;
		float& z();
		float z() const;
		/// @}

		/// @name Operators with same
		/// @{
		Euler& operator=(const Euler& b);
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


/// @name Other operators
/// @{
extern std::ostream& operator<<(std::ostream& s, const Euler& e);
/// @}


} // end namespace


#include "Euler.inl.h"


#endif
