#ifndef M_TRANSFORM_H
#define M_TRANSFORM_H

#include "Common.h"


namespace M {


/// For transformations
class Transform
{
	public:
		/// @name Constructors
		/// @{
		explicit Transform();
		         Transform(const Transform& b);
		explicit Transform(const Mat4& m4);
		explicit Transform(const Vec3& origin, const Mat3& rotation_,
			float scale_);
		/// @}

		/// @name Accessors
		/// @{
		const Vec3& getOrigin() const;
		Vec3& getOrigin();
		void setOrigin(const Vec3 o);
		
		const Mat3& getRotation() const;
		Mat3& getRotation();
		void setRotation(const Mat3& r);
		
		float getScale() const;
		float& getScale();
		void setScale(float s);
		/// @}

		/// @name Operators with same
		/// @{
		Transform& operator=(const Transform& b);
		/// @}

		/// @name Other
		/// @{
		void setIdentity();
		static const Transform& getIdentity();
		static Transform combineTransformations(const Transform& a,
			const Transform& b); ///< @see M::combineTransformations
		/// @}
		
	private:
		/// @name Data
		/// @{
		Vec3 origin; ///< The rotation
		Mat3 rotation; ///< The translation
		float scale; ///< The uniform scaling
		/// @}
};


} // end namespace


#include "Transform.inl.h"


#endif
