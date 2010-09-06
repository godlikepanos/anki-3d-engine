#ifndef _VEC2_H_
#define _VEC2_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// 2D vector
class Vec2
{
	public:
		// data members
		float x;
		float y;
		// accessors
		float& operator [](uint i);
		float  operator [](uint i) const;
		// constructors & distructors
		explicit Vec2();
		explicit Vec2(float f);
		explicit Vec2(float x, float y);
		         Vec2(const Vec2& b);
		explicit Vec2(const Vec3& v3);
		explicit Vec2(const Vec4& v4);
		// ops with same type
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
		// ops with float
		Vec2 operator +(float f) const;
		Vec2& operator +=(float f);
		Vec2 operator -(float f) const;
		Vec2& operator -=(float f);
		Vec2 operator *(float f) const;
		Vec2& operator *=(float f);
		Vec2 operator /(float f) const;
		Vec2& operator /=(float f);
		// other
		float getLength() const;
		Vec2 getNormalized() const;
		void normalize();
		float dot(const Vec2& b) const;
};


// other operators
extern Vec2 operator +(float f, const Vec2& v2);
extern Vec2 operator -(float f, const Vec2& v2);
extern Vec2 operator *(float f, const Vec2& v2);
extern Vec2 operator /(float f, const Vec2& v2);
extern ostream& operator<<(ostream& s, const Vec2& v);


} // end namespace


#include "Vec2.inl.h"


#endif
