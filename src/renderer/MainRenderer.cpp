#include "anki/renderer/MainRenderer.h"
#include "anki/core/App.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/Filesystem.h"
#include <cstdlib>
#include <cstdio>
#include <jpeglib.h>
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

	sProg.load("shaders/Final.glsl");

	dbgTq.reset(new Query(GL_TIME_ELAPSED));

	// init the offscreen Renderer
	//
	RendererInitializer initializer = initializer_;
	renderingQuality = initializer.mainRendererQuality;
	initializer.width = AppSingleton::get().getWindowWidth() *
		renderingQuality;
	initializer.height = AppSingleton::get().getWindowHeight() *
		renderingQuality;
	Renderer::init(initializer);
	dbg.init(initializer);
	deformer.reset(new Deformer);
	ANKI_LOGI("Main renderer initialized");
}

//==============================================================================
void MainRenderer::initGl()
{
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		throw ANKI_EXCEPTION("GLEW initialization failed");
	}

	// Ignore the first error
	glGetError();

	// print GL info
	ANKI_LOGI("OpenGL info: OGL "
		<< reinterpret_cast<const char*>(glGetString(GL_VERSION))
		<< ", GLSL " << reinterpret_cast<const char*>(
		glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// get max texture units
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDepthFunc(GL_LEQUAL);
	// CullFace is always on
	glCullFace(GL_BACK);
	GlStateSingleton::get().enable(GL_CULL_FACE);

	// defaults
	//glDisable(GL_LIGHTING);
	//glDisable(GL_TEXTURE_2D);
	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	try
	{
		ANKI_CHECK_GL_ERROR();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("OpenGL initialization failed") << e;
	}
}

//==============================================================================
void MainRenderer::render(Scene& scene)
{
	Renderer::render(scene);

	if(getStagesProfilingEnabled())
	{
		dbgTq->begin();
		dbg.run();
		dbgTq->end();
	}
	else
	{
		dbg.run();
	}

	// Render the PPS FAI to the framebuffer
	//
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the window framebuffer

	GlStateSingleton::get().setViewport(
		0, 0, AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight());
	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);
	sProg->bind();
	const Texture& finalFai = ms.getDiffuseFai();
	sProg->findUniformVariableByName("rasterImage")->set(finalFai);
	drawQuad();
}

//==============================================================================
void MainRenderer::takeScreenshotTga(const char* filename)
{
	// open file and check
	std::fstream fs;
	fs.open(filename, std::ios::out | std::ios::binary);
	if(!fs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot create screenshot. File \""
			+ filename + "\"");
	}

	// write headers
	unsigned char tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0,
		0, 0, 0, 0};
	unsigned char header[6];

	header[1] = getWidth() / 256;
	header[0] = getWidth() % 256;
	header[3] = getHeight() / 256;
	header[2] = getHeight() % 256;
	header[4] = 24;
	header[5] = 0;

	fs.write((char*)tgaHeaderUncompressed, 12);
	fs.write((char*)header, 6);

	// write the buffer
	char* buffer = (char*)calloc(getWidth()*getHeight()*3, sizeof(char));

	glReadPixels(0, 0, getWidth(), getHeight(), GL_BGR, GL_UNSIGNED_BYTE,
		buffer);
	fs.write(buffer, getWidth()*getHeight()*3);

	// end
	fs.close();
	free(buffer);
}

//==============================================================================
void MainRenderer::takeScreenshotJpeg(const char* filename)
{
	// open file
	FILE* outfile = fopen(filename, "wb");

	if(!outfile)
	{
		throw ANKI_EXCEPTION("Cannot open file \"" + filename + "\"");
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
}

//==============================================================================
void MainRenderer::takeScreenshot(const char* filename)
{
	std::string ext = getFileExtension(filename);

	// exec from this extension
	if(ext == ".tga")
	{
		takeScreenshotTga(filename);
	}
	else if(ext == ".jpg" || ext == ".jpeg")
	{
		takeScreenshotJpeg(filename);
	}
	else
	{
		throw ANKI_EXCEPTION("Unsupported file extension: " + filename);
	}
	//ANKI_LOGI("Screenshot \"" << filename << "\" saved");
}

} // end namespace
