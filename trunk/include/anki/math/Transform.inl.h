#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline Transform::Transform()
{}

// Copy
inline Transform::Transform(const Transform& b)
	: origin(b.origin), rotation(b.rotation), scale(b.scale)
{}

// Mat4
inline Transform::Transform(const Mat4& m4)
{
	rotation = m4.getRotationPart();
	origin = m4.getTranslationPart();
	scale = 1.0;
}

// Vec3, Quat, F32
inline Transform::Transform(const Vec3& origin, const Mat3& rotation_,
	const F32 scale_)
	: origin(origin), rotation(rotation_), scale(scale_)
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

inline F32 Transform::getScale() const
{
	return scale;
}

inline F32& Transform::getScale()
{
	return scale;
}

inline void Transform::setScale(const F32 s)
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

// ==
inline Bool Transform::operator==(const Transform& b) const
{
	return origin == b.origin && rotation == b.rotation && scale == b.scale;
}

// !=
inline Bool Transform::operator!=(const Transform& b) const
{
	return !operator==(b);
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
inline void Transform::combineTransformations(
	const Vec3& t0, const Mat3& r0, const F32 s0,
	const Vec3& t1, const Mat3& r1, const F32 s1,
	Vec3& tf, Mat3& rf, F32& sf)
{
	tf = t1.getTransformed(t0, r0, s0);
	rf = r0 * r1;
	sf = s0 * s1;
}

// combineTransformations
inline void Transform::combineTransformations(
	const Vec3& t0, const Mat3& r0,
	const Vec3& t1, const Mat3& r1,
	Vec3& tf, Mat3& rf)
{
	tf = t1.getTransformed(t0, r0);
	rf = r0 * r1;
}

// combineTransformations
inline Transform Transform::combineTransformations(const Transform& a,
	const Transform& b)
{
	Transform out;

	out.origin = b.origin.getTransformed(a.origin, a.rotation, a.scale);
	out.rotation = a.rotation * b.rotation;
	out.scale = a.scale * b.scale;

	return out;
}

// getInverse
inline Transform Transform::getInverse() const
{
	Transform o;
	o.rotation = rotation.getTransposed(); // Rotation
	o.scale = 1.0 / scale; // Apply scale
	o.origin = -((o.rotation * o.scale) * origin); // Translation
	return o;
}

// invert
inline void Transform::invert()
{
	*this = getInverse();
}

// transform
inline void Transform::transform(const Transform& b)
{
	origin = b.origin.getTransformed(origin, rotation, scale);
	rotation = rotation * b.rotation;
	scale *= b.scale;
}

//==============================================================================
// Friends                                                                     =
//==============================================================================

// Print
inline std::ostream& operator<<(std::ostream& s, const Transform& a)
{
	s << "o: " << a.origin << "\n" 
		<< "s: " << a.scale << "\n" 
		<< "r:\n" << a.rotation;
	return s;
}

} // end namespace
