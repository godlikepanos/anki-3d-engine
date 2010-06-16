#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "Common.h"
#include "MathForwardDecls.h"


namespace M {


/// For transformations
class Transform
{
	PROPERTY_RW(Mat3, rotation, setRotation, getRotation) ///< @ref PROPERTY_RW : The rotation
	PROPERTY_RW(Vec3, origin, setOrigin, getOrigin) ///< @ref PROPERTY_RW : The translation
	PROPERTY_RW(float, scale, setScale, getScale) ///< @ref PROPERTY_RW : The uniform scaling

	public:
		// constructors
		explicit Transform();
		         Transform(const Transform& b);
		explicit Transform(const Mat4& m4);
		explicit Transform(const Vec3& origin, const Mat3& rotation_, float scale_);
		// funcs
		void setIdentity();
		static const Transform& getIdentity();
		static Transform combineTransformations(const Transform& a, const Transform& b); ///< @see M::combineTransformations
};


} // end namespace


#include "Transform.inl.h"


#endif
