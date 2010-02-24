#include "Camera.h"
#include "renderer.h"


//=====================================================================================================================================
// setAll                                                                                                                             =
//=====================================================================================================================================
void Camera::setAll( float fovx_, float fovy_, float znear_, float zfar_ )
{
	fovX = fovx_;
	fovY = fovy_;
	zNear = znear_;
	zFar = zfar_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
void Camera::render()
{
	glPushMatrix();
	r::MultMatrix( transformationWspace );

	const float camLen = 1.0;
	float tmp0 = camLen / tan( (PI - fovX)*0.5 ) + 0.001;
	float tmp1 = camLen * tan(fovY*0.5) + 0.001;

	float points [][3] = {
		{0.0, 0.0, 0.0}, // 0: eye point
		{-tmp0, tmp1, -camLen}, // 1: top left
		{-tmp0, -tmp1, -camLen}, // 2: bottom left
		{tmp0, -tmp1, -camLen}, // 3: bottom right
		{tmp0, tmp1, -camLen}, // 4: top right
	};

	//glLineWidth( 2.0 );

	glColor3fv( &vec3_t(1.0,0.0,1.0)[0] );
	glBegin( GL_LINES );
		glVertex3fv( &points[0][0] );
		glVertex3fv( &points[1][0] );
		glVertex3fv( &points[0][0] );
		glVertex3fv( &points[2][0] );
		glVertex3fv( &points[0][0] );
		glVertex3fv( &points[3][0] );
		glVertex3fv( &points[0][0] );
		glVertex3fv( &points[4][0] );
	glEnd();

	glBegin( GL_LINE_STRIP );
		glVertex3fv( &points[1][0] );
		glVertex3fv( &points[2][0] );
		glVertex3fv( &points[3][0] );
		glVertex3fv( &points[4][0] );
		glVertex3fv( &points[1][0] );
	glEnd();


	glPopMatrix();

//	if( !strcmp( camera_data_user_class_t::GetName(), "main_cam") ) return;
//	//for( uint i=0; i<2; i++ )
//		wspaceFrustumPlanes[TOP].render();
}


//=====================================================================================================================================
// lookAtPoint                                                                                                                        =
//=====================================================================================================================================
void Camera::lookAtPoint( const vec3_t& point )
{
	const vec3_t& j = vec3_t( 0.0, 1.0, 0.0 );
	vec3_t vdir = (point - translationLspace).GetNormalized();
	vec3_t vup = j - vdir * j.Dot(vdir);
	vec3_t vside = vdir.Cross( vup );
	rotationLspace.SetColumns( vside, vup, -vdir );
}


//=====================================================================================================================================
// calcLSpaceFrustumPlanes                                                                                                            =
//=====================================================================================================================================
void Camera::calcLSpaceFrustumPlanes()
{
	float c, s; // cos & sine

	sinCos( PI+fovX/2, s, c );
	// right
	lspaceFrustumPlanes[FP_RIGHT] = plane_t( vec3_t(c, 0.0, s), 0.0 );
	// left
	lspaceFrustumPlanes[FP_LEFT] = plane_t( vec3_t(-c, 0.0, s), 0.0 );

	sinCos( (3*PI-fovY)*0.5, s, c );
	// top
	lspaceFrustumPlanes[FP_TOP] = plane_t( vec3_t(0.0, s, c), 0.0 );
	// bottom
	lspaceFrustumPlanes[FP_BOTTOM] = plane_t( vec3_t(0.0, -s, c), 0.0 );

	// near
	lspaceFrustumPlanes[FP_NEAR] = plane_t( vec3_t( 0.0, 0.0, -1.0 ), zNear );
	// far
	lspaceFrustumPlanes[FP_FAR] = plane_t( vec3_t( 0.0, 0.0, 1.0 ), -zFar );
}


//=====================================================================================================================================
// updateWSpaceFrustumPlanes                                                                                                          =
//=====================================================================================================================================
void Camera::updateWSpaceFrustumPlanes()
{
	for( uint i=0; i<6; i++ )
		wspaceFrustumPlanes[i] = lspaceFrustumPlanes[i].Transformed( translationWspace, rotationWspace, scaleWspace );
}


//=====================================================================================================================================
// insideFrustum                                                                                                                      =
//=====================================================================================================================================
/// Check if the volume is inside the frustum cliping planes
bool Camera::insideFrustum( const bvolume_t& bvol ) const
{
	for( uint i=0; i<6; i++ )
		if( bvol.PlaneTest( wspaceFrustumPlanes[i] ) < 0.0 )
			return false;

	return true;
}


//=====================================================================================================================================
// insideFrustum                                                                                                                      =
//=====================================================================================================================================
/// Check if the given camera is inside the frustum cliping planes. This is used mainly to test if the projected lights are visible
bool Camera::insideFrustum( const Camera& cam ) const
{
	//** get five points. These points are the tips of the given camera **
	vec3_t points[5];

	// get 3 sample floats
	float x = cam.getZFar() / tan( (PI-cam.getFovX())/2 );
	float y = tan( cam.getFovY()/2 ) * cam.getZFar();
	float z = -cam.getZFar();

	// the actual points in local space
	points[0] = vec3_t( x, y, z ); // top right
	points[1] = vec3_t( -x, y, z ); // top left
	points[2] = vec3_t( -x, -y, z ); // bottom left
	points[3] = vec3_t( x, -y, z ); // bottom right
	points[4] = vec3_t( cam.translationWspace ); // eye (allready in world space)

	// transform them to the given camera's world space (exept the eye)
	for( uint i=0; i<4; i++ )
		points[i].Transform( cam.translationWspace, cam.rotationWspace, cam.scaleWspace );


	//** the collision code **
	for( uint i=0; i<6; i++ ) // for the 6 planes
	{
		int failed = 0;

		for( uint j=0; j<5; j++ ) // for the 5 points
		{
			if( wspaceFrustumPlanes[i].Test( points[j] ) < 0.0 )
				++failed;
		}
		if( failed == 5 ) return false; // if all points are behind the plane then the cam is not in frustum
	}

	return true;
}


//=====================================================================================================================================
// calcProjectionMatrix                                                                                                               =
//=====================================================================================================================================
void Camera::calcProjectionMatrix()
{
	float f = 1.0/tan( fovY*0.5f ); // f = cot(fovY/2)

	projectionMat(0,0) = f*fovY/fovX; // = f/aspect_ratio;
	projectionMat(0,1) = 0.0;
	projectionMat(0,2) = 0.0;
	projectionMat(0,3) = 0.0;
	projectionMat(1,0) = 0.0;
	projectionMat(1,1) = f;
	projectionMat(1,2) = 0.0;
	projectionMat(1,3) = 0.0;
	projectionMat(2,0) = 0.0;
	projectionMat(2,1) = 0.0;
	projectionMat(2,2) = (zFar+zNear) / (zNear-zFar);
	projectionMat(2,3) = (2.0f*zFar*zNear) / (zNear-zFar);
	projectionMat(3,0) = 0.0;
	projectionMat(3,1) = 0.0;
	projectionMat(3,2) = -1.0;
	projectionMat(3,3) = 0.0;

	invProjectionMat = projectionMat.GetInverse();
}


//=====================================================================================================================================
// updateViewMatrix                                                                                                                   =
//=====================================================================================================================================
void Camera::updateViewMatrix()
{
	/* The point at which the camera looks:
	vec3_t viewpoint = translationLspace + z_axis;
	as we know the up vector, we can easily use gluLookAt:
	gluLookAt( translationLspace.x, translationLspace.x, translationLspace.z, z_axis.x, z_axis.y, z_axis.z, y_axis.x, y_axis.y, y_axis.z );
	*/


	// The view matrix is: Mview = camera.world_transform.Inverted(). Bus instead of inverting we do the following:
	mat3_t cam_inverted_rot = rotationWspace.GetTransposed();
	vec3_t cam_inverted_tsl = -( cam_inverted_rot * translationWspace );
	viewMat = mat4_t( cam_inverted_tsl, cam_inverted_rot );
}


//=====================================================================================================================================
// updateWorldStuff                                                                                                                   =
//=====================================================================================================================================
void Camera::updateWorldStuff()
{
	updateWorldTransform();
	updateViewMatrix();
	updateWSpaceFrustumPlanes();
}



