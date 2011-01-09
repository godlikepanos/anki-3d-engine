#ifndef MAIN_RENDERER_H
#define MAIN_RENDERER_H

#include "Renderer.h"
#include "Singleton.h"


/// Main onscreen renderer
class MainRenderer: public Renderer
{
	public:
		MainRenderer();

		~MainRenderer() throw() {}

		/// @name Setters & getters
		/// @{
		int& getScreenshotJpegQuality() {return screenshotJpegQuality;}
		void setScreenshotJpegQuality(int i) {screenshotJpegQuality = i;}
		float getRenderingQuality() const {return renderingQuality;}
		Dbg& getDbg() {return dbg;}
		/// @}

		/// The same as Renderer::init but with additional initialization. @see Renderer::init
		void init(const RendererInitializer& initializer);

		/// The same as Renderer::render but in addition it renders the final FAI to the framebuffer
		/// @param cam @see Renderer::render
		void render(Camera& cam);

		/// Save the color buffer to a tga (lossless & uncompressed & slow) or jpeg (lossy & compressed/// fast)
		/// @param filename The file to save
		void takeScreenshot(const char* filename);

	private:
		/// @name Passes
		/// @{
		Dbg dbg; ///< Debugging rendering stage. Only the main renderer has it
		/// @}

		RsrcPtr<ShaderProg> sProg; ///< Final pass' shader program
		int screenshotJpegQuality; ///< The quality of the JPEG screenshots. From 0 to 100

		/// The global rendering quality of the raster image. Its a percentage of the application's window size. From
		/// 0.0(low) to 1.0(high)
		float renderingQuality;

		void takeScreenshotTga(const char* filename);
		void takeScreenshotJpeg(const char* filename);
		static void initGl();
};


inline MainRenderer::MainRenderer():
	dbg(*this),
	screenshotJpegQuality(90)
{}


typedef Singleton<MainRenderer> MainRendererSingleton; ///< Make the MainRenderer singleton class


#endif
