#ifndef ANKI_MATH_QUAT_H
#define ANKI_MATH_QUAT_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Quaternion. Used in rotations
class Quat
{
public:
	/// @name Constructors
	/// @{
	explicit Quat();
	explicit Quat(const F32 f);
	explicit Quat(const F32 x, const F32 y, const F32 z,
		const F32 w);
	explicit Quat(const Vec2& v2, const F32 z, const F32 w);
	explicit Quat(const Vec3& v3, const F32 w);
	explicit Quat(const Vec4& v4);
			 Quat(const Quat& b);
	explicit Quat(const Mat3& m3);
	explicit Quat(const Euler& eu);
	explicit Quat(const Axisang& axisang);
	/// @}

	/// @name Accessors
	/// @{
	F32 x() const;
	F32& x();
	F32 y() const;
	F32& y();
	F32 z() const;
	F32& z();
	F32 w() const;
	F32& w();
	/// @}

	/// Operators with same type
	/// @{
	Quat& operator=(const Quat& b);
	Quat operator*(const Quat& b) const; ///< 16 muls, 12 adds
	Quat& operator*=(const Quat& b);
	Bool operator==(const Quat& b) const;
	Bool operator!=(const Quat& b) const;
	/// @}

	/// @name Other
	/// @{

	/// Calculates the rotation from Vec3 v0 to v1
	void setFrom2Vec3(const Vec3& v0, const Vec3& v1);
	F32 getLength() const;
	Quat getInverted() const;
	void invert();
	void conjugate();
	Quat getConjugated() const;
	void normalize();
	Quat getNormalized() const;
	F32 dot(const Quat& b) const;
	/// Returns slerp(this, q1, t)
	Quat slerp(const Quat& q1, const F32 t) const;
	Quat getRotated(const Quat& b) const; ///< The same as Quat * Quat
	void rotate(const Quat& b); ///< @see getRotated
	void setIdentity();
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
		F32 x, y, z, w;
	} vec;
	/// @}
};
/// @}

} // end namespace

#include "anki/math/Quat.inl.h"

#endif
