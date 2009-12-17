#include "renderer.h"

namespace r {


namespace dfr {

extern void Init(); // none else needs to know about r::dfr::Init()

}


/*
=======================================================================================================================================
data vars                                                                                                                             =
=======================================================================================================================================
*/

uint w = 1024, h = 768, frames_num = 0;
float aspect_ratio = (float)w/(float)h;

int  max_color_atachments = 0;

// standard shader preprocessor defines. Used to pass some global params to the shaders
char* std_shader_preproc_defines = " ";

// if to show debug crap
bool show_axis = true;
bool show_fnormals = false;
bool show_vnormals = false;
bool show_lights = true;
bool show_skeletons = true;
bool show_cameras = true;
bool show_bvolumes = true;


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	glewInit();
	if( !glewIsSupported("GL_VERSION_2_1") )
	{
		FATAL( "OpenGL ver 2.1 not supported" );
	}

	if( !glewIsSupported("GL_EXT_framebuffer_object") )
	{
		FATAL( "Frame buffer objects not supported" );
	}

	glClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
	glClearDepth( 1.0f );
	glClearStencil( 0 );
	glDepthFunc( GL_LEQUAL );


	// query for max_color_atachments
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &max_color_atachments );


	// CullFace is always on
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	// init deffered stuf
	dfr::Init();
}


/*
=======================================================================================================================================
SetProjectionMatrix                                                                                                                   =
=======================================================================================================================================
*/
void SetProjectionMatrix( const camera_t& cam )
{
	glMatrixMode( GL_PROJECTION );
	LoadMatrix( cam.GetProjectionMatrix() );
}


/*
=======================================================================================================================================
SetViewMatrix                                                                                                                         =
=======================================================================================================================================
*/
void SetViewMatrix( const camera_t& cam )
{
	glMatrixMode( GL_MODELVIEW );
	LoadMatrix( cam.GetViewMatrix() );
}


/*
=======================================================================================================================================
UnProject                                                                                                                             =
my version of gluUproject                                                                                                             =
=======================================================================================================================================
*/
bool UnProject( float winX, float winY, float winZ, // window screen coords
                const mat4_t& modelview_mat, const mat4_t& projection_mat, const int* view,
                float& objX, float& objY, float& objZ )
{
	mat4_t inv_pm = projection_mat * modelview_mat;
	inv_pm.Invert();

	// the vec is in ndc space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	vec4_t vec;
	vec.x = (2.0*(winX-view[0]))/view[2] - 1.0;
	vec.y = (2.0*(winY-view[1]))/view[3] - 1.0;
	vec.z = 2.0*winZ - 1.0;
	vec.w = 1.0;

	vec4_t final = inv_pm * vec;
	final /= final.w;
	objX = final.x;
	objY = final.y;
	objZ = final.z;
	return true;
}


/*
=======================================================================================================================================
PrepareNextFrame                                                                                                                      =
=======================================================================================================================================
*/
void PrepareNextFrame()
{
	frames_num++;
}


/*
=======================================================================================================================================
PrintLastError                                                                                                                        =
=======================================================================================================================================
*/
void PrintLastError()
{
	GLenum errid = glGetError();
	if( errid != GL_NO_ERROR )
	{
		int x = 10;
		ERROR( "GL_ERR: " << gluErrorString( errid ) );
	}
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
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_LINE_STIPPLE );
	glLineWidth(1.0);
	glColor3fv( col0 );

	float space = 1.0; // space between lines
	int num = 57;  // lines number. must be odd

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


/*
=======================================================================================================================================
SetGLState                                                                                                                            =
=======================================================================================================================================
*/

void SetGLState_Wireframe()
{
	NoShaders();
	glPolygonMode( GL_FRONT, GL_LINE );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_LINE_STIPPLE );
	glDisable( GL_BLEND );
	glLineWidth( 1.0 );
}

void SetGLState_WireframeDotted()
{
	NoShaders();
	glPolygonMode( GL_FRONT, GL_LINE );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LINE_STIPPLE );
	glLineStipple( 4, 0xAAAA );
	glLineWidth( 1.0 );
}

void SetGLState_Solid()
{
	NoShaders();
	glPolygonMode( GL_FRONT, GL_FILL );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_LINE_STIPPLE );
	glDisable( GL_BLEND );
}

void SetGLState_AlphaSolid()
{
	NoShaders();
	glPolygonMode( GL_FRONT, GL_FILL );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_LINE_STIPPLE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}


} // end namespace

