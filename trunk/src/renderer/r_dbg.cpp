#include "renderer.h"
#include "r_private.h"
#include "fbo.h"
#include "Scene.h"
#include "Texture.h"
#include "fbo.h"
#include "Node.h"
#include "SkelNode.h"


#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDebuger.h"

extern btDefaultCollisionConfiguration* collisionConfiguration;
extern btCollisionDispatcher* dispatcher;
extern btDbvtBroadphase* broadphase;
extern btSequentialImpulseConstraintSolver* sol;
extern btDiscreteDynamicsWorld* dynamicsWorld;

void renderscene( int pass )
{
	btScalar m[16];
	btMatrix3x3 rot;
	rot.setIdentity();
	const int numObjects = dynamicsWorld->getNumCollisionObjects();
	btVector3 wireColor( 1, 0, 0 );
	for( int i = 0; i < numObjects; i++ )
	{
		btCollisionObject* colObj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast( colObj );
		if( body && body->getMotionState() )
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
			myMotionState->m_graphicsWorldTrans.getOpenGLMatrix( m );
			rot = myMotionState->m_graphicsWorldTrans.getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix( m );
			rot = colObj->getWorldTransform().getBasis();
		}
		btVector3 wireColor( 1.f, 1.0f, 0.5f ); //wants deactivation
		if( i & 1 ) wireColor = btVector3( 0.f, 0.0f, 1.f );
		///color differently for active, sleeping, wantsdeactivation states
		if( colObj->getActivationState() == 1 ) //active
		{
			if( i & 1 )
			{
				wireColor += btVector3( 1.f, 0.f, 0.f );
			}
			else
			{
				wireColor += btVector3( .5f, 0.f, 0.f );
			}
		}
		if( colObj->getActivationState() == 2 ) //ISLAND_SLEEPING
		{
			if( i & 1 )
			{
				wireColor += btVector3( 0.f, 1.f, 0.f );
			}
			else
			{
				wireColor += btVector3( 0.f, 0.5f, 0.f );
			}
		}

		btVector3 aabbMin, aabbMax;
		dynamicsWorld->getBroadphase()->getBroadphaseAabb( aabbMin, aabbMax );

		aabbMin -= btVector3( BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT );
		aabbMax += btVector3( BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT );
		//		printf("aabbMin=(%f,%f,%f)\n",aabbMin.getX(),aabbMin.getY(),aabbMin.getZ());
		//		printf("aabbMax=(%f,%f,%f)\n",aabbMax.getX(),aabbMax.getY(),aabbMax.getZ());
		//		m_dynamicsWorld->getDebugDrawer()->drawAabb(aabbMin,aabbMax,btVector3(1,1,1));


		if( 1 )
		{
			switch( pass )
			{
				case 0:
					//m_shapeDrawer->drawOpenGL( m, colObj->getCollisionShape(), wireColor, getDebugMode(), aabbMin, aabbMax );
					break;
				case 1:
					//m_shapeDrawer->drawShadow( m, m_sundirection * rot, colObj->getCollisionShape(), aabbMin, aabbMax );
					break;
				case 2:
					//m_shapeDrawer->drawOpenGL( m, colObj->getCollisionShape(), wireColor * btScalar( 0.3 ), 0, aabbMin, aabbMax );
					break;
			}
		}
	}
}






