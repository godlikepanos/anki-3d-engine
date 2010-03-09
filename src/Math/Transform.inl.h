#include "MathDfltHeader.h"


#define ME (*this)


namespace M {


// constructor []
inline Transform::Transform()
{}

// constructor [Transform]
inline Transform::Transform( const Transform& b ):
	rotation(b.rotation), translation(b.translation)
{}

// constructor [Mat4]
inline Transform::Transform( const Mat4& m4 )
{
	rotation = Quat( m4.getRotationPart() );
	translation = m4.getTranslationPart();
}

// constructor [Vec3, Quat, float]
inline Transform::Transform( const Vec3& origin, const Quat& rotation_, float scale_ ):
	rotation(rotation_), translation(origin), scale(scale_)
{}

// getIdentity
inline const Transform& Transform::getIdentity()
{
	static Transform ident( Vec3(0.0), Quat::getIdentity(), 1.0 );
	return ident;
}


} // end namespace
