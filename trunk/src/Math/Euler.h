#ifndef EULER_H
#define EULER_H

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// Used for rotations. It cannot describe a rotation accurately though
class Euler
{
	public:
		/// @name Data
		/// @{
		float x, y, z;
		/// @}

		/// @name Constructors & distructors
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
		float& getBank();
		float getBank() const;
		float& getHeading();
		float getHeading() const;
		float& getAttitude();
		float getAttitude() const;
		/// @}
};


/// @name Other operators
/// @{
extern ostream& operator<<(ostream& s, const Euler& e);
/// @}


} // end namespace


#include "Euler.inl.h"


#endif
