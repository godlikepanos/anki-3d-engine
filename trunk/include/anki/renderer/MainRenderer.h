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
	{}

	~MainRenderer();

	/// The same as Renderer::init but with additional initialization.
	/// @see Renderer::init
	void init(const RendererInitializer& initializer);

	/// Save the color buffer to a tga (lossless & uncompressed & slow)
	/// or jpeg (lossy & compressed fast)
	/// @param filename The file to save
	void takeScreenshot(const char* filename);

private:
	std::unique_ptr<Deformer> deformer;

	void takeScreenshotTga(const char* filename);
	void initGl();
};

typedef Singleton<MainRenderer> MainRendererSingleton;

} // end namespace anki

#endif
