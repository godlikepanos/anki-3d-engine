#include "SkelNode.h"
#include "renderer.h"
#include "SkelAnim.h"
#include "Skeleton.h"
#include "SkelAnimCtrl.h"


//=====================================================================================================================================
// SkelNode                                                                                                                        =
//=====================================================================================================================================
SkelNode::SkelNode(): 
	Node( NT_SKELETON ),
	skelAnimCtrl( NULL )
{
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void SkelNode::init( const char* filename )
{
	skeleton = rsrc::skeletons.load( filename );
	skelAnimCtrl = new SkelAnimCtrl( this );
}


//=====================================================================================================================================
// deinit                                                                                                                             =
//=====================================================================================================================================
void SkelNode::deinit()
{
	rsrc::skeletons.unload( skeleton );
}


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
void SkelNode::render()
{
	glPushMatrix();
	r::MultMatrix( transformationWspace );

	//glPointSize( 4.0f );
	//glLineWidth( 2.0f );

	for( uint i=0; i<skeleton->bones.size(); i++ )
	{
		glColor3fv( &Vec3( 1.0, 1.0, 1.0 )[0] );
		glBegin( GL_POINTS );
			glVertex3fv( &skelAnimCtrl->heads[i][0] );
		glEnd();

		glBegin( GL_LINES );
			glVertex3fv( &skelAnimCtrl->heads[i][0] );
			glColor3fv( &Vec3( 1.0, 0.0, 0.0 )[0] );
			glVertex3fv( &skelAnimCtrl->tails[i][0] );
		glEnd();
	}

	glPopMatrix();
}
