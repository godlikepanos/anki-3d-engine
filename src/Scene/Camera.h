#ifndef CAMERA_H
#define CAMERA_H

#include <boost/array.hpp>
#include "Collision.h"
#include "SceneNode.h"


/// Camera SceneNode
class Camera: public SceneNode
{
	public:
		enum FrustrumPlanes
		{
			FP_LEFT = 0,
			FP_RIGHT,
			FP_NEAR,
			FP_TOP,
			FP_BOTTOM,
			FP_FAR
		};

		Camera(bool compoundFlag, SceneNode* parent): SceneNode(SNT_CAMERA, compoundFlag, parent) {}
		~Camera() {}

		/// @name Accessors
		//// @{
		void setFovX (float fovx_)  {fovX=fovx_; calcProjectionMatrix(); calcLSpaceFrustumPlanes();}
		void setFovY (float fovy_)  {fovY=fovy_; calcProjectionMatrix(); calcLSpaceFrustumPlanes();}
		void setZNear(float znear_) {zNear=znear_; calcProjectionMatrix(); calcLSpaceFrustumPlanes();}
		void setZFar (float zfar_)  {zFar=zfar_; calcProjectionMatrix(); calcLSpaceFrustumPlanes();}
		void setAll(float fovx_, float fovy_, float znear_, float zfar_);
		float getFovX() const {return fovX;}
		float getFovY() const {return fovY;}
		float getZNear() const {return zNear;}
		float getZFar() const {return zFar;}
		const Mat4& getProjectionMatrix() const {return projectionMat;}
		const Mat4& getViewMatrix() const {return viewMat;}
		const Mat4& getInvProjectionMatrix() const {return invProjectionMat;} ///< See the declaration of invProjectionMat for info
		//// @}

		void lookAtPoint(const Vec3& point);

		/// This does:
		/// - Update view matrix
		/// - Update frustum planes
		void updateTrf();

		/// Do nothing
		void init(const char*) {}

		/// @name Frustum checks
		/// @{

		/// Check if the given camera is inside the frustum clipping planes. This is used mainly to test if the projected
		/// lights are visible
		bool insideFrustum(const CollisionShape& vol) const;

		/// Check if another camera is inside our view (used for projected lights)
		bool insideFrustum(const Camera& cam) const;
		/// @}

	private:
		/// @name Angles
		/// fovX is the angle in the y axis (imagine the cam positioned in the default OGL pos) Note that fovX > fovY
		/// (most of the time) and aspectRatio = fovX/fovY
		/// @{
		float fovX, fovY;
		/// @}

		float zNear, zFar;

		/// @name The frustum planes in local and world space
		/// @{
		boost::array<Plane, 6> lspaceFrustumPlanes;
		boost::array<Plane, 6> wspaceFrustumPlanes;
		/// @}

		/// @name Matrices
		/// @{
		Mat4 projectionMat;
		Mat4 viewMat;

		/// Used in deferred shading for the calculation of view vector (see CalcViewVector). The reason we store this
		/// matrix here is that we dont want it to be re-calculated all the time but only when the projection params
		/// (fovX, fovY, zNear, zFar) change. Fortunately the projection params change rarely. Note that the Camera as we
		/// all know re-calculates the matrices only when the parameters change!!
		Mat4 invProjectionMat;
		/// @}

		void calcProjectionMatrix();
		void updateViewMatrix();
		void calcLSpaceFrustumPlanes();
		void updateWSpaceFrustumPlanes();
};


#endif
