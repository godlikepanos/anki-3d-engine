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
	Vec3 axis = cam.getWorldTransform().getRotation().getXAxis();
	float c = axis.dot( cam.getWorldTransform().getOrigin() );
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
void renderSphere( float radius, int complexity )
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

	Vec<Vec3> positions;
	Vec<Vec3> normals;
	Vec<Vec2> texCoodrs;

	for( int i = 0; i < complexity/2; ++i )
	{
		theta1 = i * twopi / complexity - pidiv2;
		theta2 = (i + 1) * twopi / complexity - pidiv2;

		for( int j = complexity; j >= 0; --j )
		{
			theta3 = j * twopi / complexity;

			float sintheta1, costheta1;
			sinCos( theta1, sintheta1, costheta1 );
			float sintheta2, costheta2;
			sinCos( theta2, sintheta2, costheta2 );
			float sintheta3, costheta3;
			sinCos( theta3, sintheta3, costheta3 );


			ex = costheta2 * costheta3;
			ey = sintheta2;
			ez = costheta2 * sintheta3;
			px = radius * ex;
			py = radius * ey;
			pz = radius * ez;

			positions.push_back( Vec3(px, py, pz) );
			normals.push_back( Vec3(ex, ey, ez) );
			texCoodrs.push_back( Vec2(-(j/(float)complexity), 2*(i+1)/(float)complexity) );

			ex = costheta1 * costheta3;
			ey = sintheta1;
			ez = costheta1 * sintheta3;
			px = radius * ex;
			py = radius * ey;
			pz = radius * ez;

			positions.push_back( Vec3(px, py, pz) );
			normals.push_back( Vec3(ex, ey, ez) );
			texCoodrs.push_back( Vec2(-(j/(float)complexity), 2*i/(float)complexity) );
		}
	}

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, &positions[0][0] );
	glNormalPointer( GL_FLOAT, 0, &normals[0][0] );
	glDrawArrays( GL_QUAD_STRIP, 0, positions.size() );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
}


//=====================================================================================================================================
// renderCube                                                                                                                         =
//=====================================================================================================================================
void renderCube( bool cols, float size )
{
	Vec3 maxPos( 0.5 * size );
	Vec3 minPos( -0.5 * size );

	Vec3 vertPositions[] = {
		Vec3( maxPos.x, maxPos.y, maxPos.z ),  // right top front
		Vec3( minPos.x, maxPos.y, maxPos.z ),  // left top front
		Vec3( minPos.x, minPos.y, maxPos.z ),  // left bottom front
		Vec3( maxPos.x, minPos.y, maxPos.z ),  // right bottom front
		Vec3( maxPos.x, maxPos.y, minPos.z ),  // right top back
		Vec3( minPos.x, maxPos.y, minPos.z ),  // left top back
		Vec3( minPos.x, minPos.y, minPos.z ),  // left bottom back
		Vec3( maxPos.x, minPos.y, minPos.z )   // right bottom back
	};

	Vec3 bakedVertPositions[] = {
		vertPositions[0], vertPositions[1], vertPositions[2], vertPositions[3], // front face
		vertPositions[5], vertPositions[4], vertPositions[7], vertPositions[6], // back face
		vertPositions[4], vertPositions[0], vertPositions[3], vertPositions[7], // right face
		vertPositions[1], vertPositions[5], vertPositions[6], vertPositions[2], // left face
		vertPositions[0], vertPositions[4], vertPositions[5], vertPositions[1], // top face
		vertPositions[3], vertPositions[2], vertPositions[6], vertPositions[7]  // bottom face
	};

	static Vec3 normals[] = {
		Vec3( 0.0, 0.0, 1.0 ),  // front face
		Vec3( 0.0, 0.0, -1.0 ), // back face
		Vec3( 1.0, 0.0, 0.0 ),  // right face
		Vec3( -1.0, 0.0, 0.0 ), // left face
		Vec3( 0.0, 1.0, 0.0 ),  // top face
		Vec3( 0.0, -1.0, 0.0 )  // bottom face
	};

	static Vec3 bakedNormals [] = {
		normals[0], normals[0], normals[0], normals[0],
		normals[1], normals[1], normals[1], normals[1],
		normals[2], normals[2], normals[2], normals[2],
		normals[3], normals[3], normals[3], normals[3],
		normals[4], normals[4], normals[4], normals[4],
		normals[5], normals[5], normals[5], normals[5]
	};

	static Vec3 colors [] = {
		Vec3( 0.0, 0.0, 1.0 ),
		Vec3( 0.0, 0.0, 0.5 ),
		Vec3( 1.0, 0.0, 0.0 ),
		Vec3( 0.5, 0.0, 1.0 ),
		Vec3( 0.0, 1.0, 1.0 ),
		Vec3( 0.0, 0.5, 1.0 )
	};

	static Vec3 bakedColors [] = {
		colors[0], colors[0], colors[0], colors[0],
		colors[1], colors[1], colors[1], colors[1],
		colors[2], colors[2], colors[2], colors[2],
		colors[3], colors[3], colors[3], colors[3],
		colors[4], colors[4], colors[4], colors[4],
		colors[5], colors[5], colors[5], colors[5]
	};

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, bakedVertPositions );
	glDrawArrays( GL_QUADS, 0, 24 );
	glDisableClientState( GL_VERTEX_ARRAY );
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
