#include "Renderer.h"
#include "Fbo.h"
#include "Scene.h"
#include "Texture.h"
#include "Fbo.h"
#include "SceneNode.h"
#include "SkelNode.h"
#include "App.h"
#include "PhyCommon.h"

extern btDefaultCollisionConfiguration* collisionConfiguration;
extern btCollisionDispatcher* dispatcher;
extern btDbvtBroadphase* broadphase;
extern btSequentialImpulseConstraintSolver* sol;
extern btDiscreteDynamicsWorld* dynamicsWorld;

void renderscene( int pass )
{
	return;
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
			MotionState* myMotionState = (MotionState*)body->getMotionState();
			myMotionState->getWorldTransform().getOpenGLMatrix( m );
			rot = myMotionState->getWorldTransform().getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix( m );
			rot = colObj->getWorldTransform().getBasis();
		}

		glPushMatrix();
		glMultMatrixf(m);

		R::Dbg::renderCube( true, 2.0 );

		glPopMatrix();
	}
}






namespace R {
namespace Dbg {


static void renderSun();

//=====================================================================================================================================
// DATA VARS                                                                                                                          =
//=====================================================================================================================================
bool showAxis = true;
bool showFnormals = false;
bool showVnormals = false;
bool showLights = true;
bool showSkeletons = false;
bool showCameras = true;
bool showBvolumes = true;

static Fbo fbo;

class DbgShaderProg: public ShaderProg
{
	public:
		struct
		{
			int color;
		}uniLocs;
};

static DbgShaderProg sProg;


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// attach the textures
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, R::Pps::fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, R::Ms::depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create debug FBO" );

	// unbind
	fbo.unbind();

