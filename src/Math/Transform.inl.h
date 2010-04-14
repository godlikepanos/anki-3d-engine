#include "MathDfltHeader.h"


#define ME (*this)


namespace M {


// constructor []
inline Transform::Transform()
{}

// constructor [Transform]
inline Transform::Transform( const Transform& b ):
	rotation(b.rotation), origin(b.origin)
{}

// constructor [Mat4]
inline Transform::Transform( const Mat4& m4 )
{
	rotation = m4.getRotationPart();
	origin = m4.getTranslationPart();
	scale = 1.0;
}

// constructor [Vec3, Quat, float]
inline Transform::Transform( const Vec3& origin, const Mat3& rotation_, float scale_ ):
	rotation(rotation_), origin(origin), scale(scale_)
{}

// setIdentity
inline void Transform::setIdentity()
{
	ME = getIdentity();
}

// getIdentity
inline const Transform& Transform::getIdentity()
{
	static Transform ident( Vec3(0.0), Mat3::getIdentity(), 1.0 );
	return ident;
}


} // end namespace
