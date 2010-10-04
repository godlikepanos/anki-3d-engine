#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "MathCommon.h"


namespace M {


/// For transformations
class Transform
{
	public:
		/// @name Data
		/// @{
		Vec3 origin; ///< The rotation
		Mat3 rotation; ///< The translation
		float scale; ///< The uniform scaling
		/// @}

		/// @name Constructors
		/// @{
		explicit Transform();
		         Transform(const Transform& b);
		explicit Transform(const Mat4& m4);
		explicit Transform(const Vec3& origin, const Mat3& rotation_, float scale_);
		/// @}

		/// @name Other
		/// @{
		void setIdentity();
		static const Transform& getIdentity();
		static Transform combineTransformations(const Transform& a, const Transform& b); ///< @see M::combineTransformations
		/// @}
};


} // end namespace


#include "Transform.inl.h"


#endif
