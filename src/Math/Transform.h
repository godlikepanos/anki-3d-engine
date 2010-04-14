#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/**
 * @brief For transformations
 */
class Transform
{
	PROPERTY_RW( Mat3, rotation, setRotation, getRotation ) ///< Property
	PROPERTY_RW( Vec3, origin, setOrigin, getOrigin ) ///< Property
	PROPERTY_RW( float, scale, setScale, getScale ) ///< Property

	public:
		// constructors
		explicit Transform();
		         Transform( const Transform& b );
		explicit Transform( const Mat4& m4 );
		explicit Transform( const Vec3& origin, const Mat3& rotation_, float scale_ );
		// funcs
		void setIdentity();
		static const Transform& getIdentity();
};


} // end namespace


#include "Transform.inl.h"


#endif
