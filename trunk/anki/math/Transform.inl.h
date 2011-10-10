#include "anki/math/MathCommonSrc.h"


//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Transform::Transform()
{}


// Copy
inline Transform::Transform(const Transform& b)
:	origin(b.origin),
	rotation(b.rotation),
	scale(b.scale)
{}


// Mat4
inline Transform::Transform(const Mat4& m4)
{
	rotation = m4.getRotationPart();
	origin = m4.getTranslationPart();
	scale = 1.0;
}


// Vec3, Quat, float
inline Transform::Transform(const Vec3& origin, const Mat3& rotation_,
	const float scale_)
:	origin(origin),
	rotation(rotation_),
	scale(scale_)
{}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

inline const Vec3& Transform::getOrigin() const
{
	return origin;
}


inline Vec3& Transform::getOrigin()
{
	return origin;
}


inline void Transform::setOrigin(const Vec3 o)
{
	origin = o;
}


inline const Mat3& Transform::getRotation() const
{
	return rotation;
}


inline Mat3& Transform::getRotation()
{
	return rotation;
}


inline void Transform::setRotation(const Mat3& r)
{
	rotation = r;
}


inline float Transform::getScale() const
{
	return scale;
}


inline float& Transform::getScale()
{
	return scale;
}


inline void Transform::setScale(const float s)
{
	scale = s;
}


//==============================================================================
// Operators with same                                                         =
//==============================================================================

// =
inline Transform& Transform::operator=(const Transform& b)
{
	origin = b.origin;
	rotation = b.rotation;
	scale = b.scale;
	return *this;
}


//==============================================================================
// Other                                                                       =
//==============================================================================

// setIdentity
inline void Transform::setIdentity()
{
	(*this) = getIdentity();
}


// getIdentity
inline const Transform& Transform::getIdentity()
{
	static Transform ident(Vec3(0.0), Mat3::getIdentity(), 1.0);
	return ident;
}


// combineTransformations
inline Transform Transform::combineTransformations(const Transform& a,
	const Transform& b)
{
	Transform out;

	Math::combineTransformations(
		a.origin, a.rotation, a.scale,
		b.origin, b.rotation, b.scale,
		out.origin, out.rotation, out.scale);

	return out;
}
