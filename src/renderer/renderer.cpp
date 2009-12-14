#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include "renderer.h"
#include "texture.h"
#include "scene.h"
#include "r_private.h"

namespace r {


/*
=======================================================================================================================================
data vars                                                                                                                             =
=======================================================================================================================================
*/

// misc
uint w = 1280, h = 800;
//uint w = 720, h = 480;
uint frames_num = 0;
float aspect_ratio = (float)w/(float)h;

int max_color_atachments = 0;
float rendering_quality = 1.;
int screenshot_jpeg_quality = 90;

static shader_prog_t* shdr_final;

// for the pps and is quad rendering
float quad_vert_cords [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };


/**
 * Standard shader preprocessor defines. Used to pass some global params to the shaders. The standard shader preprocessor defines
 * go on top of the shader code and its defines
 */
char* std_shader_preproc_defines = NULL;

// texture
bool mipmaping = true;
int max_anisotropy = 8;
int max_texture_units = -1;
bool texture_compression = false;


/*
=======================================================================================================================================
BuildStdShaderPreProcStr                                                                                                              =
I pass all the static vars (vars that do not change at all) with defines so I dont have to update uniform vars                        =
=======================================================================================================================================
*/
static void BuildStdShaderPreProcStr()
{
	string tmp = "";

	tmp += "#version 120\n";
	tmp += "#pragma optimize(on)\n";
	tmp += "#pragma debug(off)\n";
	tmp += "#define R_W " + FloatToStr(r::w) + "\n";
	tmp += "#define R_H " + FloatToStr(r::h) + "\n";
	tmp += "#define R_Q " + FloatToStr(r::rendering_quality) + "\n";
	tmp += "#define SHADOWMAP_SIZE " + IntToStr(r::is::shadows::shadow_resolution) + "\n";
	if( r::is::shadows::pcf )
		tmp += "#define _SHADOW_MAPPING_PCF_\n";
	if( r::pps::ssao::enabled )
	{
		tmp += "#define _SSAO_\n";
		tmp += "#define SSAO_RENDERING_QUALITY " + FloatToStr(r::pps::ssao::rendering_quality) + "\n";
	}
	if( r::pps::edgeaa::enabled )
		tmp += "#define _EDGEAA_\n";
	if( r::pps::bloom::enabled )
	{
		tmp += "#define _BLOOM_\n";
		tmp += "#define BLOOM_RENDERING_QUALITY " + FloatToStr(r::pps::bloom::rendering_quality) + "\n";
	}
	if( r::pps::lscatt::enabled )
	{
		tmp += "#define _LSCATT_\n";
		tmp += "#define LSCATT_RENDERING_QUALITY " + FloatToStr(r::pps::lscatt::rendering_quality) + "\n";
	}

	std_shader_preproc_defines = new char [tmp.length()+1];
	strcpy( std_shader_preproc_defines, tmp.c_str() );
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	PRINT( "Renderer initializing..." );
	glewInit();
	if( !glewIsSupported("GL_VERSION_2_1") )
		WARNING( "OpenGL ver 2.1 not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_EXT_framebuffer_object") )
		WARNING( "Framebuffer objects not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_ARB_vertex_buffer_object") )
		WARNING( "Vertex buffer objects not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_ARB_texture_non_power_of_two") )
		WARNING( "Textures of non power of two not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_ARB_vertex_buffer_object") )
		WARNING( "Vertex Buffer Objects not supported. The application may crash (and burn)" );

	glClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
	glClearDepth( 1.0f );
	glClearStencil( 0 );
	glDepthFunc( GL_LEQUAL );

	// query for max_color_atachments
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &max_color_atachments );

	// get max texture units
	glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &max_texture_units );

	// CullFace is always on
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );

	// defaults
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glPolygonMode( GL_FRONT, GL_FILL );

	// Hints
	//glHint( GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_FASTEST );
	//glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
	//glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
	//glHint( GL_POINT_SMOOTH_HINT, GL_FASTEST );
	//glHint( GL_POLYGON_SMOOTH_HINT, GL_FASTEST );
	//glHint( GL_TEXTURE_COMPRESSION_HINT, GL_NICEST );

	// execute this after the cvars are set and before the other inits (be cause these inits contain shader loads)
	BuildStdShaderPreProcStr();

	// misc
	shdr_final = rsrc::shaders.Load( "shaders/final.shdr" );

	// init deffered stages
	// WARNING: the order of the inits is crucial!!!!!
	r::ms::Init();
	r::is::Init();
	r::bs::Init();
	r::pps::Init();
	r::dbg::Init();

	PRINT( "Renderer initialization ends" );
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void Render( const camera_t& cam )
{
	r::ms::RunStage( cam );
	r::is::RunStage( cam );
	r::bs::RunStage( cam );
	r::pps::RunStage( cam );
	r::dbg::RunStage( cam );

	r::SetViewport( 0, 0, r::w, r::h );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	shdr_final->Bind();
	shdr_final->LocTexUnit( shdr_final->GetUniformLocation(0), r::pps::fai, 0 );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
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
Unproject                                                                                                                             =
my version of gluUnproject                                                                                                            =
=======================================================================================================================================
*/
bool Unproject( float winX, float winY, float winZ, // window screen coords
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
Ortho                                                                                                                                 =
=======================================================================================================================================
*/
mat4_t Ortho( float left, float right, float bottom, float top, float near, float far )
{
	float difx = right-left;
	float dify = top-bottom;
	float difz = far-near;
	float tx = -(right+left) / difx;
	float ty = -(top+bottom) / dify;
	float tz = -(far+near) / difz;
	mat4_t m;

	m(0,0) = 2.0 / difx;
	m(0,1) = 0.0;
	m(0,2) = 0.0;
	m(0,3) = tx;
	m(1,0) = 0.0;
	m(1,1) = 2.0 / dify;
	m(1,2) = 0.0;
	m(1,3) = ty;
	m(2,0) = 0.0;
	m(2,1) = 0.0;
	m(2,2) = -2.0 / difz;
	m(2,3) = tz;
	m(3,0) = 0.0;
	m(3,1) = 0.0;
	m(3,2) = 0.0;
	m(3,3) = 1.0;

	return m;
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
		ERROR( "GL_ERR: " << gluErrorString( errid ) );
}


/*
=======================================================================================================================================
TakeScreenshotTGA                                                                                                                     =
=======================================================================================================================================
*/
static bool TakeScreenshotTGA( const char* filename )
{
	// open file and check
	fstream fs;
	fs.open( filename, ios::out|ios::binary );
	if( !fs.good() )
	{
		ERROR( "Cannot create screenshot. File \"" << filename << "\"" );
		return false;
	}

	// write headers
	unsigned char tga_header_uncompressed[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
	unsigned char header[6];

	header[1] = r::w / 256;
	header[0] = r::w % 256;
	header[3] = r::h / 256;
	header[2] = r::h % 256;
	header[4] = 24;
	header[5] = 0;

	fs.write( (char*)tga_header_uncompressed, 12 );
	fs.write( (char*)header, 6 );

	// write the buffer
	char* buffer = (char*)calloc( r::w*r::h*3, sizeof(char) );

	glReadPixels( 0, 0, r::w, r::h, GL_BGR, GL_UNSIGNED_BYTE, buffer );
	fs.write( buffer, r::w*r::h*3 );

	// end
	fs.close();
	free( buffer );
	return true;
}


/*
=======================================================================================================================================
TakeScreenshotJPEG                                                                                                                    =
=======================================================================================================================================
*/
static bool TakeScreenshotJPEG( const char* filename )
{
	// open file
	FILE* outfile = fopen( filename, "wb" );

	if( !outfile )
	{
		ERROR( "Cannot open file \"" << filename << "\"" );
		return false;
	}

	// set jpg params
	jpeg_compress_struct cinfo;
	jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress( &cinfo );
	jpeg_stdio_dest( &cinfo, outfile );

	cinfo.image_width      = r::w;
	cinfo.image_height     = r::h;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults( &cinfo);
	jpeg_set_quality ( &cinfo, screenshot_jpeg_quality, true );
	jpeg_start_compress( &cinfo, true );

	// read from OGL
	char* buffer = (char*)malloc( r::w*r::h*3*sizeof(char) );
	glReadPixels( 0, 0, r::w, r::h, GL_RGB, GL_UNSIGNED_BYTE, buffer );

	// write buffer to file
	JSAMPROW row_pointer;

	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer = (JSAMPROW) &buffer[ (r::h-1-cinfo.next_scanline)*3*r::w ];
		jpeg_write_scanlines( &cinfo, &row_pointer, 1 );
	}

	jpeg_finish_compress(&cinfo);

	// done
	free( buffer );
	return true;
}


/*
=======================================================================================================================================
TakeScreenshot                                                                                                                        =
=======================================================================================================================================
*/
void TakeScreenshot( const char* filename )
{
	char* ext = GetFileExtension( filename );
	bool ret;

	// exec from this extension
	if( strcmp( ext, "tga" ) == 0 )
	{
		ret = TakeScreenshotTGA( filename );
	}
	else if( strcmp( ext, "jpg" ) == 0 )
	{
		ret = TakeScreenshotJPEG( filename );
	}
	else
	{
		ERROR( "File \"" << filename << "\": Unsupported extension. Watch for capital" );
		return;
	}
	if( !ret ) ERROR( "In taking screenshot" )
	else INFO( "Screenshot \"" << filename << "\" saved" );
}


} // end namespace

