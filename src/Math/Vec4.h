#ifndef VEC4_H
#define VEC4_H

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// 4D vector
class Vec4
{
	public:
		/// @name Data members
		/// @{
		float x, y, z, w;
		/// @}

		/// @name Accessors
		/// @{
		float& operator [](uint i);
		float  operator [](uint i) const;
		/// @}

		/// @name Constructors & distructors
		/// @{
		explicit Vec4();
		explicit Vec4(float f);
		explicit Vec4(float x, float y, float z, float w);
		explicit Vec4(const Vec2& v2, float z, float w);
		explicit Vec4(const Vec3& v3, float w);
		         Vec4(const Vec4& b);
		explicit Vec4(const Quat& q);
		/// @}

		/// @name Operators with same
		/// @{
		Vec4 operator +(const Vec4& b) const;
		Vec4& operator +=(const Vec4& b);
		Vec4 operator -(const Vec4& b) const;
		Vec4& operator -=(const Vec4& b);
		Vec4 operator *(const Vec4& b) const;
		Vec4& operator *=(const Vec4& b);
		Vec4 operator /(const Vec4& b) const;
		Vec4& operator /=(const Vec4& b);
		Vec4 operator -() const;
		bool operator ==(const Vec4& b) const;
		bool operator !=(const Vec4& b) const;
		/// @}

		/// @name Operators with float
		/// @{
		Vec4 operator +(float f) const;
		Vec4& operator +=(float f);
		Vec4 operator -(float f) const;
		Vec4& operator -=(float f);
		Vec4 operator *(float f) const;
		Vec4& operator *=(float f);
		Vec4 operator /(float f) const;
		Vec4& operator /=(float f);
		/// @}

		/// @name Operators with other
		/// @{
		Vec4  operator * (const Mat4& m4) const;
		/// @}

		/// @name Other
		/// @{
		float getLength() const;
		Vec4 getNormalized() const;
		void normalize();
		float dot(const Vec4& b) const;
		/// @}
};


/// @name Other operators
/// @{
extern Vec4 operator +(float f, const Vec4& v4);
extern Vec4 operator -(float f, const Vec4& v4);
extern Vec4 operator *(float f, const Vec4& v4);
extern Vec4 operator /(float f, const Vec4& v4);
extern ostream& operator<<(ostream& s, const Vec4& v);
/// @}


} // end namespace


#include "Vec4.inl.h"


#endif
