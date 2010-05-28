#ifndef _MAINRENDERER_H_
#define _MAINRENDERER_H_

#include "Common.h"
#include "Renderer.hpp"


/**
 * Main onscreen renderer
 */
class MainRenderer: public Renderer
{
	PROPERTY_RW( int, screenshotJpegQuality, setScreenshotJpegQuality, getScreenshotJpegQuality ) ///< The quality of the JPEG screenshots. From 0 to 100
	PROPERTY_R( int, maxColorAtachments, getMaxColorAtachments ) ///< Max color attachments a FBO can accept

	/**
	 * The global rendering quality of the raster image. Its a percentage of the application's window size. From 0.0(low) to 1.0(high)
	 */
	PROPERTY_R( float, renderingQuality, getRenderingQuality )

	private:
		ShaderProg sProg; ///< Final pass' shader program

		bool takeScreenshotTga( const char* filename );
		bool takeScreenshotJpeg( const char* filename );

	public:
		MainRenderer(): screenshotJpegQuality( 90 ) {}

		/**
		 * The same as Renderer::init but with additional initialization. @see Renderer::init
		 */
		void init( const RendererInitializer& initializer );

		/**
		 * The same as Renderer::render but in addition it renders the final FAI to the framebuffer
		 * @param cam @see Renderer::render
		 */
		void render( Camera& cam );

		/**
		 * Save the color buffer to a tga (lossless & uncompressed & slow) or jpeg (lossy & compressed * fast)
		 * @param filename The file to save
		 */
		void takeScreenshot( const char* filename ); ///< Save the colorbuffer as 24bit uncompressed TGA image
};

#endif
