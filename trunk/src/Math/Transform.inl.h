#include "MathCommon.inl.h"


#define ME (*this)


namespace M {


// constructor []
inline Transform::Transform()
{}

// constructor [Transform]
inline Transform::Transform(const Transform& b):
	origin(b.origin),
	rotation(b.rotation),
	scale(b.scale)
{}

// constructor [Mat4]
inline Transform::Transform(const Mat4& m4)
{
	rotation = m4.getRotationPart();
	origin = m4.getTranslationPart();
	scale = 1.0;
}

// constructor [Vec3, Quat, float]
inline Transform::Transform(const Vec3& origin, const Mat3& rotation_, float scale_):
	origin(origin),
	rotation(rotation_),
	scale(scale_)
{}

// setIdentity
inline void Transform::setIdentity()
{
	ME = getIdentity();
}

// getIdentity
inline const Transform& Transform::getIdentity()
{
	static Transform ident(Vec3(0.0), Mat3::getIdentity(), 1.0);
	return ident;
}

// combineTransformations
inline Transform Transform::combineTransformations(const Transform& a, const Transform& b)
{
	Transform out;
	M::combineTransformations(a.origin, a.rotation, a.scale, b.origin, b.rotation, b.scale, out.origin, out.rotation, out.scale);
	return out;
}


} // end namespace
