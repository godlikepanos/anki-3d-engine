#ifndef _VEC4_H_
#define _VEC4_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// 4D vector
class Vec4
{
	public:
		// data members
		float x, y, z, w;
		// accessors
		float& operator [](uint i);
		float  operator [](uint i) const;
		// constructors & distructors
		explicit Vec4();
		explicit Vec4(float f);
		explicit Vec4(float x, float y, float z, float w);
		explicit Vec4(const Vec2& v2, float z, float w);
		explicit Vec4(const Vec3& v3, float w);
		         Vec4(const Vec4& b);
		explicit Vec4(const Quat& q);
		// ops with same
		Vec4  operator + (const Vec4& b) const;
		Vec4& operator +=(const Vec4& b);
		Vec4  operator - (const Vec4& b) const;
		Vec4& operator -=(const Vec4& b);
		Vec4  operator * (const Vec4& b) const;
		Vec4& operator *=(const Vec4& b);
		Vec4  operator / (const Vec4& b) const;
		Vec4& operator /=(const Vec4& b);
		Vec4  operator - () const;
		// ops with float
		Vec4  operator + (float f) const;
		Vec4& operator +=(float f);
		Vec4  operator - (float f) const;
		Vec4& operator -=(float f);
		Vec4  operator * (float f) const;
		Vec4& operator *=(float f);
		Vec4  operator / (float f) const;
		Vec4& operator /=(float f);
		// ops with other
		Vec4  operator * (const Mat4& m4) const;
		// comparision
		bool operator ==(const Vec4& b) const;
		bool operator !=(const Vec4& b) const;
		// other
		float getLength() const;
		Vec4  getNormalized() const;
		void  normalize();
		float dot(const Vec4& b) const;
};


// other operators
extern Vec4 operator +(float f, const Vec4& v4);
extern Vec4 operator -(float f, const Vec4& v4);
extern Vec4 operator *(float f, const Vec4& v4);
extern Vec4 operator /(float f, const Vec4& v4);
extern ostream& operator<<(ostream& s, const Vec4& v);


} // end namespace


#include "Vec4.inl.h"


#endif
