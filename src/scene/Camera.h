#ifndef CAMERA_H
#define CAMERA_H

#include <boost/array.hpp>
#include <deque>
#include "util/Vec.h"
#include "cln/Collision.h"
#include "SceneNode.h"
#include "VisibilityInfo.h"


class SpotLight;
class PointLight;


/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class Camera: public SceneNode, public VisibilityInfo
{
	public:
		/// @note Don't EVER change the order
		enum CameraType
		{
			CT_PERSPECTIVE,
			CT_ORTHOGRAPHIC,
			CT_NUM
		};

		enum FrustrumPlanes
		{
			FP_LEFT = 0,
			FP_RIGHT,
			FP_NEAR,
			FP_TOP,
			FP_BOTTOM,
			FP_FAR,
			FP_NUM
		};

		Camera(CameraType type, Scene& scene, ulong flags, SceneNode* parent);

		virtual ~Camera();

		/// @name Accessors
		/// @{
		CameraType getCameraType() const
		{
			return type;
		}

		float getZNear() const
		{
			return zNear;
		}
		void setZNear(float znear);

		float getZFar() const
		{
			return zFar;
		}
		void setZFar(float zfar);

		const Mat4& getProjectionMatrix() const
		{
			return projectionMat;
		}

		const Mat4& getViewMatrix() const
		{
			return viewMat;
		}

		/// See the declaration of invProjectionMat for info
		const Mat4& getInvProjectionMatrix() const
		{
			return invProjectionMat;
		}

		const Plane& getWSpaceFrustumPlane(FrustrumPlanes id) const;
		/// @}

		void lookAtPoint(const Vec3& point);

		/// This does:
		/// - Update view matrix
		/// - Update frustum planes
		void moveUpdate();

		/// @name Frustum checks
		/// @{

		/// Check if the given camera is inside the frustum clipping planes.
		/// This is used mainly to test if the projected lights are visible
		bool insideFrustum(const CollisionShape& vol) const;

		/// Check if another camera is inside our view (used for projected
		/// lights)
		bool insideFrustum(const Camera& cam) const;
		/// @}

	protected:
		float zNear, zFar;

		/// @name The frustum planes in local and world space
		/// @{
		boost::array<Plane, FP_NUM> lspaceFrustumPlanes;
		boost::array<Plane, FP_NUM> wspaceFrustumPlanes;
		/// @}

		/// @name Matrices
		/// @{
		Mat4 projectionMat;
		Mat4 viewMat;

		/// Used in deferred shading for the calculation of view vector (see
		/// CalcViewVector). The reason we store this matrix here is that we
		/// don't want it to be re-calculated all the time but only when the
		/// projection params (fovX, fovY, zNear, zFar) change. Fortunately
		/// the projection params change rarely. Note that the Camera as we all
		/// know re-calculates the matrices only when the parameters change!!
		Mat4 invProjectionMat;
		/// @}

		/// Calculate projectionMat and invProjectionMat
		virtual void calcProjectionMatrix() = 0;
		virtual void calcLSpaceFrustumPlanes() = 0;
		void updateViewMatrix();
		void updateWSpaceFrustumPlanes();

		/// Get the edge points of the camera
		virtual void getExtremePoints(Vec3* pointsArr,
			uint& pointsNum) const = 0;

	private:
		CameraType type;
};
/// @}



inline Camera::Camera(CameraType t, Scene& scene, ulong flags,
	SceneNode* parent)
:	SceneNode(SNT_CAMERA, scene, flags, parent),
 	type(t)
{
	name = "Camera:" + name;
}


inline const Plane& Camera::getWSpaceFrustumPlane(FrustrumPlanes id) const
{
	return wspaceFrustumPlanes[id];
}


inline void Camera::setZNear(float znear_)
{
	zNear = znear_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


inline void Camera::setZFar(float zfar_)
{
	zFar = zfar_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


#endif
