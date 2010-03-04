#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


class Transform
{
	PROPERTY_RW( Quat, rotation, setRotation, getRotation );
	PROPERTY_RW( Vec3, translation, setOrigin, getOrigin );

	public:
		explicit Transform();
		         Transform( const Transform& b );
		explicit Transform( const Mat4& m4 );
};


} // end namespace


#include "Transform.inl.h"


#endif
