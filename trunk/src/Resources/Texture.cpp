#include <GL/glew.h>
#include "Texture.h"
#include "Image.h"
#include "GlException.h"


#define LAST_TEX_UNIT (textureUnitsNum - 1)


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
	glId(std::numeric_limits<uint>::max()),
	target(GL_TEXTURE_2D)
{}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
Texture::~Texture()
{
	if(isLoaded())
	{
		glDeleteTextures(1, &glId);
	}
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Texture::load(const char* filename)
{
	try
	{
		target = GL_TEXTURE_2D;
		if(isLoaded())
		{
			throw EXCEPTION("Texture already loaded");
		}

		Image img(filename);

		// bind the texture
		glGenTextures(1, &glId);
		bind(LAST_TEX_UNIT);
		if(mipmappingEnabled)
		{
			setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		setTexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, float(anisotropyLevel));

		// leave to GL_REPEAT. There is not real performance hit
		setRepeat(true);

		// chose formats
		int internalFormat;
		int format;
		int type;
		switch(img.getType())
		{
			case Image::CT_R:
				internalFormat = (compressionEnabled) ? GL_COMPRESSED_RED : GL_RED;
				format = GL_RED;
				type = GL_UNSIGNED_BYTE;
				break;

			case Image::CT_RGB:
				internalFormat = (compressionEnabled) ? GL_COMPRESSED_RGB : GL_RGB;
				format = GL_RGB;
				type = GL_UNSIGNED_BYTE;
				break;

			case Image::CT_RGBA:
				internalFormat = (compressionEnabled) ? GL_COMPRESSED_RGBA : GL_RGBA;
				format = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				break;

			default:
				throw EXCEPTION("See file");
		}

		glTexImage2D(target, 0, internalFormat, img.getWidth(), img.getHeight(), 0, format, type, &img.getData()[0]);
		if(mipmappingEnabled)
		{
			glGenerateMipmap(target);
		}

		ON_GL_FAIL_THROW_EXCEPTION();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("File \"" + filename + "\": " + e.what());
	}
}


//======================================================================================================================
// createEmpty2D                                                                                                       =
//======================================================================================================================
void Texture::createEmpty2D(float width_, float height_, int internalFormat, int format_, uint type_)
{
	target = GL_TEXTURE_2D;
	RASSERT_THROW_EXCEPTION(internalFormat > 0 && internalFormat <= 4); // deprecated internal format
	RASSERT_THROW_EXCEPTION(isLoaded());

	// GL stuff
	glGenTextures(1, &glId);
	bind(LAST_TEX_UNIT);

	setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setRepeat(true);

	// allocate to vram
	glTexImage2D(target, 0, internalFormat, width_, height_, 0, format_, type_, NULL);

	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// createEmpty2DMSAA                                                                                                   =
//======================================================================================================================
void Texture::createEmpty2DMsaa(int samplesNum, int internalFormat, int width_, int height_, bool mimapping)
{
	target = GL_TEXTURE_2D_MULTISAMPLE;
	RASSERT_THROW_EXCEPTION(isLoaded());

	glGenTextures(1, &glId);
	bind(LAST_TEX_UNIT);
	
	// allocate
	glTexImage2DMultisample(target, samplesNum, internalFormat, width_, height_, false);

	if(mimapping)
	{
		glGenerateMipmap(target);
	}
}


//======================================================================================================================
// bind                                                                                                                =
//======================================================================================================================
void Texture::bind(uint unit) const
{
	if(unit >= static_cast<uint>(textureUnitsNum))
	{
		throw EXCEPTION("Max tex units passed");
	}

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target, getGlId());
}


//======================================================================================================================
// getWidth                                                                                                            =
//======================================================================================================================
int Texture::getWidth() const
{
	bind(LAST_TEX_UNIT);
	int i;
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &i);
	return i;
}


//======================================================================================================================
// getHeight                                                                                                           =
//======================================================================================================================
int Texture::getHeight() const
{
	bind(LAST_TEX_UNIT);
	int i;
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &i);
	return i;
}


//======================================================================================================================
// setTexParameter [int]                                                                                               =
//======================================================================================================================
void Texture::setTexParameter(uint paramName, int value) const
{
	bind(LAST_TEX_UNIT);
	glTexParameteri(target, paramName, value);
}


//======================================================================================================================
// setTexParameter [float]                                                                                             =
//======================================================================================================================
void Texture::setTexParameter(uint paramName, float value) const
{
	bind(LAST_TEX_UNIT);
	glTexParameterf(target, paramName, value);
}


//======================================================================================================================
// setRepeat                                                                                                           =
//======================================================================================================================
void Texture::setRepeat(bool repeat) const
{
	bind(LAST_TEX_UNIT);
	if(repeat)
	{
		setTexParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		setTexParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}


//======================================================================================================================
// getBaseLevel                                                                                                        =
//======================================================================================================================
int Texture::getBaseLevel() const
{
	bind(LAST_TEX_UNIT);
	int level;
	glGetTexParameteriv(target, GL_TEXTURE_BASE_LEVEL, &level);
	return level;
}


//======================================================================================================================
// getActiveTexUnit                                                                                                    =
//======================================================================================================================
uint Texture::getActiveTexUnit()
{
	int unit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &unit);
	return unit - GL_TEXTURE0;
}


//======================================================================================================================
// getBindedTexId                                                                                                      =
//======================================================================================================================
uint Texture::getBindedTexId(uint unit)
{
	int id;
	glActiveTexture(GL_TEXTURE0 + unit);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &id);
	return id;
}


//======================================================================================================================
// getMaxLevel                                                                                                         =
//======================================================================================================================
int Texture::getMaxLevel() const
{
	bind(LAST_TEX_UNIT);
	int level;
	glGetTexParameteriv(target, GL_TEXTURE_MAX_LEVEL, &level);
	return level;
}


//======================================================================================================================
// setAnisotropy                                                                                                       =
//======================================================================================================================
void Texture::setAnisotropy(uint level)
{
	bind(LAST_TEX_UNIT);
	setTexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, int(level));
}


//======================================================================================================================
// setMipmapLevel                                                                                                      =
//======================================================================================================================
void Texture::setMipmapLevel(uint level)
{
	bind(LAST_TEX_UNIT);
	setTexParameter(GL_TEXTURE_BASE_LEVEL, int(level));
}


//======================================================================================================================
// genMipmap                                                                                                           =
//======================================================================================================================
void Texture::genMipmap()
{
	bind(LAST_TEX_UNIT);
	glGenerateMipmap(target);
}


//======================================================================================================================
// setFiltering                                                                                                        =
//======================================================================================================================
void Texture::setFiltering(TextureFilteringType filterType)
{
	switch(filterType)
	{
		case TFT_NEAREST:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case TFT_LINEAR:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case TFT_TRILINEAR:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}
