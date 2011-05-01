#include <GL/glew.h>
#include "Texture.h"
#include "Image.h"
#include "GlException.h"
#include "Logger.h" /// @todo remove it
#include "Math.h"  /// @todo remove it


#define LAST_TEX_UNIT (textureUnitsNum - 1)


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================
int Texture::textureUnitsNum = -1;
bool Texture::mipmappingEnabled = true;
bool Texture::compressionEnabled = true;
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
// load (const char*)                                                                                                  =
//======================================================================================================================
void Texture::load(const char* filename)
{
	try
	{
		load(Image(filename));
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("File \"" + filename + "\": " + e.what());
	}
}


//======================================================================================================================
// load (const Image&)                                                                                                 =
//======================================================================================================================
void Texture::load(const Image& img)
{
	Initializer init;
	init.width = img.getWidth();
	init.height = img.getHeight();
	init.dataSize = img.getData().getSizeInBytes();

	switch(img.getColorType())
	{
		case Image::CT_R:
			init.internalFormat = (compressionEnabled) ? GL_COMPRESSED_RED : GL_RED;
			init.format = GL_RED;
			init.type = GL_UNSIGNED_BYTE;
			break;

		case Image::CT_RGB:
			init.internalFormat = (compressionEnabled) ? GL_COMPRESSED_RGB : GL_RGB;
			init.format = GL_RGB;
			init.type = GL_UNSIGNED_BYTE;
			break;

		case Image::CT_RGBA:
			init.internalFormat = (compressionEnabled) ? GL_COMPRESSED_RGBA : GL_RGBA;
			init.format = GL_RGBA;
			init.type = GL_UNSIGNED_BYTE;
			break;

		default:
			throw EXCEPTION("See file");
	}

	switch(img.getDataCompression())
	{
		case Image::DC_NONE:
			init.dataCompression = DC_NONE;
			break;

		case Image::DC_DXT1:
			init.dataCompression = DC_DXT1;
			init.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

		case Image::DC_DXT3:
			init.dataCompression = DC_DXT3;
			init.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

		case Image::DC_DXT5:
			init.dataCompression = DC_DXT5;
			init.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	}

	init.data = &img.getData()[0];
	init.mipmapping = mipmappingEnabled;
	init.filteringType = mipmappingEnabled ? TFT_TRILINEAR : TFT_LINEAR;
	init.repeat = true;
	init.anisotropyLevel = anisotropyLevel;

	create(init);
}


//======================================================================================================================
// create                                                                                                              =
//======================================================================================================================
void Texture::create(const Initializer& init_)
{
	Initializer init = init_;

	//
	// Sanity checks
	//
	if(isLoaded())
	{
		throw EXCEPTION("Already loaded");
	}

	if(init.internalFormat <= 4)
	{
		throw EXCEPTION("Deprecated internal format");
	}

	//
	// Create
	//
	glGenTextures(1, &glId);
	bind(LAST_TEX_UNIT);
	target = GL_TEXTURE_2D;

	if(init.data == NULL)
	{
		init.dataCompression = DC_NONE;
	}

	switch(init.dataCompression)
	{
		case DC_NONE:
			glTexImage2D(target, 0, init.internalFormat, init.width, init.height, 0, init.format,
			             init.type, init.data);
			break;

		case DC_DXT1:
		case DC_DXT3:
		case DC_DXT5:
			glCompressedTexImage2D(target, 0, init.internalFormat, init.width, init.height, 0,
			                       init.dataSize, init.data);
			break;

		default:
			throw EXCEPTION("Incorrect data compression value");
	}

	setRepeat(init.repeat);

	if(init.mipmapping)
	{
		glGenerateMipmap(target);
	}

	// If not mipmapping then the filtering cannot be trilinear
	if(init.filteringType == TFT_TRILINEAR && !init.mipmapping)
	{
		setFiltering(TFT_LINEAR);
	}
	else
	{
		setFiltering(init.filteringType);
	}

	if(init.anisotropyLevel > 1)
	{
		setTexParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, float(init.anisotropyLevel));
	}

	ON_GL_FAIL_THROW_EXCEPTION();
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
int Texture::getWidth(uint level) const
{
	bind(LAST_TEX_UNIT);
	int i;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &i);
	return i;
}


//======================================================================================================================
// getHeight                                                                                                           =
//======================================================================================================================
int Texture::getHeight(uint level) const
{
	bind(LAST_TEX_UNIT);
	int i;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &i);
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
