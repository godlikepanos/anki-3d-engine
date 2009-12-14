#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <cstring>
#include "common.h"
#include "primitives.h"
#include "collision.h"
#include "object.h"

class camera_data_user_class_t: public data_user_class_t {}; // for ambiguity reasons

class camera_t: public object_t, public camera_data_user_class_t
{
	public:
		enum planes_e
		{
			LEFT_PL = 0,
			RIGHT_PL,
			NEAR_PL,
			TOP_PL,
			BOTTOM_PL,
			FAR_PL
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
		/**
		 * Used in deferred shading for the calculation of view vector (see CalcViewVector). The reason we store this matrix here is
		 * that we dont want it to be re-calculated all the time but only when the projection params (fovx, fovy, znear, zfar) change.
		 * Fortunately the projection params change rarely. Note that the camera_t as we all know re-calculates the matreces only when the
		 * parameteres change!!
		 */
		mat4_t inv_projection_mat;

		// misc
		void CalcProjectionMatrix();
		void UpdateViewMatrix();
		void CalcLSpaceFrustumPlanes();
		void UpdateWSpaceFrustumPlanes();

	public:
		// constructors and destuctors
		camera_t( float fovx_, float fovy_, float znear_, float zfar_ ): object_t(CAMERA), fovx(fovx_), fovy(fovy_), znear(znear_), zfar(zfar_)
		{
			CalcLSpaceFrustumPlanes();
			UpdateWSpaceFrustumPlanes();
			CalcProjectionMatrix();
		}
		camera_t( const camera_t& c ): object_t(CAMERA) { memcpy( this, &c, sizeof( camera_t ) ); }
		camera_t(): object_t(CAMERA) {}
		~camera_t() {}

		// Sets & Gets
		void SetFovX ( float fovx_ )  { fovx=fovx_; CalcProjectionMatrix(); CalcLSpaceFrustumPlanes(); }
		void SetFovY ( float fovy_ )  { fovy=fovy_; CalcProjectionMatrix(); CalcLSpaceFrustumPlanes(); }
		void SetZNear( float znear_ ) { znear=znear_; CalcProjectionMatrix(); CalcLSpaceFrustumPlanes(); }
		void SetZFar ( float zfar_ )  { zfar=zfar_; CalcProjectionMatrix(); CalcLSpaceFrustumPlanes(); }
		void SetAll( float fovx_, float fovy_, float znear_, float zfar_ );
		float GetFovX () const { return fovx; }
		float GetFovY () const { return fovy; }
		float GetZNear() const { return znear; }
		float GetZFar () const { return zfar; }
		const mat4_t& GetProjectionMatrix() const { return projection_mat; }
		const mat4_t& GetViewMatrix() const { return view_mat; }
		const mat4_t& GetInvProjectionMatrix() const { return inv_projection_mat; } // see the declaration of inv_projection_mat for info

		// misc
		void LookAtPoint( const vec3_t& point );
		void UpdateWorldStuff();
		void RenderDebug();
		void Render();
		void RenderDepth() { ERROR("You may want to reconsider rendering a camera for depth passes") };

		// frustum stuff
		bool InsideFrustum( const bvolume_t& vol ) const;
		bool InsideFrustum( const camera_t& cam ) const; // check if another camera is inside our view (used for projected lights)
};


#endif
