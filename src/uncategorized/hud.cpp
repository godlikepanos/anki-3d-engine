#include <stdio.h>
#include <stdarg.h>
#include "hud.h"
#include "renderer.h"
#include "texture.h"
#include "resource.h"

namespace hud {


/*
=======================================================================================================================================
data members                                                                                                                          =
=======================================================================================================================================
*/
static texture_t* font_map;

static shader_prog_t* shader;

static float  initial_x;
static float  font_w;
static float  font_h;
static float  color[4];
static bool   italic;
static float  crnt_x;
static float  crnt_y;



/*
=======================================================================================================================================
static funcs                                                                                                                          =
=======================================================================================================================================
*/


// SetGL
static void SetGL()
{
	shader->Bind();
	shader->LocTexUnit( shader->GetUniformLocation(0), *font_map, 0 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
	glColor4fv( color );

	// matrix stuff
	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	r::LoadMatrix( r::Ortho( -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 ) );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

}


// RestoreGL
static void RestoreGL()
{
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_TEXTURE );
	glPopMatrix();
}


// DrawChar
static void DrawChar( char c )
{
	// first check for special chars
	if( c=='\n' ) // new line
	{
		//glTranslatef( initial_x, -font_h, 0.0f );
		crnt_x = initial_x;
		crnt_y -= font_h;
		return;
	}
	if( c=='\t' ) // tab
	{
		DrawChar( ' ' );
		DrawChar( ' ' );
		return;
	}
	if( c<' ' || c>'~' ) // out of range
	{
		c = '~'+1;
	}

	const int chars_per_line = 16; // the chars that font_map.tga has per line
	const int lines_num       = 8; // the lines of chars in font_map.tga

	// the uvs
	float char_width = 1.0f/float(chars_per_line);
	float char_height = 1.0f/float(lines_num);
	float uv_top = float(lines_num - (c-' ')/chars_per_line) / float(lines_num);
	float uv_left = float( (c-' ')%chars_per_line ) / float(chars_per_line);
	float uv_right = uv_left + char_width;
	float uv_bottom = uv_top - char_height;
	float uvs[4][2] = { {uv_left, uv_top}, {uv_left, uv_bottom}, {uv_right, uv_bottom}, {uv_right, uv_top} };

	// the coords
	float fwh = font_w/2.0f;
	float fhh = font_h/2.0f;
	float coords[4][2] = { {-fwh, fhh}, {-fwh, -fhh}, {fwh, -fhh}, {fwh, fhh} }; // fron top left counterclockwise


	if( italic )
	{
		coords[0][0] += font_w/5.0f;
		coords[3][0] += font_w/5.0f;
	}

	glBegin(GL_QUADS);
		glTexCoord2fv( uvs[0] );  // top left
		glVertex2fv( coords[0] );
		glTexCoord2fv( uvs[1] );  // bottom left
		glVertex2fv( coords[1] );
		glTexCoord2fv( uvs[2] );  // botton right
		glVertex2fv( coords[2] );
		glTexCoord2fv( uvs[3] );  // top right
		glVertex2fv( coords[3] );
	glEnd();

	// draw outline
	/*if( 1 )
	glDisable( GL_TEXTURE_2D );
	glColor3f( 0.0, 0.0, 1.0 );
	glBegin(GL_LINES);
		glVertex2fv( coords[0] );
		glVertex2fv( coords[1] );
		glVertex2fv( coords[2] );
		glVertex2fv( coords[3] );
	glEnd();
	glEnable( GL_TEXTURE_2D );*/
	// end draw outline

	crnt_x += font_w*0.8f;
	//glTranslatef( font_w*0.8f, 0.0, 0.0 ); // font_w*(float) to remove the space
}


/*
=======================================================================================================================================
non static funcs                                                                                                                      =
=======================================================================================================================================
*/


// Init
// exec after init SDL
void Init()
{
	font_map = rsrc::textures.Load( "gfx/fontmapa.tga" );
	font_map->TexParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//font_map->TexParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	shader = rsrc::shaders.Load( "shaders/txt.shdr" );
	SetPos( 0.0f, 0.0f );
	SetFontWidth( 0.05f );
	SetColor( vec4_t(1.0f, 1.0f, 1.0f, 1.0f) );
	italic = false;
}


// SetFontWidth
// sets font width from the given param and the height fron the aspect ratio
void SetFontWidth( float w_ )
{
	// width
	font_w = w_;
	// height
	font_h = font_w * r::aspect_ratio;
}


// SetColor
void SetColor( const vec4_t& color_ )
{
	for( int i=0; i<4; i++ )
		color[i] = color_[i];
}

// SetPos
void SetPos( float x_, float y_ )
{
	initial_x = crnt_x = x_;
	crnt_y = y_;
}


// Printf
void Printf( const char* format, ... )
{
	va_list ap;
	char text[512];

	va_start(ap, format);		// Parses The String For Variables
		vsprintf(text, format, ap);  // And Converts Symbols To Actual Numbers
	va_end(ap);

	Print( text );
}


//Print
void Print( const char* text )
{
	SetGL();
	glTranslatef( crnt_x, crnt_y, 0.0f );
	for( char* pc=const_cast<char*>(text); *pc!='\0'; pc++ )
	{
		glLoadIdentity();
		glTranslatef( crnt_x, crnt_y, 0.0f );
		DrawChar( *pc );
	}
	RestoreGL();
}


} // end namespace
