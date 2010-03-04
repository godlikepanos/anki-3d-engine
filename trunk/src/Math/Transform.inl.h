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


} // end namespace
