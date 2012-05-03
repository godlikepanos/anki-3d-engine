#include "anki/gl/Texture.h"
#include "anki/gl/GlException.h"
#include "anki/util/Exception.h"
#include <boost/lexical_cast.hpp>


namespace anki {


//==============================================================================
// TextureManager                                                              =
//==============================================================================

//==============================================================================
TextureManager::TextureManager()
{
	GLint tmp;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
	units.resize(tmp, nullptr);
	
	activeUnit = -1;
	mipmapping = true;
	anisotropyLevel = 8;
	compression = true;
}


//==============================================================================
void TextureManager::activateUnit(uint unit)
{
	ANKI_ASSERT(unit < units.size());
	if(activeUnit == unit)
	{
		return;
	}

	activeUnit = unit;
	glActiveTexture(GL_TEXTURE0 + activeUnit);
}


//==============================================================================
uint TextureManager::choseUnit(Texture* tex)
{
	// Already binded
	//
	if(tex->unit != -1)
	{
		ANKI_ASSERT(units[tex->unit] == tex);
		return tex->unit;
	}

	// Find an empty slot for it
	//
	for(uint i = 0; i < units.size(); i++)
	{
		if(units[i] == nullptr)
		{
			units[i] = tex;
			return i;
		}
	}

	// If all units occupied chose a random for now
	//
	int tmp = rand() % units.size();
	units[tmp]->unit = -1;
	units[tmp] = tex;
	return tmp;
}


//==============================================================================
void TextureManager::textureDeleted(Texture* tex)
{
	for(auto it = units.begin(); it != units.end(); ++it)
	{
		if(*it == tex)
		{
			*it = nullptr;
			return;
		}
	}
	ANKI_ASSERT(0 && "Pointer not found");
}


//==============================================================================
// Texture                                                                     =
//==============================================================================

//==============================================================================
Texture::Texture()
	: glId(std::numeric_limits<uint>::max()), target(GL_TEXTURE_2D), unit(-1)
{}


//==============================================================================
Texture::~Texture()
{
	if(isLoaded())
	{
		glDeleteTextures(1, &glId);
	}

	TextureManagerSingleton::get().textureDeleted(this);
}


//==============================================================================
void Texture::create(const Initializer& init)
{
	// Sanity checks
	//
	ANKI_ASSERT(!isLoaded());
	ANKI_ASSERT(init.internalFormat <= 4 && "Deprecated internal format");

	// Create
	//
	glGenTextures(1, &glId);
	target = GL_TEXTURE_2D;
	internalFormat = init.internalFormat;
	format = init.format;
	type = init.type;
	width = init.width;
	height = init.height;

	// Bind
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target, glId);

	switch(internalFormat)
	{
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		glCompressedTexImage2D(target, 0, internalFormat,
			width, height, 0, init.dataSize, init.data);
		break;

	default:
		glTexImage2D(target, 0, internalFormat, width,
			height, 0, format, type, init.data);
	}

	if(init.repeat)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if(init.mipmapping && init.data)
	{
		glGenerateMipmap(target);
	}

	// If not mipmapping then the filtering cannot be trilinear
	if(init.filteringType == TFT_TRILINEAR && !init.mipmapping)
	{
		setFilteringNoBind(TFT_LINEAR);
	}
	else
	{
		setFilteringNoBind(init.filteringType);
	}

	if(init.anisotropyLevel > 1)
	{
		glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 
			GLint(init.anisotropyLevel));
	}

	ANKI_CHECK_GL_ERROR();
}


//==============================================================================
void Texture::bind() const
{
	unit = TextureManagerSingleton::get().choseUnit(
		const_cast<Texture*>(this));

	TextureManagerSingleton::get().activateUnit(unit);
	glBindTexture(target, glId);
}


//==============================================================================
void Texture::genMipmap()
{
	bind();
	glGenerateMipmap(target);
}


//==============================================================================
void Texture::setFilteringNoBind(TextureFilteringType filterType) const
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


} // end namespace
