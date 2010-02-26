#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include "renderer.h"
#include "Texture.h"
#include "Scene.h"
#include "Camera.h"
#include "App.h"

namespace r {


/*
=======================================================================================================================================
data vars                                                                                                                             =
=======================================================================================================================================
*/

// misc
uint w, h;
uint framesNum = 0;
float aspectRatio;

int maxColorAtachments = 0;
//float renderingQuality = 1.0;
int screenshotJpegQuality = 90;

static ShaderProg* shdr_final;

// for the pps and is quad rendering
float quad_vert_cords [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };


/**
 * Standard shader preprocessor defines. Used to pass some global params to the shaders. The standard shader preprocessor defines
 * go on top of the shader code and its defines
 */
string std_shader_preproc_defines;

// texture
bool mipmaping = true;
int max_anisotropy = 8;
int maxTextureUnits = -1;
bool textureCompression = false;


//=====================================================================================================================================
// DrawQuad                                                                                                                           =
//=====================================================================================================================================
void DrawQuad( int vertCoords_uni_loc )
{
	/*glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableClientState( GL_VERTEX_ARRAY );*/
	glVertexAttribPointer( vertCoords_uni_loc, 2, GL_FLOAT, false, 0, quad_vert_cords );
	glEnableVertexAttribArray( vertCoords_uni_loc );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableVertexAttribArray( vertCoords_uni_loc );
}


/*
=======================================================================================================================================
BuildStdShaderPreProcStr                                                                                                              =
I pass all the static vars (vars that do not change at all) with defines so I dont have to update uniform vars                        =
=======================================================================================================================================
*/
static void BuildStdShaderPreProcStr()
{
	string& tmp = std_shader_preproc_defines;

	tmp  = "#version 150 compatibility\n";
	tmp += "precision lowp float;\n";
	tmp += "#pragma optimize(on)\n";
	tmp += "#pragma debug(off)\n";
	tmp += "#define R_W " + Util::floatToStr(r::w) + "\n";
	tmp += "#define R_H " + Util::floatToStr(r::h) + "\n";
	//tmp += "#define R_Q " + floatToStr(r::renderingQuality) + "\n";
	tmp += "#define SHADOWMAP_SIZE " + Util::intToStr(r::is::shadows::shadowResolution) + "\n";
	if( r::is::shadows::pcf )
		tmp += "#define _SHADOW_MAPPING_PCF_\n";
	if( r::pps::ssao::enabled )
	{
		tmp += "#define _SSAO_\n";
		tmp += "#define SSAO_RENDERING_QUALITY " + Util::floatToStr(r::pps::ssao::renderingQuality) + "\n";
	}
	if( r::pps::edgeaa::enabled )
		tmp += "#define _EDGEAA_\n";
	if( r::pps::hdr::enabled )
	{
		tmp += "#define _HDR_\n";
		tmp += "#define HDR_RENDERING_QUALITY " + Util::floatToStr(r::pps::hdr::renderingQuality) + "\n";
	}
	if( r::pps::lscatt::enabled )
	{
		tmp += "#define _LSCATT_\n";
		tmp += "#define LSCATT_RENDERING_QUALITY " + Util::floatToStr(r::pps::lscatt::renderingQuality) + "\n";
	}
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	PRINT( "Renderer initializing..." );

	glewInit();
	if( !glewIsSupported("GL_VERSION_2_1") )
		WARNING( "OpenGL ver 2.1 not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_EXT_framebuffer_object") )
		WARNING( "Framebuffer objects not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_EXT_packed_depth_stencil") )
		WARNING( "GL_EXT_packed_depth_stencil not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_ARB_vertex_buffer_object") )
		WARNING( "Vertex buffer objects not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_ARB_texture_non_power_of_two") )
		WARNING( "Textures of non power of two not supported. The application may crash (and burn)" );

	if( !glewIsSupported("GL_ARB_vertex_buffer_object") )
		WARNING( "Vertex Buffer Objects not supported. The application may crash (and burn)" );

	w = App::windowW /* * renderingQuality*/;
	h = App::windowH /* * renderingQuality*/;
	aspectRatio = float(w)/h;

	glClearColor( 0.1, 0.1, 0.1, 0.0 );
	glClearDepth( 1.0 );
	glClearStencil( 0 );
	glDepthFunc( GL_LEQUAL );

	// query for maxColorAtachments
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorAtachments );

	// get max texture units
	glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits );

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
	shdr_final = rsrc::shaders.load( "shaders/final.glsl" );

	// init deferred stages
	// WARNING: the order of the inits is crucial!!!!!
	r::ms::init();
	r::is::init();
	r::bs::init();
	r::pps::init();
	r::bs::init2();
	r::dbg::init();

	PRINT( "Renderer initialization ends" );
}


