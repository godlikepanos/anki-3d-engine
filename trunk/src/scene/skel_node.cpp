#include "skel_node.h"
#include "renderer.h"
#include "SkelAnim.h"
#include "Skeleton.h"
#include "skel_anim_ctrl.h"


//=====================================================================================================================================
// skel_node_t                                                                                                                        =
//=====================================================================================================================================
skel_node_t::skel_node_t(): 
	node_t( NT_SKELETON ),
	skel_anim_ctrl( NULL )
{
}


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void skel_node_t::Init( const char* filename )
{
	skeleton = rsrc::skeletons.load( filename );
	skel_anim_ctrl = new skel_anim_ctrl_t( this );
}


//=====================================================================================================================================
// Deinit                                                                                                                             =
//=====================================================================================================================================
void skel_node_t::Deinit()
{
	rsrc::skeletons.unload( skeleton );
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
			glVertex3fv( &skel_anim_ctrl->heads[i][0] );
		glEnd();

		glBegin( GL_LINES );
			glVertex3fv( &skel_anim_ctrl->heads[i][0] );
			glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
			glVertex3fv( &skel_anim_ctrl->tails[i][0] );
		glEnd();
	}

	glPopMatrix();
}
