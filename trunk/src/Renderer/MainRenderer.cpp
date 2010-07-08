#include <cstdlib>
#include <cstdio>
#include <jpeglib.h>
#include <fstream>
#include "MainRenderer.h"
#include "App.h"
#include "RendererInitializer.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void MainRenderer::init(const RendererInitializer& initializer_)
{
	INFO("Main renderer initializing...");

	initGl();

	sProg = Resource::shaders.load("shaders/final.glsl");

	//
	// init the offscreen Renderer
	//
	RendererInitializer initializer = initializer_;
	renderingQuality = initializer.mainRendererQuality;
	initializer.width = app->getWindowWidth() * renderingQuality;
	initializer.height = app->getWindowHeight() * renderingQuality;
	Renderer::init(initializer);

	INFO("Main renderer initialization ends");
}


//======================================================================================================================
// initGl                                                                                                              =
//======================================================================================================================
void MainRenderer::initGl()
{
	GLenum err = glewInit();
	if(err != GLEW_OK)
		FATAL("GLEW initialization failed");

	// print GL info
	INFO("OpenGL info: OGL " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(!glewIsSupported("GL_VERSION_3_1"))
		WARNING("OpenGL ver 3.1 not supported. The application may crash (and burn)");

	// get max texture units
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &Texture::textureUnitsNum);

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDepthFunc(GL_LEQUAL);
	// CullFace is always on
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	// defaults
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDepthMask(true);
	glDepthFunc(GL_LESS);

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAtachments);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void MainRenderer::render(Camera& cam_)
{
	Renderer::render(cam_);

	//
	// Render the PPS FAI to the framebuffer
	//
	Fbo::unbind();
	setViewport(0, 0, app->getWindowWidth(), app->getWindowHeight());
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	sProg->bind();
	//sProg->findUniVar("rasterImage")->setTexture(is.fai, 0);
	sProg->findUniVar("rasterImage")->setTexture(pps.postPassFai, 0);
	drawQuad(0);
}


//======================================================================================================================
// takeScreenshotTga                                                                                                   =
//======================================================================================================================
bool MainRenderer::takeScreenshotTga(const char* filename)
{
	// open file and check
	fstream fs;
	fs.open(filename, ios::out|ios::binary);
	if(!fs.good())
	{
		ERROR("Cannot create screenshot. File \"" << filename << "\"");
		return false;
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
	return true;
}


//======================================================================================================================
// takeScreenshotJpeg                                                                                                  =
//======================================================================================================================
bool MainRenderer::takeScreenshotJpeg(const char* filename)
{
	// open file
	FILE* outfile = fopen(filename, "wb");

	if(!outfile)
	{
		ERROR("Cannot open file \"" << filename << "\"");
		return false;
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
	return true;
}


//======================================================================================================================
// takeScreenshot                                                                                                      =
//======================================================================================================================
void MainRenderer::takeScreenshot(const char* filename)
{
	string ext = Util::getFileExtension(filename);
	bool ret;

	// exec from this extension
	if(ext == "tga")
	{
		ret = takeScreenshotTga(filename);
	}
	else if(ext == "jpg" || ext == "jpeg")
	{
		ret = takeScreenshotJpeg(filename);
	}
	else
	{
		ERROR("File \"" << filename << "\": Unsupported extension. Watch for capital");
		return;
	}

	if(!ret)
		ERROR("In taking screenshot");
	else
		INFO("Screenshot \"" << filename << "\" saved");
}

