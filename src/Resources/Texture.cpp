#include <GL/glew.h>
#include "Texture.h"
#include "Renderer.h"
#include "Image.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================
int Texture::textureUnitsNum = -1;
bool Texture::mipmappingEnabled = true;
bool Texture::compressionEnabled = false;
int Texture::anisotropyLevel = 8;


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Texture::Texture():
	Resource(RT_TEXTURE),
	glId(numeric_limits<uint>::max()),
	target(GL_TEXTURE_2D)
{
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Texture::load(const char* filename)
{
	target = GL_TEXTURE_2D;
	if(glId != numeric_limits<uint>::max())
	{
		ERROR("Texture already loaded");
		return false;
	}

	Image img;
	if(!img.load(filename)) return false;


	// bind the texture
	glGenTextures(1, &glId);
	bind(0);
	if(mipmappingEnabled)
	{
		setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, float(anisotropyLevel));

	// leave to GL_REPEAT. There is not real performance impact
	setRepeat(true);

	int format = (img.bpp==32) ? GL_RGBA : GL_RGB;

	int intFormat; // the internal format of the image
	if(compressionEnabled)
	{
		//int_format = (img.bpp==32) ? GL_COMPRESSED_RGBA_S3TC_DXT1 : GL_COMPRESSED_RGB_S3TC_DXT1;
		intFormat = (img.bpp==32) ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
	}
	else
	{
		intFormat = (img.bpp==32) ? GL_RGBA : GL_RGB;
	}

	glTexImage2D(target, 0, intFormat, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.data);
	if(mipmappingEnabled)
	{
		glGenerateMipmap(target);
	}

	img.unload();
	return true;
}


//======================================================================================================================
// createEmpty2D                                                                                                       =
//======================================================================================================================
bool Texture::createEmpty2D(float width_, float height_, int internalFormat, int format_, GLenum type_,
                            bool mimapping)
{
	target = GL_TEXTURE_2D;
	DEBUG_ERR(internalFormat>0 && internalFormat<=4); // deprecated internal format
	DEBUG_ERR(glId != numeric_limits<uint>::max()); // Texture already loaded

	// GL stuff
	glGenTextures(1, &glId);
	bind();

	if(mimapping)
		setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setRepeat(true);


	// allocate to vram
	glTexImage2D(target, 0, internalFormat, width_, height_, 0, format_, type_, NULL);

	if(mimapping)
		glGenerateMipmap(target);

	return GL_OK();
}


//======================================================================================================================
// createEmpty2DMSAA                                                                                                   =
//======================================================================================================================
bool Texture::createEmpty2DMsaa(int samplesNum, int internalFormat, int width_, int height_, bool mimapping)
{
	target = GL_TEXTURE_2D_MULTISAMPLE;
	DEBUG_ERR(glId != numeric_limits<uint>::max()); // Texture already loaded

	glGenTextures(1, &glId);
	bind();
	
	// allocate
	glTexImage2DMultisample(target, samplesNum, internalFormat, width_, height_, false);

	if(mimapping)
		glGenerateMipmap(target);

	return GL_OK();
}


//======================================================================================================================
// unload                                                                                                              =
//======================================================================================================================
void Texture::unload()
{
	glDeleteTextures(1, &glId);
	INFO("ASFD");
}


//======================================================================================================================
// bind                                                                                                                =
//======================================================================================================================
void Texture::bind(uint unit) const
{
	if(unit >= static_cast<uint>(textureUnitsNum))
		WARNING("Max tex units passed");

	glActiveTexture(GL_TEXTURE0+unit);
	glBindTexture(target, glId);
}


//======================================================================================================================
// getWidth                                                                                                            =
//======================================================================================================================
int Texture::getWidth() const
{
	bind();
	int i;
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &i);
	return i;
}


//======================================================================================================================
// getHeight                                                                                                           =
//======================================================================================================================
int Texture::getHeight() const
{
	bind();
	int i;
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &i);
	return i;
}


//======================================================================================================================
// setTexParameter [int]                                                                                                  =
//======================================================================================================================
void Texture::setTexParameter(GLenum paramName, GLint value) const
{
	bind();
	glTexParameteri(target, paramName, value);
}


//======================================================================================================================
// setTexParameter [float]                                                                                                =
//======================================================================================================================
void Texture::texParameter(GLenum paramName, GLfloat value) const
{
	bind();
	glTexParameterf(target, paramName, value);
}