namespace r {
namespace dbg {


static void RenderSun();

//=====================================================================================================================================
// DATA VARS                                                                                                                          =
//=====================================================================================================================================
bool show_axis = true;
bool show_fnormals = false;
bool show_vnormals = false;
bool show_lights = true;
bool show_skeletons = false;
bool show_cameras = true;
bool show_bvolumes = true;


static fbo_t fbo;

static ShaderProg* shdr;


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// attach the textures
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::pps::fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.getGlId(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create debug FBO" );

	// unbind
	fbo.Unbind();

	// shader
	shdr = rsrc::shaders.load( "shaders/dbg.glsl" );
}


/*
=======================================================================================================================================
RunStage                                                                                                                              =
=======================================================================================================================================
*/
void RunStage( const Camera& cam )
{
	fbo.Bind();

	shdr->bind();

	// OGL stuff
	SetProjectionViewMatrices( cam );
	SetViewport( 0, 0, r::w, r::h );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	//r::RenderGrid();
	for( uint i=0; i<scene::nodes.size(); i++ )
	{
		if
		(
			(scene::nodes[i]->type == Node::NT_LIGHT && show_lights) ||
			(scene::nodes[i]->type == Node::NT_CAMERA && show_cameras)
		)
		{
			scene::nodes[i]->render();
		}
		else if( scene::nodes[i]->type == Node::NT_SKELETON && show_skeletons )
		{
			SkelNode* skel_node = static_cast<SkelNode*>( scene::nodes[i] );
			glDisable( GL_DEPTH_TEST );
			skel_node->render();
			glEnable( GL_DEPTH_TEST );
		}
	}

	// the sun
	//RenderSun();

	renderscene(1);


	// unbind
	fbo.Unbind();
}


/*
=======================================================================================================================================
RenderGrid                                                                                                                            =
=======================================================================================================================================
*/
void RenderGrid()
{
	float col0[] = { 0.5f, 0.5f, 0.5f };
	float col1[] = { 0.0f, 0.0f, 1.0f };
	float col2[] = { 1.0f, 0.0f, 0.0f };

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_LINE_STIPPLE );
	//glLineWidth(1.0);
	glColor3fv( col0 );

	const float space = 1.0; // space between lines
	const int num = 57;  // lines number. must be odd

