#include <cstdlib>
#include <cstdio>
#include <jpeglib.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "GfxApi/GlException.h"
#include "MainRenderer.h"
#include "Core/App.h"
#include "RendererInitializer.h"
#include "Ssao.h"
#include "Core/Logger.h"


//==============================================================================
// init                                                                        =
//==============================================================================
void MainRenderer::init(const RendererInitializer& initializer_)
{
	INFO("Initializing main renderer...");
	initGl();

	sProg.loadRsrc("shaders/Final.glsl");

	//
	// init the offscreen Renderer
	//
	RendererInitializer initializer = initializer_;
	renderingQuality = initializer.mainRendererQuality;
	initializer.width = AppSingleton::getInstance().getWindowWidth() * renderingQuality;
	initializer.height = AppSingleton::getInstance().getWindowHeight() * renderingQuality;
	Renderer::init(initializer);
	dbg.init(initializer);
	INFO("Main renderer initialized");
}


//==============================================================================
// initGl                                                                      =
//==============================================================================
void MainRenderer::initGl()
{
	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		throw EXCEPTION("GLEW initialization failed");
	}

	// Ignore re first error
	glGetError();

	// print GL info
	INFO("OpenGL info: OGL " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) <<
	     ", GLSL " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// get max texture units
	//glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAtachments);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &Texture::textureUnitsNum);
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDepthFunc(GL_LEQUAL);
	// CullFace is always on
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// defaults
	//glDisable(GL_LIGHTING);
	//glDisable(GL_TEXTURE_2D);
	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);
	glDisable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(true);
	glDepthFunc(GL_LESS);

	try
	{
		ON_GL_FAIL_THROW_EXCEPTION();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("OpenGL initialization failed: " + e.what());
	}
}


//==============================================================================
// render                                                                      =
//==============================================================================
void MainRenderer::render(Camera& cam_)
{
	Renderer::render(cam_);
	dbg.run();

	//
	// Render the PPS FAI to the framebuffer
	//
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the window framebuffer

	setViewport(0, 0, AppSingleton::getInstance().getWindowWidth(), AppSingleton::getInstance().getWindowHeight());
	GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, false);
	GlStateMachineSingleton::getInstance().enable(GL_BLEND, false);
	sProg->bind();
	//sProg->findUniVar("rasterImage")->set(ms.getNormalFai(), 0);
	//sProg->findUniVar("rasterImage")->set(pps.getSsao().getFai(), 0);
	sProg->findUniVar("rasterImage")->set(pps.getPostPassFai(), 0);
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
		throw EXCEPTION("Cannot create screenshot. File \"" + filename + "\"");
	}

	// write headers
	unsigned char tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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

	glReadPixels(0, 0, getWidth(), getHeight(), GL_BGR, GL_UNSIGNED_BYTE, buffer);
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
		throw EXCEPTION("Cannot open file \"" + filename + "\"");
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
	glReadPixels(0, 0, getWidth(), getHeight(), GL_RGB, GL_UNSIGNED_BYTE, buffer);

	// write buffer to file
	JSAMPROW row_pointer;

	while(cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW) &buffer[(getHeight()-1-cinfo.next_scanline)*3*getWidth()];
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
		throw EXCEPTION("File \"" + filename + "\": Unsupported extension");
	}
	//INFO("Screenshot \"" << filename << "\" saved");
}

