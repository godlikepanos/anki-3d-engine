#ifndef ANKI_RENDERER_MAIN_RENDERER_H
#define ANKI_RENDERER_MAIN_RENDERER_H

#include "anki/renderer/Renderer.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/Singleton.h"

namespace anki {

/// Main onscreen renderer
class MainRenderer: public Renderer
{
public:
	MainRenderer()
		: dbg(this)
	{}

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

	const Dbg& getDbg() const
	{
		return dbg;
	}
	Dbg& getDbg()
	{
		return dbg;
	}
	/// @}

	/// The same as Renderer::init but with additional initialization.
	/// @see Renderer::init
	void init(const RendererInitializer& initializer);

	/// The same as Renderer::render but in addition it renders the final
	/// FAI to the framebuffer
	/// @param scene @see Renderer::render
	void render(Scene& scene);

	/// Save the color buffer to a tga (lossless & uncompressed & slow)
	/// or jpeg (lossy & compressed fast)
	/// @param filename The file to save
	void takeScreenshot(const char* filename);

private:
	/// @name Passes
	/// @{
	Dbg dbg; ///< Debugging rendering stage. Only the main renderer has it
	/// @}

	ShaderProgramResourcePointer sProg; ///< Final pass' shader program
	int screenshotJpegQuality = 90; ///< The quality of the JPEG screenshots.
							        ///< From 0 to 100

	U32 windowWidth, windowHeight;

	Bool drawToDefaultFbo;

	std::unique_ptr<Deformer> deformer;

	void takeScreenshotTga(const char* filename);
	void takeScreenshotJpeg(const char* filename);
	void initGl();
};

typedef Singleton<MainRenderer> MainRendererSingleton;

} // end namespace anki

#endif
