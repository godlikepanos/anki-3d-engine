#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


class Transform
{
	PROPERTY_RW( Quat, rotation, setRotation, getRotation );
	PROPERTY_RW( Vec3, translation, setOrigin, getOrigin );
	PROPERTY_RW( float, scale, setScale, getScale );

	public:
		explicit Transform();
		         Transform( const Transform& b );
		explicit Transform( const Mat4& m4 );
		explicit Transform( const Vec3& origin, const Quat& rotation_, float scale_ );

		static const Transform& getIdentity();
};


} // end namespace


#include "Transform.inl.h"


#endif
