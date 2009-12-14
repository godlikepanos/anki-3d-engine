#include "camera.h"
#include "renderer.h"



/*
=======================================================================================================================================
RenderDebug                                                                                                                           =
=======================================================================================================================================
*/
void camera_t::RenderDebug()
{
	glPushMatrix();
	r::MultMatrix( world_transformation );

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

	glLineWidth( 2.0 );

	r::SetGLState_Wireframe();

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
}

/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void camera_t::Render()
{
	if( !r::show_cameras ) return;
	RenderDebug();
}


/*
=======================================================================================================================================
LookAtPoint                                                                                                                           =
=======================================================================================================================================
*/
void camera_t::LookAtPoint( const vec3_t& point )
{
	const vec3_t& j = vec3_t( 0.0, 1.0, 0.0 );
	vec3_t vdir = (point - local_translation).Normalized();
	vec3_t vup = j - vdir * j.Dot(vdir);
	vec3_t vside = vdir.Cross( vup );
	local_rotation.SetColumns( vside, vup, -vdir );
}


/*
=======================================================================================================================================
CalcLSpaceFrustumPlanes                                                                                                               =
=======================================================================================================================================
*/
void camera_t::CalcLSpaceFrustumPlanes()
{
	// right
	lspace_frustum_planes[RIGHT] = plane_t( vec3_t( cos(PI - fovx/2), 0.0f, sin(fovx - PI/2) ), 0.0f );
	// left
	lspace_frustum_planes[LEFT] = plane_t( vec3_t( -cos(PI - fovx/2), 0.0f, sin(fovx - PI/2) ), 0.0f );
	// top
	lspace_frustum_planes[TOP] = plane_t( vec3_t( 0.0f, sin( (3*PI-fovy)*0.5 ), cos( (3*PI-fovy)*0.5 ) ), 0.0f );
	// bottom
	lspace_frustum_planes[BOTTOM] = plane_t( vec3_t( 0.0f, -sin( (3*PI-fovy)*0.5 ), cos( (3*PI-fovy)*0.5 ) ), 0.0f );
	// near
	lspace_frustum_planes[NEAR] = plane_t( vec3_t( 0.0f, 0.0f, -1.0f ), znear );
	// far
	lspace_frustum_planes[FAR] = plane_t( vec3_t( 0.0f, 0.0f, 1.0f ), -zfar );
}


/*
=======================================================================================================================================
UpdateWSpaceFrustumPlanes                                                                                                             =
=======================================================================================================================================
*/
void camera_t::UpdateWSpaceFrustumPlanes()
{
	for( uint i=0; i<6; i++ )
		wspace_frustum_planes[i] = lspace_frustum_planes[i].Transformed( world_translation, world_rotation, world_scale );
}


/*
=======================================================================================================================================
InsideFrustum                                                                                                                         =
check if the volume is inside the frustum cliping planes                                                                              =
=======================================================================================================================================
*/
bool camera_t::InsideFrustum( const bvolume_t& bvol )
{
	for( uint i=0; i<6; i++ )
		if( bvol.PlaneTest( wspace_frustum_planes[i] ) < 0.0 )
			return false;

	return true;
}


/*
=======================================================================================================================================
UpdateProjectionMatrix                                                                                                                =
=======================================================================================================================================
*/
void camera_t::UpdateProjectionMatrix()
{
	float f = 1.0f/tan( fovy*0.5f ); // f = cot(fovy/2)

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
	projection_mat(3,2) = -1.0f;
	projection_mat(3,3) = 0.0;
}


/*
=======================================================================================================================================
UpdateViewMatrix                                                                                                                      =
=======================================================================================================================================
*/
void camera_t::UpdateViewMatrix()
{
	/* The point at which the camera looks:
	vec3_t viewpoint = local_translation + z_axis;
	as we know the up vector, we can easily use gluLookAt:
	gluLookAt( local_translation.x, local_translation.x, local_translation.z, z_axis.x, z_axis.y, z_axis.z, y_axis.x, y_axis.y, y_axis.z );
	*/


	// The view matrix is: Mview = camera.world_transform.Inverted(). Bus instead of inverting we do the following:
	mat3_t cam_inverted_rot = world_rotation.Transposed();
	vec3_t cam_inverted_tsl = -( cam_inverted_rot * world_translation );
	view_mat = mat4_t( cam_inverted_tsl, cam_inverted_rot );
}


/*
=======================================================================================================================================
Update                                                                                                                                =
=======================================================================================================================================
*/
void camera_t::Update()
{
	UpdateViewMatrix();
	UpdateWSpaceFrustumPlanes();
}



