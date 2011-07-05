#ifndef PERSPECTIVE_CAMERA_H
#define PERSPECTIVE_CAMERA_H

#include "Camera.h"


/// @todo
class PerspectiveCamera: public Camera
{
	public:
		PerspectiveCamera(bool compoundFlag, SceneNode* parent);

		/// @name Accessors
		/// @{
		float getFovX() const {return fovX;}
		void setFovX(float fovx);

		float getFovY() const {return fovY;}
		void setFovY(float fovy);
		/// @}

		void setAll(float fovx, float fovy, float znear, float zfar);

	private:
		/// @name Data
		/// @{

		/// fovX is the angle in the y axis (imagine the cam positioned in
		/// the default OGL pos) Note that fovX > fovY (most of the time) and
		/// aspectRatio = fovX/fovY
		float fovX;
		float fovY; /// @see fovX
		/// @}

		/// Implements Camera::calcLSpaceFrustumPlanes
		void calcLSpaceFrustumPlanes();

		/// Implements Camera::calcProjectionMatrix
		void calcProjectionMatrix();

		/// Implements Camera::getExtremePoints
		void getExtremePoints(Vec3* pointsArr, uint& pointsNum) const;
};


inline PerspectiveCamera::PerspectiveCamera(bool compoundFlag,
	SceneNode* parent)
:	Camera(CT_PERSPECTIVE, compoundFlag, parent)
{
	name = "PerspectiveCamera:" + name;
}


inline void PerspectiveCamera::setFovX(float fovx_)
{
	fovX = fovx_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


inline void PerspectiveCamera::setFovY(float fovy_)
{
	fovY = fovy_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


#endif
