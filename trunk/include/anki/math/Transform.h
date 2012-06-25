#ifndef ANKI_MATH_TRANSFORM_H
#define ANKI_MATH_TRANSFORM_H

#include "anki/math/Vec3.h"
#include "anki/math/Mat3.h"
#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Transformation
class Transform
{
public:
	/// @name Constructors
	/// @{
	explicit Transform();
	Transform(const Transform& b);
	explicit Transform(const Mat4& m4);
	explicit Transform(const Vec3& origin, const Mat3& rotation,
		const float scale);
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getOrigin() const;
	Vec3& getOrigin();
	void setOrigin(const Vec3 o);

	const Mat3& getRotation() const;
	Mat3& getRotation();
	void setRotation(const Mat3& r);

	float getScale() const;
	float& getScale();
	void setScale(const float s);
	/// @}

	/// @name Operators with same type
	/// @{
	Transform& operator=(const Transform& b);
	bool operator==(const Transform& b) const;
	bool operator!=(const Transform& b) const;
	/// @}

	/// @name Other
	/// @{
	void setIdentity();
	static const Transform& getIdentity();
	static Transform combineTransformations(const Transform& a,
		const Transform& b); ///< @copybrief Math::combineTransformations

	/// Get the inverse transformation. Its faster that inverting a Mat4
	Transform getInverse() const;
	void invert();
	void transform(const Transform& b);
	/// @}

	/// @name Friends
	/// @{
	friend std::ostream& operator<<(std::ostream& s, const Transform& a);
	/// @}

private:
	/// @name Data
	/// @{
	Vec3 origin; ///< The rotation
	Mat3 rotation; ///< The translation
	float scale; ///< The uniform scaling
	/// @}
};
/// @}

} // end namespace

#include "anki/math/Transform.inl.h"

#endif
