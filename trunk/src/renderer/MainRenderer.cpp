#include "anki/renderer/MainRenderer.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/File.h"
#include "anki/core/Counters.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>

#define glewGetContext() (&glContext)

namespace anki {

//==============================================================================
MainRenderer::~MainRenderer()
{}

//==============================================================================
void MainRenderer::init(const Renderer::Initializer& initializer_)
{
	ANKI_LOGI("Initializing main renderer...");
	initGl();

	renderingQuality = initializer_.renderingQuality;
	windowWidth = initializer_.width;
	windowHeight = initializer_.height;

	// init the offscreen Renderer
	//
	RendererInitializer initializer = initializer_;
	initializer.width *= renderingQuality;
	initializer.height *= renderingQuality;

	sProg.load("shaders/Final.glsl");

	Renderer::init(initializer);
	dbg.init(initializer);
	deformer.reset(new Deformer);

	ANKI_LOGI("Main renderer initialized");
}

//==============================================================================
void MainRenderer::initGl()
{
	// Ignore the first error
	glGetError();

	// print GL info
	ANKI_LOGI("OpenGL info: OGL "
		<< reinterpret_cast<const char*>(glGetString(GL_VERSION))
		<< ", GLSL " << reinterpret_cast<const char*>(
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

	glEnable(GL_FRAMEBUFFER_SRGB);

	glDisable(GL_DITHER);

	// Check for error
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void MainRenderer::render(SceneGraph& scene)
{
	ANKI_COUNTER_START_TIMER(C_MAIN_RENDERER_TIME);

	Bool drawToDefaultFbo = renderingQuality > 0.9 && !dbg.getEnabled();

	pps.setDrawToDefaultFbo(drawToDefaultFbo);

	Renderer::render(scene);

	if(dbg.getEnabled())
	{
		dbg.run();
	}

	// Render the PPS FAI to the framebuffer
	//
	if(!drawToDefaultFbo)
	{
		Fbo::bindDefault(); // Bind the window framebuffer

		GlStateSingleton::get().setViewport(0, 0, windowWidth, windowHeight);
		GlStateSingleton::get().disable(GL_DEPTH_TEST);
		GlStateSingleton::get().disable(GL_BLEND);
		sProg->bind();
#if 0
		const Texture& finalFai = pps.getLf().fai;
#else
		const Texture& finalFai = pps.getFai();
#endif
		sProg->findUniformVariable("rasterImage").set(finalFai);
		drawQuad();
	}

	// Check for error
	ANKI_CHECK_GL_ERROR();

	ANKI_COUNTER_STOP_TIMER_INC(C_MAIN_RENDERER_TIME);
}

//==============================================================================
void MainRenderer::takeScreenshotTga(const char* filename)
{
	// open file and check
	std::fstream fs;
	fs.open(filename, std::ios::out | std::ios::binary);
	if(!fs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot write screenshot file:"
			+ filename);
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
	fs.close();
	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void MainRenderer::takeScreenshotJpeg(const char* filename)
{
#if 0
	// open file
	FILE* outfile = fopen(filename, "wb");

	if(!outfile)
	{
		throw ANKI_EXCEPTION("Cannot open file: " + filename);
	}

	// set jpg params
	jpeg_compress_struct cinfo;
	jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = getWidth();
	cinfo.image_height     = getHeight();
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality (&cinfo, screenshotJpegQuality, true);
	jpeg_start_compress(&cinfo, true);

	// read from OGL
	char* buffer = (char*)malloc(getWidth()*getHeight()*3*sizeof(char));
	glReadPixels(0, 0, getWidth(), getHeight(), GL_RGB, GL_UNSIGNED_BYTE,
		buffer);

	// write buffer to file
	JSAMPROW row_pointer;

	while(cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW)&buffer[(getHeight() - 1 -
			cinfo.next_scanline) * 3 * getWidth()];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	// done
	free(buffer);
	fclose(outfile);
#endif
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
	else if(ext == "jpg" || ext == "jpeg")
	{
		takeScreenshotJpeg(filename);
	}
	else
	{
		throw ANKI_EXCEPTION("Unsupported file extension: " + filename);
	}
	//ANKI_LOGI("Screenshot \"" << filename << "\" saved");
}

} // end namespace anki
