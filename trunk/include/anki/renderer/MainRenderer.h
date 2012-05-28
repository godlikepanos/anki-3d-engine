#ifndef ANKI_RENDERER_MAIN_RENDERER_H
#define ANKI_RENDERER_MAIN_RENDERER_H

#include "anki/renderer/Renderer.h"
#include <boost/scoped_ptr.hpp>
#include <GL/glew.h>


namespace anki {


class Deformer;


/// Main onscreen renderer
class MainRenderer: public Renderer
{
	public:
		MainRenderer();

		~MainRenderer();

		/// @name Setters & getters
		/// @{
		int getSceenshotJpegQuality() const
		{
			return screenshotJpegQuality;
		}
		int& getSceenshotJpegQuality()
		{
			return screenshotJpegQuality;
		}
		void setSceenshotJpegQuality(const int x)
		{
			screenshotJpegQuality = x;
		}

		float getRenderingQuality() const
		{
			return renderingQuality;
		}

		const Dbg& getDbg() const
		{
			return dbg;
		}
		Dbg& getDbg()
		{
			return dbg;
		}

		double getDbgTime() const
		{
			return dbgTime;
		}
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
		boost::scoped_ptr<Query> dbgTq;
		/// @}

		ShaderProgramResourcePointer sProg; ///< Final pass' shader program
		int screenshotJpegQuality; ///< The quality of the JPEG screenshots.
		                           ///< From 0 to 100

		/// The global rendering quality of the raster image. Its a percentage
		/// of the application's window size. From 0.0(low) to 1.0(high)
		float renderingQuality;

		boost::scoped_ptr<Deformer> deformer;

		GLEWContext glContext;

		void takeScreenshotTga(const char* filename);
		void takeScreenshotJpeg(const char* filename);
		void initGl();
};


} // end namespace


#endif
