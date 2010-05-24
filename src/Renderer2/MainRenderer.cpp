#include <cstdlib>
#include <cstdio>
#include <jpeglib.h>
#include "MainRenderer.h"
#include "App.h"


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void MainRenderer::init()
{
	INFO( "Main renderer initializing..." );

	GLenum err = glewInit();
	if( err != GLEW_OK )
		FATAL( "GLEW initialization failed" );

	// print GL info
	INFO( "OpenGL info: OGL " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) );

	if( !glewIsSupported("GL_VERSION_3_1") )
		WARNING( "OpenGL ver 3.1 not supported. The application may crash (and burn)" );

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

	//
	// Set default OpenGL
	//
	glClearColor( 0.1, 0.1, 0.1, 0.0 );
	glClearDepth( 1.0 );
	glClearStencil( 0 );
	glDepthFunc( GL_LEQUAL );
	// CullFace is always on
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	// defaults
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glPolygonMode( GL_FRONT, GL_FILL );

	//
	// init the rest
	//
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorAtachments );
	Renderer::init();
	sProg.customLoad( "shaders/final.glsl" );

	INFO( "Main renderer initialization ends" );
}


//=====================================================================================================================================
//                                                                                                                                    =
//=====================================================================================================================================
void MainRenderer::render( Camera& cam_ )
{
	Renderer::render( cam_ );

	//
	// Render the PPS FAI to the framebuffer
	//
	setViewport( 0, 0, app->getWindowWidth(), app->getWindowHeight() );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	sProg.bind();
	sProg.locTexUnit( sProg.findUniVar("rasterImage")->getLoc(), pps.fai, 0 );
	drawQuad( 0 );
}


//=====================================================================================================================================
// takeScreenshotTga                                                                                                                  =
//=====================================================================================================================================
bool MainRenderer::takeScreenshotTga( const char* filename )
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
	unsigned char tgaHeaderUncompressed[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
	unsigned char header[6];

	header[1] = width / 256;
	header[0] = width % 256;
	header[3] = height / 256;
	header[2] = height % 256;
	header[4] = 24;
	header[5] = 0;

	fs.write( (char*)tgaHeaderUncompressed, 12 );
	fs.write( (char*)header, 6 );

	// write the buffer
	char* buffer = (char*)calloc( width*height*3, sizeof(char) );

	glReadPixels( 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer );
	fs.write( buffer, width*height*3 );

	// end
	fs.close();
	free( buffer );
	return true;
}


//=====================================================================================================================================
// takeScreenshotJpeg                                                                                                                 =
//=====================================================================================================================================
bool MainRenderer::takeScreenshotJpeg( const char* filename )
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

	cinfo.image_width      = width;
	cinfo.image_height     = height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults( &cinfo);
	jpeg_set_quality ( &cinfo, screenshotJpegQuality, true );
	jpeg_start_compress( &cinfo, true );

	// read from OGL
	char* buffer = (char*)malloc( width*height*3*sizeof(char) );
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer );

	// write buffer to file
	JSAMPROW row_pointer;

	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer = (JSAMPROW) &buffer[ (height-1-cinfo.next_scanline)*3*width ];
		jpeg_write_scanlines( &cinfo, &row_pointer, 1 );
	}

	jpeg_finish_compress(&cinfo);

	// done
	free( buffer );
	fclose( outfile );
	return true;
}


//=====================================================================================================================================
// takeScreenshot                                                                                                                     =
//=====================================================================================================================================
void MainRenderer::takeScreenshot( const char* filename )
{
	char* ext = Util::getFileExtension( filename );
	bool ret;

	// exec from this extension
	if( strcmp( ext, "tga" ) == 0 )
	{
		ret = takeScreenshotTga( filename );
	}
	else if( strcmp( ext, "jpg" ) == 0 )
	{
		ret = takeScreenshotJpeg( filename );
	}
	else
	{
		ERROR( "File \"" << filename << "\": Unsupported extension. Watch for capital" );
		return;
	}
	if( !ret ) ERROR( "In taking screenshot" )
	else PRINT( "Screenshot \"" << filename << "\" saved" );
}

