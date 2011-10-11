#ifndef ANKI_MATH_QUAT_H
#define ANKI_MATH_QUAT_H

#include "anki/math/MathCommonIncludes.h"


/// @addtogroup Math
/// @{

/// Quaternion. Used in rotations
class Quat
{
	public:
		/// @name Constructors
		/// @{
		explicit Quat();
		explicit Quat(const float f);
		explicit Quat(const float x, const float y, const float z,
			const float w);
		explicit Quat(const Vec2& v2, const float z, const float w);
		explicit Quat(const Vec3& v3, const float w);
		explicit Quat(const Vec4& v4);
		         Quat(const Quat& b);
		explicit Quat(const Mat3& m3);
		explicit Quat(const Euler& eu);
		explicit Quat(const Axisang& axisang);
		/// @}

		/// @name Accessors
		/// @{
		float x() const;
		float& x();
		float y() const;
		float& y();
		float z() const;
		float& z();
		float w() const;
		float& w();
		/// @}

		/// Operators with same type
		/// @{
		Quat& operator=(const Quat& b);
		Quat operator*(const Quat& b) const; ///< 16 muls, 12 adds
		Quat& operator*=(const Quat& b);
		bool operator==(const Quat& b) const;
		bool operator!=(const Quat& b) const;
		/// @}

		/// @name Other
		/// @{

		/// Calculates the rotation from Vec3 v0 to v1
		void  setFrom2Vec3(const Vec3& v0, const Vec3& v1);
		float getLength() const;
		Quat  getInverted() const;
		void  invert();
		void  conjugate();
		Quat  getConjugated() const;
		void  normalize();
		Quat  getNormalized() const;
		float dot(const Quat& b) const;
		/// Returns slerp(this, q1, t)
		Quat  slerp(const Quat& q1, const float t) const;
		Quat  getRotated(const Quat& b) const; ///< The same as Quat * Quat
		void  rotate(const Quat& b); ///< @see getRotated
		void  setIdentity();
		static const Quat& getIdentity();
		/// @}

		/// @name Friends
		/// @{
		friend std::ostream& operator<<(std::ostream& s, const Quat& q);
		/// @}

	private:
		/// @name Data
		/// @{
		struct
		{
			float x, y, z, w;
		} vec;
		/// @}
};
/// @}


#include "anki/math/Quat.inl.h"


#endif
