#include <cstdlib>
#include <cstdio>
#include <jpeglib.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "anki/gl/GlException.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/core/App.h"
#include "anki/renderer/RendererInitializer.h"
#include "anki/renderer/Ssao.h"
#include "anki/core/Logger.h"
#include "anki/gl/TimeQuery.h"
#include "anki/renderer/Deformer.h"


#define glewGetContext() (&glContext)


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================
MainRenderer::MainRenderer():
	dbg(*this),
	screenshotJpegQuality(90)
{}


MainRenderer::~MainRenderer()
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void MainRenderer::init(const RendererInitializer& initializer_)
{
	INFO("Initializing main renderer...");
	initGl();

	sProg.loadRsrc("shaders/Final.glsl");

	dbgTq.reset(new TimeQuery);

	//
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
	deformer.reset(new Deformer(*this));
	INFO("Main renderer initialized");
}


//==============================================================================
// initGl                                                                      =
//==============================================================================
void MainRenderer::initGl()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		throw ANKI_EXCEPTION("GLEW initialization failed");
	}

	// Ignore re first error
	glGetError();

	// print GL info
	INFO("OpenGL info: OGL " <<
		reinterpret_cast<const char*>(glGetString(GL_VERSION)) <<
		", GLSL " << reinterpret_cast<const char*>(
			glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// get max texture units
	//glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAtachments);
	int& tun = Texture::getTextureUnitsNum();
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tun);
	glClearColor(0.1, 0.1, 0.0, 1.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDepthFunc(GL_LEQUAL);
	// CullFace is always on
	glCullFace(GL_BACK);
	GlStateMachineSingleton::get().enable(GL_CULL_FACE);

	// defaults
	//glDisable(GL_LIGHTING);
	//glDisable(GL_TEXTURE_2D);
	GlStateMachineSingleton::get().enable(GL_BLEND, false);
	GlStateMachineSingleton::get().disable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(true);
	glDepthFunc(GL_LESS);

	try
	{
		ON_GL_FAIL_THROW_EXCEPTION();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("OpenGL initialization failed: " + e.what());
	}
}


//==============================================================================
// render                                                                      =
//==============================================================================
void MainRenderer::render(Camera& cam_)
{
	Renderer::render(cam_);

	if(getStagesProfilingEnabled())
	{
		dbgTq->begin();
		dbg.run();
		dbgTime = dbgTq->end();
	}
	else
	{
		dbg.run();
	}

	//
	// Render the PPS FAI to the framebuffer
	//
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the window framebuffer

	GlStateMachineSingleton::get().setViewport(
		0, 0, AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight());
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, false);
	GlStateMachineSingleton::get().enable(GL_BLEND, false);
	sProg->bind();
	//sProg->getUniformVariableByName("rasterImage").set(ms.getDiffuseFai(), 0);
	//sProg->getUniformVariableByName("rasterImage").
	//	set(is.getFai(), 0);
	sProg->getUniformVariableByName("rasterImage").set(pps.getPostPassFai(), 0);
	drawQuad();
}


//==============================================================================
// takeScreenshotTga                                                           =
//==============================================================================
void MainRenderer::takeScreenshotTga(const char* filename)
{
	// open file and check
	std::fstream fs;
	fs.open(filename, std::ios::out | std::ios::binary);
	if(!fs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot create screenshot. File \"" + filename + "\"");
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
// takeScreenshotJpeg                                                          =
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
// takeScreenshot                                                              =
//==============================================================================
void MainRenderer::takeScreenshot(const char* filename)
{
	std::string ext = boost::filesystem::path(filename).extension();
	boost::to_lower(ext);

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
		throw ANKI_EXCEPTION("File \"" + filename + "\": Unsupported extension");
	}
	//INFO("Screenshot \"" << filename << "\" saved");
}

