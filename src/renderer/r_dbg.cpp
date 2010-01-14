#include "renderer.h"
#include "r_private.h"
#include "fbo.h"
#include "scene.h"
#include "texture.h"
#include "fbo.h"
#include "node.h"
#include "skel_node.h"


namespace r {
namespace dbg {


/*
=======================================================================================================================================
DATA VARS                                                                                                                             =
=======================================================================================================================================
*/
bool show_axis = true;
bool show_fnormals = false;
bool show_vnormals = false;
bool show_lights = true;
bool show_skeletons = false;
bool show_cameras = true;
bool show_bvolumes = true;


static fbo_t fbo;

static shader_prog_t* shdr;


/*
=======================================================================================================================================
Init                                                                                                                                  =
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
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, r::pps::fai.GetGLID(), 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, r::ms::depth_fai.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create debug FBO" );

	// unbind
	fbo.Unbind();

	// shader
	shdr = rsrc::shaders.Load( "shaders/dbg.glsl" );
}


/*
=======================================================================================================================================
RunStage                                                                                                                              =
=======================================================================================================================================
*/
void RunStage( const camera_t& cam )
{
	fbo.Bind();

	shdr->Bind();

	// OGL stuff
	SetProjectionViewMatrices( cam );
	SetViewport( 0, 0, r::w*r::rendering_quality, r::h*r::rendering_quality );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	//r::RenderGrid();
	for( uint i=0; i<scene::nodes.size(); i++ )
	{
		if
		(
			(scene::nodes[i]->type == node_t::NT_LIGHT && show_lights) ||
			(scene::nodes[i]->type == node_t::NT_CAMERA && show_cameras)
		)
		{
			scene::nodes[i]->Render();
		}
		else if( scene::nodes[i]->type == node_t::NT_SKELETON && show_skeletons )
		{
			skel_node_t* skel_node = static_cast<skel_node_t*>( scene::nodes[i] );
			glDisable( GL_DEPTH_TEST );
			skel_node->Render();
			glEnable( GL_DEPTH_TEST );
		}
	}


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
		glNormal3fv( &(-vec3_t( 0.0, 0.0, 1.0 ))[0] );
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
				SinCos( theta1, sintheta1, costheta1 );
				float sintheta2, costheta2;
				SinCos( theta2, sintheta2, costheta2 );
				float sintheta3, costheta3;
				SinCos( theta3, sintheta3, costheta3 );


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


} } // end namespaces
