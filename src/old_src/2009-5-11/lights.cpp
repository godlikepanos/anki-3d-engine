#include "lights.h"


uint light_t::lights_num = 0;


/*
=======================================================================================================================================
Update                                                                                                                                =
=======================================================================================================================================
*/
void light_t::Update()
{
	glEnable( GL_LIGHT0 + gl_id );
	glLightfv( GL_LIGHT0+gl_id, GL_POSITION, &world_translation[0] );
	glLightfv( GL_LIGHT0+gl_id, GL_SPOT_DIRECTION, &GetDir()[0] );
}


/*
=======================================================================================================================================
Render light                                                                                                                          =
=======================================================================================================================================
*/
void light_t::Render()
{
	glPushMatrix();
	r::MultMatrix( world_transformation );

	if( r::show_lights )
	{
		glEnable( GL_LIGHT0 + gl_id );
		// set GL
		r::SetGLState_Solid();
		glColor3fv( &(GetAmbient()*GetDiffuse())[0] );

		r::RenderSphere( 1.0f/8.0f, 8 );

		// render the light direction

		if( type != SPOT )
		{
			r::SetGLState_WireframeDotted();
			glLineWidth( 4.0 );
			glBegin( GL_LINES );
				glVertex3fv( &vec3_t( 0.0, 0.0, 0.0 )[0] );
				glVertex3fv( &(vec3_t( 1.0, 0.0, 0.0 )*2.0)[0] );
			glEnd();
		}

	}

	if( r::show_axis )
	{
		r::SetGLState_Solid();
		RenderAxis();
	}

	glPopMatrix();
}
