#ifndef R_MAIN_RENDERER_H
#define R_MAIN_RENDERER_H

#include "Renderer.h"
#include "Util/Singleton.h"


namespace R {


/// Main onscreen renderer
class MainRenderer: public Renderer
{
	public:
		MainRenderer();

		~MainRenderer();

		/// @name Setters & getters
		/// @{
		GETTER_SETTER_BY_VAL(int, screenshotJpegQuality,
			getScreenshotJpegQuality, setScreenshotJpegQuality)
		GETTER_R_BY_VAL(float, renderingQuality, getRenderingQuality)
		GETTER_RW(Dbg, dbg, getDbg)
		GETTER_R_BY_VAL(double, dbgTime, getDbgTime)
		/// @}

		/// The same as Renderer::init but with additional initialization.
		/// @see Renderer::init
		void init(const RendererInitializer& initializer);

		/// The same as Renderer::render but in addition it renders the final
		/// FAI to the framebuffer
		/// @param cam @see Renderer::render
		void render(Camera& cam);

		/// Save the color buffer to a tga (lossless & uncompressed & slow)
		/// or jpeg (lossy & compressed fast)
		/// @param filename The file to save
		void takeScreenshot(const char* filename);

	private:
		/// @name Passes
		/// @{
		Dbg dbg; ///< Debugging rendering stage. Only the main renderer has it
		/// @}

		/// @name Profiling stuff
		/// @{
		double dbgTime;
		boost::scoped_ptr<TimeQuery> dbgTq;
		/// @}

		RsrcPtr<ShaderProg> sProg; ///< Final pass' shader program
		int screenshotJpegQuality; ///< The quality of the JPEG screenshots.
		                           ///< From 0 to 100

		/// The global rendering quality of the raster image. Its a percentage
		/// of the application's window size. From 0.0(low) to 1.0(high)
		float renderingQuality;

		void takeScreenshotTga(const char* filename);
		void takeScreenshotJpeg(const char* filename);
		static void initGl();
};


inline MainRenderer::MainRenderer():
	dbg(*this),
	screenshotJpegQuality(90)
{}


} // end namespace


#endif