/*
=======================================================================================================================================
render                                                                                                                                =
=======================================================================================================================================
*/
void render( const Camera& cam )
{
	r::ms::runStage( cam );
	r::is::runStage( cam );
	r::bs::runStage( cam );
	r::pps::runStage( cam );
	r::bs::runStage2( cam );
	r::dbg::runStage( cam );

	//r::setViewport( 0, 0, App::windowW, App::windowH );
	r::setViewport( 0, 0, App::windowW, App::windowH );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	shdr_final->bind();
	shdr_final->locTexUnit( shdr_final->GetUniLoc(0), r::pps::fai, 0 );

	/*const int step = 100;
	if( r::framesNum < step )
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::ms::diffuseFai, 0 );
	else if( r::framesNum < step*2 )
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::ms::normalFai, 0 );
	else if( r::framesNum < step*3 )
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::ms::specularFai, 0 );
	else if( r::framesNum < step*4 )
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::ms::depthFai, 0 );
	else if( r::framesNum < step*5 )
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::pps::ssao::bluredFai, 0 );
	else if( r::framesNum < step*6 )
	{
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::pps::hdr::pass2Fai, 0 );
	}
	else
		shdr_final->locTexUnit( shdr_final->getUniLoc(0), r::pps::fai, 0 );*/


	r::DrawQuad( shdr_final->getAttribLoc(0) );
}


/*
=======================================================================================================================================
setProjectionMatrix                                                                                                                   =
=======================================================================================================================================
*/
void setProjectionMatrix( const Camera& cam )
{
	glMatrixMode( GL_PROJECTION );
	loadMatrix( cam.getProjectionMatrix() );
}


/*
=======================================================================================================================================
setViewMatrix                                                                                                                         =
=======================================================================================================================================
*/
void setViewMatrix( const Camera& cam )
{
	glMatrixMode( GL_MODELVIEW );
	loadMatrix( cam.getViewMatrix() );
}


/*
=======================================================================================================================================
unproject                                                                                                                             =
my version of gluUnproject                                                                                                            =
=======================================================================================================================================
*/
bool unproject( float winX, float winY, float winZ, // window screen coords
                const Mat4& modelview_mat, const Mat4& projection_mat, const int* view,
                float& objX, float& objY, float& objZ )
{
	Mat4 inv_pm = projection_mat * modelview_mat;
	inv_pm.invert();

	// the vec is in ndc space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	Vec4 vec;
	vec.x = (2.0*(winX-view[0]))/view[2] - 1.0;
	vec.y = (2.0*(winY-view[1]))/view[3] - 1.0;
	vec.z = 2.0*winZ - 1.0;
	vec.w = 1.0;

	Vec4 final = inv_pm * vec;
	final /= final.w;
	objX = final.x;
	objY = final.y;
	objZ = final.z;
	return true;
}


/*
=======================================================================================================================================
ortho                                                                                                                                 =
=======================================================================================================================================
*/
Mat4 ortho( float left, float right, float bottom, float top, float near, float far )
{
	float difx = right-left;
	float dify = top-bottom;
	float difz = far-near;
	float tx = -(right+left) / difx;
	float ty = -(top+bottom) / dify;
	float tz = -(far+near) / difz;
	Mat4 m;

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
prepareNextFrame                                                                                                                      =
=======================================================================================================================================
*/
void prepareNextFrame()
{
	framesNum++;
}


/*
=======================================================================================================================================
printLastError                                                                                                                        =
=======================================================================================================================================
*/
void printLastError()
{
	GLenum errid = glGetError();
	if( errid != GL_NO_ERROR )
		ERROR( "OpenGL Error: " << gluErrorString( errid ) );
}


//=====================================================================================================================================
// getLastError                                                                                                                       =
//=====================================================================================================================================
const uchar* getLastError()
{
	return gluErrorString( glGetError() );
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
	jpeg_set_quality ( &cinfo, screenshotJpegQuality, true );
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
	fclose( outfile );
	return true;
}


/*
=======================================================================================================================================
takeScreenshot                                                                                                                        =
=======================================================================================================================================
*/
void takeScreenshot( const char* filename )
{
	char* ext = Util::getFileExtension( filename );
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
	else PRINT( "Screenshot \"" << filename << "\" saved" );
}


} // end namespace

