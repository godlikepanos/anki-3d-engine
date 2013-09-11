#include "anki/renderer/MainRenderer.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/File.h"
#include "anki/core/Counters.h"
#include <cstdlib>
#include <cstdio>

#define glewGetContext() (&glContext)

namespace anki {

//==============================================================================
MainRenderer::~MainRenderer()
{}

//==============================================================================
void MainRenderer::init(const RendererInitializer& initializer_)
{
	ANKI_LOGI("Initializing main renderer...");

	RendererInitializer initializer = initializer_;
	initializer.set("offscreen", false);
	initializer.get("width") *= initializer.get("renderingQuality");
	initializer.get("height") *= initializer.get("renderingQuality");

	initGl();

	blitProg.load("shaders/Final.glsl");

	Renderer::init(initializer);
	deformer.reset(new Deformer);

	ANKI_LOGI("Main renderer initialized. Rendering size %dx%d", 
		getWidth(), getHeight());
}

//==============================================================================
void MainRenderer::render(SceneGraph& scene)
{
	ANKI_COUNTER_START_TIMER(C_MAIN_RENDERER_TIME);
	Renderer::render(scene);

	Bool alreadyDrawn = getRenderingQuality() == 1.0;

	if(!alreadyDrawn)
	{
		Fbo::bindDefault(Fbo::FT_ALL, true); // Bind the window framebuffer

		GlStateSingleton::get().setViewport(
			0, 0, getWindowWidth(), getWindowHeight());
		GlStateSingleton::get().disable(GL_DEPTH_TEST);
		GlStateSingleton::get().disable(GL_BLEND);
		blitProg->bind();

		Texture* fai;

		if(getPps().getEnabled())
		{
			fai = &getPps().getFai();
		}
		else
		{
			fai = &getIs().getFai();
		}

		fai->setFiltering(Texture::TFT_LINEAR);

		blitProg->findUniformVariable("rasterImage").set(*fai);
		drawQuad();
	}
	

	ANKI_COUNTER_STOP_TIMER_INC(C_MAIN_RENDERER_TIME);
}

//==============================================================================
void MainRenderer::initGl()
{
	// Ignore the first error
	glGetError();

	// print GL info
	ANKI_LOGI("OpenGL info: OGL %s, GLSL %s",
		reinterpret_cast<const char*>(glGetString(GL_VERSION)),
		reinterpret_cast<const char*>(
		glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// get max texture units
	GlStateSingleton::get().setClearColor(Vec4(1.0, 0.0, 1.0, 1.0));
	GlStateSingleton::get().setClearDepthValue(1.0);
	GlStateSingleton::get().setClearStencilValue(0);
	// CullFace is always on
	glCullFace(GL_BACK);
	GlStateSingleton::get().enable(GL_CULL_FACE);

	// defaults
	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_STENCIL_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GlStateSingleton::get().setDepthMaskEnabled(true);
	GlStateSingleton::get().setDepthFunc(GL_LESS);

	//glEnable(GL_FRAMEBUFFER_SRGB);

	glDisable(GL_DITHER);

	// Check for error
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void MainRenderer::takeScreenshotTga(const char* filename)
{
	// open file and check
	File fs;
	try
	{
		fs.open(filename, File::OF_WRITE | File::OF_BINARY);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot write screenshot file:"
			+ filename) << e;
	}

	// write headers
	static const U8 tgaHeaderUncompressed[12] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	Array<U8, 6> header;

	header[1] = getWidth() / 256;
	header[0] = getWidth() % 256;
	header[3] = getHeight() / 256;
	header[2] = getHeight() % 256;
	header[4] = 24;
	header[5] = 0;

	fs.write((char*)tgaHeaderUncompressed, sizeof(tgaHeaderUncompressed));
	fs.write((char*)&header[0], sizeof(header));

	// Create the buffers
	U outBufferSize = getWidth() * getHeight() * 3;
	Vector<U8> outBuffer;
	outBuffer.resize(outBufferSize);

	U rgbaBufferSize = getWidth() * getHeight() * 4;
	Vector<U8> rgbaBuffer;
	rgbaBuffer.resize(rgbaBufferSize);

	// Read pixels
	glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		&rgbaBuffer[0]);

	U j = 0;
	for(U i = 0; i < outBufferSize; i += 3)
	{
		outBuffer[i] = rgbaBuffer[j + 2];
		outBuffer[i + 1] = rgbaBuffer[j + 1];
		outBuffer[i + 2] = rgbaBuffer[j];

		j += 4;
	}

	fs.write((char*)&outBuffer[0], outBufferSize);

	// end
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void MainRenderer::takeScreenshot(const char* filename)
{
	std::string ext = File::getFileExtension(filename);

	// exec from this extension
	if(ext == "tga")
	{
		takeScreenshotTga(filename);
	}
	else
	{
		throw ANKI_EXCEPTION("Unsupported file extension: " + filename);
	}
	//ANKI_LOGI("Screenshot \"" << filename << "\" saved");
}

} // end namespace anki
