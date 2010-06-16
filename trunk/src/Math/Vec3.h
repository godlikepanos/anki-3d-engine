#ifndef _VEC3_H_
#define _VEC3_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// 3D vector. One of the most used classes
class Vec3
{
	public:
		// data members
		float x, y, z;
		// accessors
		float& operator [](uint i);
		float  operator [](uint i) const;
		// constructors & distructors
		explicit Vec3();
		explicit Vec3(float f);
		explicit Vec3(float x, float y, float z);
		explicit Vec3(const Vec2& v2, float z);
		         Vec3(const Vec3& b);
		explicit Vec3(const Vec4& v4);
		explicit Vec3(const Quat& q);
		// ops with same type
		Vec3  operator + (const Vec3& b) const;
		Vec3& operator +=(const Vec3& b);
		Vec3  operator - (const Vec3& b) const;
		Vec3& operator -=(const Vec3& b);
		Vec3  operator * (const Vec3& b) const;
		Vec3& operator *=(const Vec3& b);
		Vec3  operator / (const Vec3& b) const;
		Vec3& operator /=(const Vec3& b);
		Vec3  operator - () const; // return the negative
		// ops with float
		Vec3  operator + (float f) const;
		Vec3& operator +=(float f);
		Vec3  operator - (float f) const;
		Vec3& operator -=(float f);
		Vec3  operator * (float f) const;
		Vec3& operator *=(float f);
		Vec3  operator / (float f) const;
		Vec3& operator /=(float f);
		// ops with other types
		Vec3  operator * (const Mat3& m3) const;
		// comparision
		bool operator ==(const Vec3& b) const;
		bool operator !=(const Vec3& b) const;
		// other
		float dot(const Vec3& b) const;
		Vec3  cross(const Vec3& b) const;
		float getLength() const;
		float getLengthSquared() const;
		float getDistanceSquared(const Vec3& b) const;
		void  normalize();
		Vec3  getNormalized() const;
		Vec3  getProjection(const Vec3& toThis) const;
		Vec3  getRotated(const Quat& q) const; ///< Returns q * this * q.Conjucated() aka returns a rotated this. 18 muls, 12 adds
		void  rotate(const Quat& q);
		Vec3  lerp(const Vec3& v1, float t) const; ///< return lerp(this, v1, t)
		void  print() const;
		// transformations. The faster way is by far the mat4 * vec3 or the Transformed(Vec3, Mat3)
		Vec3  getTransformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void  transform(const Vec3& translate, const Mat3& rotate, float scale);
		Vec3  getTransformed(const Vec3& translate, const Mat3& rotate) const;
		void  transform(const Vec3& translate, const Mat3& rotate);
		Vec3  getTransformed(const Vec3& translate, const Quat& rotate, float scale) const;
		void  transform(const Vec3& translate, const Quat& rotate, float scale);
		Vec3  getTransformed(const Vec3& translate, const Quat& rotate) const;
		void  transform(const Vec3& translate, const Quat& rotate);
		Vec3  getTransformed(const Mat4& transform) const;  ///< 9 muls, 9 adds
		void  transform(const Mat4& transform);
		Vec3  getTransformed(const Transform& transform) const;
		void  transform(const Transform& transform);
};


// other operators
extern Vec3 operator +(float f, const Vec3& v);
extern Vec3 operator -(float f, const Vec3& v);
extern Vec3 operator *(float f, const Vec3& v);
extern Vec3 operator /(float f, const Vec3& v);


} // end namespace


#include "Vec3.inl.h"


#endif
