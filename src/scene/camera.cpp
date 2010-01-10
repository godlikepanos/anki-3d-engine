#include "camera.h"
#include "renderer.h"


//=====================================================================================================================================
// SetAll                                                                                                                             =
//=====================================================================================================================================
void camera_t::SetAll( float fovx_, float fovy_, float znear_, float zfar_ )
{
	fovx = fovx_;
	fovy = fovy_;
	znear = znear_;
	zfar = zfar_;
	CalcProjectionMatrix();
	CalcLSpaceFrustumPlanes();
}


//=====================================================================================================================================
// Render                                                                                                                             =
//=====================================================================================================================================
void camera_t::Render()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	const float cam_len = 1.0;
	float tmp0 = cam_len / tan( (PI - fovx)*0.5 ) + 0.001;
	float tmp1 = cam_len * tan(fovy*0.5) + 0.001;

	float points [][3] = {
		{0.0, 0.0, 0.0}, // 0: eye point
		{-tmp0, tmp1, -cam_len}, // 1: top left
		{-tmp0, -tmp1, -cam_len}, // 2: bottom left
		{tmp0, -tmp1, -cam_len}, // 3: bottom right
		{tmp0, tmp1, -cam_len}, // 4: top right
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
//		wspace_frustum_planes[TOP].Render();
}


//=====================================================================================================================================
// LookAtPoint                                                                                                                        =
//=====================================================================================================================================
void camera_t::LookAtPoint( const vec3_t& point )
{
	const vec3_t& j = vec3_t( 0.0, 1.0, 0.0 );
	vec3_t vdir = (point - translation_lspace).GetNormalized();
	vec3_t vup = j - vdir * j.Dot(vdir);
	vec3_t vside = vdir.Cross( vup );
	rotation_lspace.SetColumns( vside, vup, -vdir );
}


//=====================================================================================================================================
// CalcLSpaceFrustumPlanes                                                                                                            =
//=====================================================================================================================================
void camera_t::CalcLSpaceFrustumPlanes()
{
	float c, s; // cos & sine

	SinCos( PI+fovx/2, s, c );
	// right
	lspace_frustum_planes[FP_RIGHT] = plane_t( vec3_t(c, 0.0, s), 0.0 );
	// left
	lspace_frustum_planes[FP_LEFT] = plane_t( vec3_t(-c, 0.0, s), 0.0 );

	SinCos( (3*PI-fovy)*0.5, s, c );
	// top
	lspace_frustum_planes[FP_TOP] = plane_t( vec3_t(0.0, s, c), 0.0 );
	// bottom
	lspace_frustum_planes[FP_BOTTOM] = plane_t( vec3_t(0.0, -s, c), 0.0 );

	// near
	lspace_frustum_planes[FP_NEAR] = plane_t( vec3_t( 0.0, 0.0, -1.0 ), znear );
	// far
	lspace_frustum_planes[FP_FAR] = plane_t( vec3_t( 0.0, 0.0, 1.0 ), -zfar );
}


//=====================================================================================================================================
// UpdateWSpaceFrustumPlanes                                                                                                          =
//=====================================================================================================================================
void camera_t::UpdateWSpaceFrustumPlanes()
{
	for( uint i=0; i<6; i++ )
		wspace_frustum_planes[i] = lspace_frustum_planes[i].Transformed( translation_wspace, rotation_wspace, scale_wspace );
}


//=====================================================================================================================================
// InsideFrustum                                                                                                                      =
//=====================================================================================================================================
/// Check if the volume is inside the frustum cliping planes
bool camera_t::InsideFrustum( const bvolume_t& bvol ) const
{
	for( uint i=0; i<6; i++ )
		if( bvol.PlaneTest( wspace_frustum_planes[i] ) < 0.0 )
			return false;

	return true;
}


//=====================================================================================================================================
// InsideFrustum                                                                                                                      =
//=====================================================================================================================================
/// Check if the given camera is inside the frustum cliping planes. This is used mainly to test if the projected lights are visible
bool camera_t::InsideFrustum( const camera_t& cam ) const
{
	//** get five points. These points are the tips of the given camera **
	vec3_t points[5];

	// get 3 sample floats
	float x = cam.GetZFar() / tan( (PI-cam.GetFovX())/2 );
	float y = tan( cam.GetFovY()/2 ) * cam.GetZFar();
	float z = -cam.GetZFar();

	// the actual points in local space
	points[0] = vec3_t( x, y, z ); // top right
	points[1] = vec3_t( -x, y, z ); // top left
	points[2] = vec3_t( -x, -y, z ); // bottom left
	points[3] = vec3_t( x, -y, z ); // bottom right
	points[4] = vec3_t( cam.translation_wspace ); // eye (allready in world space)

	// transform them to the given camera's world space (exept the eye)
	for( uint i=0; i<4; i++ )
		points[i].Transform( cam.translation_wspace, cam.rotation_wspace, cam.scale_wspace );


	//** the collision code **
	for( uint i=0; i<6; i++ ) // for the 6 planes
	{
		int failed = 0;

		for( uint j=0; j<5; j++ ) // for the 5 points
		{
			if( wspace_frustum_planes[i].Test( points[j] ) < 0.0 )
				++failed;
		}
		if( failed == 5 ) return false; // if all points are behind the plane then the cam is not in frustum
	}

	return true;
}


//=====================================================================================================================================
// CalcProjectionMatrix                                                                                                               =
//=====================================================================================================================================
void camera_t::CalcProjectionMatrix()
{
	float f = 1.0/tan( fovy*0.5f ); // f = cot(fovy/2)

	projection_mat(0,0) = f*fovy/fovx; // = f/aspect_ratio;
	projection_mat(0,1) = 0.0;
	projection_mat(0,2) = 0.0;
	projection_mat(0,3) = 0.0;
	projection_mat(1,0) = 0.0;
	projection_mat(1,1) = f;
	projection_mat(1,2) = 0.0;
	projection_mat(1,3) = 0.0;
	projection_mat(2,0) = 0.0;
	projection_mat(2,1) = 0.0;
	projection_mat(2,2) = (zfar+znear) / (znear-zfar);
	projection_mat(2,3) = (2.0f*zfar*znear) / (znear-zfar);
	projection_mat(3,0) = 0.0;
	projection_mat(3,1) = 0.0;
	projection_mat(3,2) = -1.0;
	projection_mat(3,3) = 0.0;

	inv_projection_mat = projection_mat.GetInverse();
}


//=====================================================================================================================================
// UpdateViewMatrix                                                                                                                   =
//=====================================================================================================================================
void camera_t::UpdateViewMatrix()
{
	/* The point at which the camera looks:
	vec3_t viewpoint = translation_lspace + z_axis;
	as we know the up vector, we can easily use gluLookAt:
	gluLookAt( translation_lspace.x, translation_lspace.x, translation_lspace.z, z_axis.x, z_axis.y, z_axis.z, y_axis.x, y_axis.y, y_axis.z );
	*/


	// The view matrix is: Mview = camera.world_transform.Inverted(). Bus instead of inverting we do the following:
	mat3_t cam_inverted_rot = rotation_wspace.GetTransposed();
	vec3_t cam_inverted_tsl = -( cam_inverted_rot * translation_wspace );
	view_mat = mat4_t( cam_inverted_tsl, cam_inverted_rot );
}


//=====================================================================================================================================
// UpdateWorldStuff                                                                                                                   =
//=====================================================================================================================================
void camera_t::UpdateWorldStuff()
{
	UpdateWorldTransform();
	UpdateViewMatrix();
	UpdateWSpaceFrustumPlanes();
}