	// shader
	sProg.customLoad( "shaders/Dbg.glsl" );
}


float projectRadius( float r, const Vec3& location, const Camera& cam )
{
	Vec3 axis = cam.rotationWspace.getXAxis();
	float c = axis.dot( cam.translationWspace );
	float dist = axis.dot( location ) - c;

	/*if( dist > 0.0 )
		return 0.0;*/

	Vec3 p( 0.0, fabs(r), -dist );
	Vec4 projected = cam.getProjectionMatrix() * Vec4( p, 1.0 );
	float pr = projected.y / projected.w;

	/*if ( pr > 1.0 )
		pr = 1.0;*/

	return pr;
}


//=====================================================================================================================================
// runStage                                                                                                                           =
//=====================================================================================================================================
void runStage( const Camera& cam )
{
	fbo.bind();

	sProg.bind();

	// OGL stuff
	R::setProjectionViewMatrices( cam );
	R::setViewport( 0, 0, R::w, R::h );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	//R::renderGrid();
	for( uint i=0; i<app->getScene()->nodes.size(); i++ )
	{
		if
		(
			(app->getScene()->nodes[i]->type == SceneNode::NT_LIGHT && showLights) ||
			(app->getScene()->nodes[i]->type == SceneNode::NT_CAMERA && showCameras) ||
			app->getScene()->nodes[i]->type == SceneNode::NT_PARTICLE_EMITTER
		)
		{
			app->getScene()->nodes[i]->render();
		}
		else if( app->getScene()->nodes[i]->type == SceneNode::NT_SKELETON && showSkeletons )
		{
			SkelNode* skel_node = static_cast<SkelNode*>( app->getScene()->nodes[i] );
			glDisable( GL_DEPTH_TEST );
			skel_node->render();
			glEnable( GL_DEPTH_TEST );
		}
	}

	// the sun
	//RenderSun();

	renderscene(1);


	glDisable( GL_DEPTH_TEST );

	glPushMatrix();
	R::multMatrix( Mat4( Vec3(5.0, 2.0, 2.0), Mat3::getIdentity(), 1.0 ) );
	R::color3( Vec3(1,0,0) );
	R::Dbg::renderSphere( 1.2, 16 );
	glPopMatrix();


	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( -1, 1, -1, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();


	Vec3 c = Vec3(5.0, 2.0, 2.0);
	float r = 1.2;

	Vec4 p = Vec4( c, 1.0 );
	p = app->getActiveCam()->getProjectionMatrix() * (app->getActiveCam()->getViewMatrix() * p);
	p /= p.w;
	//p = p/2 + 0.5;

	glPointSize( 10 );
	glBegin( GL_POINTS );
		R::color3( Vec3(0.0,1.0,0.0) );
		glVertex2fv( &p[0] );
	glEnd();


	/*Vec4 g = Vec4( Vec3(c) + Vec3(r,0,0), 1.0 );
	g = app->activeCam->getProjectionMatrix() * (app->activeCam->getViewMatrix() * g);
	g /= g.w;
	float len = Vec2(p-g).getLength();
	//g = g/2 + 0.5;

	glPointSize( 10 );
	glBegin( GL_POINTS );
		R::color3( Vec3(1.0,1.0,1.0) );
		glVertex2fv( &(g)[0] );
	glEnd();*/
	float pr = projectRadius( r, c, cam );
	//PRINT( pr );
	glPointSize( 10 );
	glBegin( GL_POINTS );
		R::color3( Vec3(1.0,0.0,1.0) );
		glVertex2fv( &( Vec2(p) + Vec2(pr,0.0) )[0] );
	glEnd();

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	// unbind
	fbo.unbind();
}


//=====================================================================================================================================
// renderGrid                                                                                                                         =
//=====================================================================================================================================
void renderGrid()
{
	float col0[] = { 0.5, 0.5, 0.5 };
	float col1[] = { 0.0, 0.0, 1.0 };
	float col2[] = { 1.0, 0.0, 0.0 };

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


//=====================================================================================================================================
// renderQuad                                                                                                                         =
//=====================================================================================================================================
void renderQuad( float w, float h )
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


//=====================================================================================================================================
// renderSphere                                                                                                                       =
//=====================================================================================================================================
void renderSphere( float r, int p )
{
	const float twopi  = M::PI*2;
	const float pidiv2 = M::PI/2;

	float theta1 = 0.0;
	float theta2 = 0.0;
	float theta3 = 0.0;

	float ex = 0.0;
	float ey = 0.0;
	float ez = 0.0;

	float px = 0.0;
	float py = 0.0;
	float pz = 0.0;


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


//=====================================================================================================================================
// renderCube                                                                                                                         =
//=====================================================================================================================================
void renderCube( bool cols, float size )
{
	size *= 0.5f;
	glBegin(GL_QUADS);
		// Front Face
		if(cols) glColor3f( 0.0, 0.0, 1.0 );
		glNormal3f( 0.0, 0.0, 1.0f);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size,  size);
		glTexCoord2f(1.0, 0.0); glVertex3f( size, -size,  size);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  size);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  size);
		// Back Face
		if(cols) glColor3f( 0.0, 0.0, size );
		glNormal3f( 0.0, 0.0,-1.0f);
		glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size, -size);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size,  size, -size);
		glTexCoord2f(0.0, 1.0); glVertex3f( size,  size, -size);
		glTexCoord2f(0.0, 0.0); glVertex3f( size, -size, -size);
		// Top Face
		if(cols) glColor3f( 0.0, 1.0, 0.0 );
		glNormal3f( 0.0, 1.0f, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size, -size);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size,  size);
		glTexCoord2f(1.0, 0.0); glVertex3f( size,  size,  size);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size, -size);
		// Bottom Face
		if(cols) glColor3f( 0.0, size, 0.0 );
		glNormal3f( 0.0,-1.0f, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size, -size, -size);
		glTexCoord2f(0.0, 1.0); glVertex3f( size, -size, -size);
		glTexCoord2f(0.0, 0.0); glVertex3f( size, -size,  size);
		glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size,  size);
		// Right face
		if(cols) glColor3f( 1.0, 0.0, 0.0 );
		glNormal3f( 1.0f, 0.0, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( size, -size, -size);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size, -size);
		glTexCoord2f(0.0, 1.0); glVertex3f( size,  size,  size);
		glTexCoord2f(0.0, 0.0); glVertex3f( size, -size,  size);
		// Left Face
		if(cols) glColor3f( size, 0.0, 0.0 );
		glNormal3f(-1.0f, 0.0, 0.0);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size, -size);
		glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size,  size);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size,  size,  size);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size, -size);
	glEnd();
}


//=====================================================================================================================================
// RenderSun                                                                                                                          =
//=====================================================================================================================================
static void renderSun()
{
	glPushMatrix();

	R::multMatrix( Mat4( app->getScene()->getSunPos(), Mat3::getIdentity(), 50.0 ) );

	R::color3( Vec3(1.0, 1.0, 0.0) );
	R::Dbg::renderSphere( 1.0/8.0, 8 );

	glPopMatrix();


/*	/////////////////////////////////////////////////////
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 0, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();


	Vec4 p = Vec4( app->getScene()->getSunPos(), 1.0 );
	p = mainCam->getProjectionMatrix() * (mainCam->getViewMatrix() * p);
	p /= p.w;
	p = p/2 + 0.5;

	glPointSize( 10 );
	glBegin( GL_POINTS );
		R::color3( Vec3(0.0,1.0,0.0) );
		glVertex3fv( &p[0] );
	glEnd();

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();*/
}


} } // end namespaces
