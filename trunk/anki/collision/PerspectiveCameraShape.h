#ifndef ANKI_PERSPECTIVE_CAMERA_SHAPE_H
#define ANKI_PERSPECTIVE_CAMERA_SHAPE_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"


namespace anki {


/// @addtogroup Collision
/// @{

/// Collision shape for the projection cameras
class PerspectiveCameraShape: public CollisionShape
{
	public:
		/// Default constructor
		PerspectiveCameraShape()
		:	CollisionShape(CST_PERSPECTIVE_CAMERA_FRUSTRUM)
		{}

		PerspectiveCameraShape(float fovX, float fovY, float zNear,
			float zFar, const Transform& trf)
		:	CollisionShape(CST_PERSPECTIVE_CAMERA_FRUSTRUM)
		{
			setAll(fovX, fovY, zNear, zFar, trf);
		}

		/// @name Accessors
		/// @{
		const Vec3& getEye() const
		{
			return eye;
		}

		const boost::array<Vec3, 4>& getDirections() const
		{
			return dirs;
		}
		/// @{

		/// Implements CollisionShape::accept
		void accept(MutableVisitor& v)
		{
			v.visit(*this);
		}
		/// Implements CollisionShape::accept
		void accept(ConstVisitor& v) const
		{
			v.visit(*this);
		}

		/// Implements CollisionShape::testPlane
		float testPlane(const Plane& p) const;

		/// Implements CollisionShape::transform
		void transform(const Transform& trf)
		{
			*this = getTransformed(trf);
		}

		PerspectiveCameraShape getTransformed(const Transform& trf) const;

		/// Set all
		void setAll(float fovX, float fovY, float zNear,
			float zFar, const Transform& trf);

	private:
		Vec3 eye; ///< The eye point
		boost::array<Vec3, 4> dirs; ///< Directions
};
///@}


} // end namespace


#endif
