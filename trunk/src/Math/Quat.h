#ifndef QUAT_H
#define QUAT_H

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// Used in rotations
class Quat
{
	public:
		/// @name Data
		/// @{
		float x, y, z, w;
		/// @}

		/// @name Constructors & destructors
		/// @{
		explicit Quat();
		explicit Quat(float f);
		explicit Quat(float x, float y, float z, float w);
		explicit Quat(const Vec2& v2, float z, float w);
		explicit Quat(const Vec3& v3, float w);
		explicit Quat(const Vec4& v4);
		         Quat(const Quat& b);
		explicit Quat(const Mat3& m3);
		explicit Quat(const Euler& eu);
		explicit Quat(const Axisang& axisang);
		/// @}

		/// Operatorswith same
		/// @{
		Quat operator *(const Quat& b) const; ///< 16 muls, 12 adds
		Quat& operator *=(const Quat& b);
		bool operator ==(const Quat& b) const;
		bool operator !=(const Quat& b) const;
		/// @}

		/// @name Other
		/// @{
		void  setFrom2Vec3(const Vec3& v0, const Vec3& v1); ///< calculates a quat from v0 to v1
		float getLength() const;
		Quat  getInverted() const;
		void  invert();
		void  conjugate();
		Quat  getConjugated() const;
		void  normalize();
		Quat  getNormalized() const;
		float dot(const Quat& b) const;
		Quat  slerp(const Quat& q1, float t) const; ///< returns slerp(this, q1, t)
		Quat  getRotated(const Quat& b) const; ///< The same as Quat * Quat
		void  rotate(const Quat& b); ///< @see getRotated
		void  setIdentity();
		static const Quat& getIdentity();
		/// @}
};


/// @name Other operators
/// @{
extern ostream& operator<<(ostream& s, const Quat& q);
/// @}


} // end namespace


#include "Quat.inl.h"


#endif
