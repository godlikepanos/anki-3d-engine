#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "SkelNode.h"


//=====================================================================================================================================
// Constructor                                                                                                                        =
//=====================================================================================================================================
Renderer::Dbg::Dbg( Renderer& r_ ):
	RenderingStage( r_ ),
	showAxisEnabled( false ),
	showLightsEnabled( false ),
	showSkeletonsEnabled( false ),
	showCamerasEnabled( false )
{
}


//=====================================================================================================================================
// renderGrid                                                                                                                         =
//=====================================================================================================================================
void Renderer::Dbg::renderGrid()
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
// renderSphere                                                                                                                       =
//=====================================================================================================================================
void Renderer::Dbg::renderSphere( float radius, int complexity )
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
void Renderer::Dbg::renderCube( bool cols, float size )
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
// init                                                                                                                               =
//=====================================================================================================================================
void Renderer::Dbg::init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// attach the textures
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r.pps.fai.getGlId(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create debug FBO" );

	// unbind
	fbo.unbind();

	// shader
	sProg.customLoad( "shaders/Dbg.glsl" );
}


//=====================================================================================================================================
// runStage                                                                                                                           =
//=====================================================================================================================================
void Renderer::Dbg::run()
{
	if( !enabled ) return;

	const Camera& cam = *r.cam;

	fbo.bind();
	sProg.bind();

	// OGL stuff
	r.setProjectionViewMatrices( cam );
	r.setViewport( 0, 0, r.width, r.height );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	//R::renderGrid();
	for( uint i=0; i<app->getScene()->nodes.size(); i++ )
	{
		if
		(
			(app->getScene()->nodes[i]->type == SceneNode::NT_LIGHT && showLightsEnabled) ||
			(app->getScene()->nodes[i]->type == SceneNode::NT_CAMERA && showCamerasEnabled) ||
			app->getScene()->nodes[i]->type == SceneNode::NT_PARTICLE_EMITTER
		)
		{
			app->getScene()->nodes[i]->render();
		}
		else if( app->getScene()->nodes[i]->type == SceneNode::NT_SKELETON && showSkeletonsEnabled )
		{
			SkelNode* skel_node = static_cast<SkelNode*>( app->getScene()->nodes[i] );
			glDisable( GL_DEPTH_TEST );
			skel_node->render();
			glEnable( GL_DEPTH_TEST );
		}
	}
}

