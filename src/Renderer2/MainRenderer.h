#ifndef _MAINRENDERER_H_
#define _MAINRENDERER_H_

#include "Common.h"
#include "Renderer.hpp"


/**
 * Main renderer
 */
class MainRenderer: public Renderer
{
	PROPERTY_RW( int, screenshotJpegQuality, setScreenshotJpegQuality, getScreenshotJpegQuality ) ///< The quality of the JPEG screenshots. From 0 to 100
	PROPERTY_R( int, maxColorAtachments, getMaxColorAtachments ) ///< Max color attachments a FBO can accept

	private:
		ShaderProg sProg; ///< Final pass' shader program

		bool takeScreenshotTga( const char* filename );
		bool takeScreenshotJpeg( const char* filename );

	public:
		MainRenderer(): screenshotJpegQuality( 90 ) {}

		void init();
		void render( Camera& cam );
		void takeScreenshot( const char* filename ); ///< Save the colorbuffer as 24bit uncompressed TGA image
};

#endif
