#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "common.h"
#include "primitives.h"
#include "collision.h"

class camera_runtime_class_t: public runtime_class_t {}; // for ambiguity reasons

class camera_t: public object_t, public camera_runtime_class_t
{
	protected:
		enum planes_e
		{
			LEFT,
			RIGHT,
			NEAR,
			TOP,
			BOTTOM,
			FAR
		};

		// Fovx is the angle in the y axis (imagine the cam positioned in the default OGL pos)
		// Note that fovx > fovy (most of the time) and aspect_ratio = fovx/fovy
		// fovx and fovy in rad
		float fovx, fovy;
		float znear, zfar;

		// the frustum planes in local and world space
		plane_t lspace_frustum_planes[6];
		plane_t wspace_frustum_planes[6];

		// matrices
		mat4_t projection_mat;
		mat4_t view_mat;

	public:
		// constructors and destuctors
		camera_t( float fovx_, float fovy_, float znear_, float zfar_ ): fovx(fovx_), fovy(fovy_), znear(znear_), zfar(zfar_)
		{
			CalcLSpaceFrustumPlanes();
			UpdateWSpaceFrustumPlanes();
			UpdateProjectionMatrix();
		}
		camera_t() {}
		~camera_t() {}

		// Sets & Gets
		void SetFovX ( float fovx_ )  { fovx=fovx_; UpdateProjectionMatrix(); }
		void SetFovY ( float fovy_ )  { fovy=fovy_; UpdateProjectionMatrix(); }
		void SetZNear( float znear_ ) { znear=znear_; UpdateProjectionMatrix(); }
		void SetZFar ( float zfar_ )  { zfar=zfar_; UpdateProjectionMatrix(); }
		float GetFovX () const { return fovx; }
		float GetFovY () const { return fovy; }
		float GetZNear() const { return znear; }
		float GetZFar () const { return zfar; }
		const mat4_t& GetProjectionMatrix() const { return projection_mat; }
		const mat4_t& GetViewMatrix() const { return view_mat; }

		// misc
		void LookAtPoint( const vec3_t& point );
		void UpdateProjectionMatrix();
		void UpdateViewMatrix();
		void Update();
		void RenderDebug();
		void Render();

		// frustum stuff
		void CalcLSpaceFrustumPlanes();
		void UpdateWSpaceFrustumPlanes();
		bool InsideFrustum( const bvolume_t& vol );
};


#endif
