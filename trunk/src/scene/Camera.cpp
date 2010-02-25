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

	glColor3fv( &Vec3(1.0,0.0,1.0)[0] );
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

//	if( !strcmp( camera_data_user_class_t::getName(), "mainCam") ) return;
//	//for( uint i=0; i<2; i++ )
//		wspaceFrustumPlanes[TOP].render();
}


//=====================================================================================================================================
// lookAtPoint                                                                                                                        =
//=====================================================================================================================================
void Camera::lookAtPoint( const Vec3& point )
{
	const Vec3& j = Vec3( 0.0, 1.0, 0.0 );
	Vec3 vdir = (point - translationLspace).getNormalized();
	Vec3 vup = j - vdir * j.dot(vdir);
	Vec3 vside = vdir.cross( vup );
	rotationLspace.setColumns( vside, vup, -vdir );
}


//=====================================================================================================================================
// calcLSpaceFrustumPlanes                                                                                                            =
//=====================================================================================================================================
void Camera::calcLSpaceFrustumPlanes()
{
	float c, s; // cos & sine

	sinCos( PI+fovX/2, s, c );
	// right
	lspaceFrustumPlanes[FP_RIGHT] = plane_t( Vec3(c, 0.0, s), 0.0 );
	// left
	lspaceFrustumPlanes[FP_LEFT] = plane_t( Vec3(-c, 0.0, s), 0.0 );

	sinCos( (3*PI-fovY)*0.5, s, c );
	// top
	lspaceFrustumPlanes[FP_TOP] = plane_t( Vec3(0.0, s, c), 0.0 );
	// bottom
	lspaceFrustumPlanes[FP_BOTTOM] = plane_t( Vec3(0.0, -s, c), 0.0 );

	// near
	lspaceFrustumPlanes[FP_NEAR] = plane_t( Vec3( 0.0, 0.0, -1.0 ), zNear );
	// far
	lspaceFrustumPlanes[FP_FAR] = plane_t( Vec3( 0.0, 0.0, 1.0 ), -zFar );
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
	Vec3 points[5];

	// get 3 sample floats
	float x = cam.getZFar() / tan( (PI-cam.getFovX())/2 );
	float y = tan( cam.getFovY()/2 ) * cam.getZFar();
	float z = -cam.getZFar();

	// the actual points in local space
	points[0] = Vec3( x, y, z ); // top right
	points[1] = Vec3( -x, y, z ); // top left
	points[2] = Vec3( -x, -y, z ); // bottom left
	points[3] = Vec3( x, -y, z ); // bottom right
	points[4] = Vec3( cam.translationWspace ); // eye (allready in world space)

	// transform them to the given camera's world space (exept the eye)
	for( uint i=0; i<4; i++ )
		points[i].transform( cam.translationWspace, cam.rotationWspace, cam.scaleWspace );


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

	invProjectionMat = projectionMat.getInverse();
}


//=====================================================================================================================================
// updateViewMatrix                                                                                                                   =
//=====================================================================================================================================
void Camera::updateViewMatrix()
{
	/* The point at which the camera looks:
	Vec3 viewpoint = translationLspace + z_axis;
	as we know the up vector, we can easily use gluLookAt:
	gluLookAt( translationLspace.x, translationLspace.x, translationLspace.z, z_axis.x, z_axis.y, z_axis.z, y_axis.x, y_axis.y, y_axis.z );
	*/


	// The view matrix is: Mview = camera.world_transform.Inverted(). Bus instead of inverting we do the following:
	Mat3 cam_inverted_rot = rotationWspace.getTransposed();
	Vec3 cam_inverted_tsl = -( cam_inverted_rot * translationWspace );
	viewMat = Mat4( cam_inverted_tsl, cam_inverted_rot );
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



