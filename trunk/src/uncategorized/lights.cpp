#include "lights.h"
#include "collision.h"
#include "renderer.h"


/*
=======================================================================================================================================
RenderDebug                                                                                                                           =
=======================================================================================================================================
*/
void light_t::RenderDebug()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	r::Color3( diffuse_color );
	r::dbg::RenderSphere( 1.0f/8.0f, 8 );

	glPopMatrix();
}

/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void point_light_t::Render()
{
	RenderDebug();

	//bsphere_t sphere( translation_wspace, radius );
	//sphere.Render();
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void spot_light_t::Render()
{
	RenderDebug();

//	vec3_t center( 0.0, 0.0, -camera.GetZFar()/2.0 );
//	obb_t box( center, mat3_t::ident, vec3_t(1.0, 1.0, 1.0) );
//	box = box.Transformed( translation_wspace, rotation_wspace, scale_wspace );
//	box.Render();


//	float x = camera.GetZFar() / tan( (PI-camera.GetFovX())/2 );
//	float y = tan( camera.GetFovY()/2 ) * camera.GetZFar();
//	float z = -camera.GetZFar();
//
//	vec3_t right_top( x, y, z );
//	vec3_t left_top( -x, y, z );
//	vec3_t left_bottom( -x, -y, z );
//	vec3_t right_bottom( x, -y, z );
//
//	right_top.Transform( translation_wspace, rotation_wspace, scale_wspace );
//	left_top.Transform( translation_wspace, rotation_wspace, scale_wspace );
//	left_bottom.Transform( translation_wspace, rotation_wspace, scale_wspace );
//	right_bottom.Transform( translation_wspace, rotation_wspace, scale_wspace );
//
//	glPointSize( 10.0 );
//	r::Color3( vec3_t( 0.0, 1.0, 0.0 ) );
//	glBegin( GL_LINES );
//		glVertex3fv( &translation_wspace[0] );
//		glVertex3fv( &right_top[0] );
//		glVertex3fv( &translation_wspace[0] );
//		glVertex3fv( &left_top[0] );
//		glVertex3fv( &translation_wspace[0] );
//		glVertex3fv( &left_bottom[0] );
//		glVertex3fv( &translation_wspace[0] );
//		glVertex3fv( &right_bottom[0] );
//	glEnd();
}
