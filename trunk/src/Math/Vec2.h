#ifndef VEC2_H
#define VEC2_H

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// 2D vector
class Vec2
{
	public:
		/// @name Data members
		/// @{
		float x;
		float y;
		/// @}

		/// @name Accessors
		/// @{
		float& operator [](uint i);
		float  operator [](uint i) const;
		/// @}

		/// @name Constructors & distructors
		/// @{
		explicit Vec2();
		explicit Vec2(float f);
		explicit Vec2(float x, float y);
		         Vec2(const Vec2& b);
		explicit Vec2(const Vec3& v3);
		explicit Vec2(const Vec4& v4);
		/// @}

		/// @name Operators with same type
		/// @{
		Vec2 operator +(const Vec2& b) const;
		Vec2& operator +=(const Vec2& b);
		Vec2 operator -(const Vec2& b) const;
		Vec2& operator -=(const Vec2& b);
		Vec2 operator *(const Vec2& b) const;
		Vec2& operator *=(const Vec2& b);
		Vec2 operator /(const Vec2& b) const;
		Vec2& operator /=(const Vec2& b);
		Vec2 operator -() const;
		bool operator ==(const Vec2& b) const;
		bool operator !=(const Vec2& b) const;
		/// @}

		/// @name Operators with float
		/// @{
		Vec2 operator +(float f) const;
		Vec2& operator +=(float f);
		Vec2 operator -(float f) const;
		Vec2& operator -=(float f);
		Vec2 operator *(float f) const;
		Vec2& operator *=(float f);
		Vec2 operator /(float f) const;
		Vec2& operator /=(float f);
		/// @}

		/// @name Other
		/// @{
		float getLength() const;
		Vec2 getNormalized() const;
		void normalize();
		float dot(const Vec2& b) const;
		/// @}
};


/// @name Other operators
/// @{
extern Vec2 operator +(float f, const Vec2& v2);
extern Vec2 operator -(float f, const Vec2& v2);
extern Vec2 operator *(float f, const Vec2& v2);
extern Vec2 operator /(float f, const Vec2& v2);
extern ostream& operator<<(ostream& s, const Vec2& v);
/// @}


} // end namespace


#include "Vec2.inl.h"


#endif
