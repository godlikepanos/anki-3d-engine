#include "skel_node.h"
#include "renderer.h"
#include "skel_anim.h"
#include "skeleton.h"
#include "skel_anim_controller.h"


//=====================================================================================================================================
// skel_node_t                                                                                                                        =
//=====================================================================================================================================
skel_node_t::skel_node_t(): 
	node_t( NT_SKELETON )
{
	skel_anim_controller = new skel_anim_controller_t(this); // It allways have a controller
}


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void skel_node_t::Init( const char* filename )
{
	skeleton = rsrc::skeletons.Load( filename );
}

//=====================================================================================================================================
// Deinit                                                                                                                             =
//=====================================================================================================================================
void skel_node_t::Deinit()
{
	rsrc::skeletons.Unload( skeleton );
}


//=====================================================================================================================================
// Render                                                                                                                             =
//=====================================================================================================================================
void skel_node_t::Render()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	//glPointSize( 4.0f );
	//glLineWidth( 2.0f );

	for( uint i=0; i<skeleton->bones.size(); i++ )
	{
		glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
		glBegin( GL_POINTS );
			glVertex3fv( &skel_anim_controller->heads[0][0] );
		glEnd();

		glBegin( GL_LINES );
			glVertex3fv( &skel_anim_controller->heads[0][0] );
			glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
			glVertex3fv( &skel_anim_controller->tails[0][0] );
		glEnd();
	}

	glPopMatrix();
}