	float opt = ((num-1)*space/2);
	glBegin( GL_LINES );
		for( int x=0; x<num; x++ )
		{
			if( x==num/2 ) // if the middle line then change color
				glColor3fv( col1 );
			else if( x==(num/2)+1 ) // if the next line after the middle one change back to default col
				glColor3fv( col0 );

			float opt1 = (x*space);
			// line in z
			glVertex3f( opt1-opt, 0.0, -opt );
			glVertex3f( opt1-opt, 0.0, opt );

			if( x==num/2 ) // if middle line change col so you can highlight the x-axis
				glColor3fv( col2 );

			// line in the x
			glVertex3f( -opt, 0.0, opt1-opt );
			glVertex3f( opt, 0.0, opt1-opt );
		}
	glEnd();
}


/*
=======================================================================================================================================
RenderQuad                                                                                                                            =
=======================================================================================================================================
*/
void RenderQuad( float w, float h )
{
	float wdiv2 = w/2, hdiv2 = h/2;
	float points [][2] = { {wdiv2,hdiv2}, {-wdiv2,hdiv2}, {-wdiv2,-hdiv2}, {wdiv2,-hdiv2} };
	float uvs [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };

	glBegin( GL_QUADS );
		glNormal3fv( &(-Vec3( 0.0, 0.0, 1.0 ))[0] );
		glTexCoord2fv( uvs[0] );
		glVertex2fv( points[0] );
		glTexCoord2fv( uvs[1] );
		glVertex2fv( points[1] );
		glTexCoord2fv( uvs[2] );
		glVertex2fv( points[2] );
		glTexCoord2fv( uvs[3] );
		glVertex2fv( points[3] );
	glEnd();
}


/*
=======================================================================================================================================
RenderSphere                                                                                                                          =
=======================================================================================================================================
*/
void RenderSphere( float r, int p )
{
	const float twopi  = PI*2;
	const float pidiv2 = PI/2;

	float theta1 = 0.0;
	float theta2 = 0.0;
	float theta3 = 0.0;

	float ex = 0.0f;
	float ey = 0.0f;
	float ez = 0.0f;

	float px = 0.0f;
	float py = 0.0f;
	float pz = 0.0f;


	for( int i = 0; i < p/2; ++i )
	{
		theta1 = i * twopi / p - pidiv2;
		theta2 = (i + 1) * twopi / p - pidiv2;

		glBegin( GL_QUAD_STRIP );
		{
			for( int j = p; j >= 0; --j )
			{
				theta3 = j * twopi / p;

				float sintheta1, costheta1;
				sinCos( theta1, sintheta1, costheta1 );
				float sintheta2, costheta2;
				sinCos( theta2, sintheta2, costheta2 );
				float sintheta3, costheta3;
				sinCos( theta3, sintheta3, costheta3 );


				ex = costheta2 * costheta3;
				ey = sintheta2;
				ez = costheta2 * sintheta3;
				px = r * ex;
				py = r * ey;
				pz = r * ez;

				glNormal3f( ex, ey, ez );
				glTexCoord2f( -(j/(float)p) , 2*(i+1)/(float)p );
				glVertex3f( px, py, pz );

				ex = costheta1 * costheta3;
				ey = sintheta1;
				ez = costheta1 * sintheta3;
				px = r * ex;
				py = r * ey;
				pz = r * ez;

				glNormal3f( ex, ey, ez );
				glTexCoord2f( -(j/(float)p), 2*i/(float)p );
				glVertex3f( px, py, pz );
			}
		}
		glEnd();
	}
}


/*
=======================================================================================================================================
RenderCube                                                                                                                            =
=======================================================================================================================================
*/
void RenderCube( bool cols, float size )
{
	size *= 0.5f;
	glBegin(GL_QUADS);
		// Front Face
		if(cols) glColor3f( 0.0, 0.0, 1.0 );
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size,  size);
		glTexCoord2f(1.0, 0.0); glVertex3f( size, -size,  size);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  size);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  size);
		// Back Face
		if(cols) glColor3f( 0.0, 0.0, size );
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size, -size);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size,  size, -size);
		glTexCoord2f(0.0, 1.0); glVertex3f( size,  size, -size);
		glTexCoord2f(0.0, 0.0); glVertex3f( size, -size, -size);
		// Top Face
		if(cols) glColor3f( 0.0, 1.0, 0.0 );
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size, -size);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size,  size);
		glTexCoord2f(1.0, 0.0); glVertex3f( size,  size,  size);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size, -size);
		// Bottom Face
		if(cols) glColor3f( 0.0, size, 0.0 );
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size, -size, -size);
		glTexCoord2f(0.0, 1.0); glVertex3f( size, -size, -size);
		glTexCoord2f(0.0, 0.0); glVertex3f( size, -size,  size);
		glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size,  size);
		// Right face
		if(cols) glColor3f( 1.0, 0.0, 0.0 );
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0, 0.0); glVertex3f( size, -size, -size);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size, -size);
		glTexCoord2f(0.0, 1.0); glVertex3f( size,  size,  size);
		glTexCoord2f(0.0, 0.0); glVertex3f( size, -size,  size);
		// Left Face
		if(cols) glColor3f( size, 0.0, 0.0 );
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size, -size);
		glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size,  size);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size,  size,  size);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size, -size);
	glEnd();
}


//=====================================================================================================================================
// RenderSun                                                                                                                          =
//=====================================================================================================================================
static void RenderSun()
{
	glPushMatrix();

	r::MultMatrix( Mat4( scene::SunPos(), Mat3::getIdentity(), 50.0 ) );

	r::Color3( Vec3(1.0, 1.0, 0.0) );
	r::dbg::RenderSphere( 1.0/8.0, 8 );

	glPopMatrix();


/*	/////////////////////////////////////////////////////
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 0, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();


	Vec4 p = Vec4( scene::SunPos(), 1.0 );
	p = main_cam->getProjectionMatrix() * (main_cam->getViewMatrix() * p);
	p /= p.w;
	p = p/2 + 0.5;

	glPointSize( 10 );
	glBegin( GL_POINTS );
		r::Color3( Vec3(0.0,1.0,0.0) );
		glVertex3fv( &p[0] );
	glEnd();

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();*/
}


} } // end namespaces
