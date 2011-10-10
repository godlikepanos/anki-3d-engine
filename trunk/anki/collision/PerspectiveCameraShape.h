#ifndef PROJECTION_CAMERA_SHAPE_H
#define PROJECTION_CAMERA_SHAPE_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"


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

		/// @copydoc CollisionShape::testPlane
		float testPlane(const Plane& p) const;

		PerspectiveCameraShape getTransformed(const Transform& trf) const;

		/// Set all
		void setAll(float fovX, float fovY, float zNear,
			float zFar, const Transform& trf);


	private:
		Vec3 eye; ///< The eye point
		boost::array<Vec3, 4> dirs; ///< Directions
};
///@}


#endif
