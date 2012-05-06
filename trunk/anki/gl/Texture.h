#ifndef ANKI_GL_TEXTURE_H
#define ANKI_GL_TEXTURE_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include "anki/util/Singleton.h"
#include <cstdlib>
#include <GL/glew.h>
#include <vector>
#include <limits>
#include <thread>


namespace anki {


class Texture;


/// XXX
class TextureManager
{
public:
	TextureManager();

	/// @name Accessors
	/// @{
	uint getAnisotropyLevel() const
	{
		return anisotropyLevel;
	}
	void setAnisotropyLevel(uint x) 
	{
		anisotropyLevel = x;
	}

	bool getMipmappingEnabled() const
	{
		return mipmapping;
	}
	void setMipmappingEnabled(bool x) 
	{
		mipmapping = x;
	}

	bool getCompressionEnabled() const
	{
		return compression;
	}
	void setCompressionEnabled(bool x) 
	{
		compression = x;
	}

	size_t getMaxUnitsCount() const
	{
		return units.size() + 1;
	}
	/// @}

	/// The manager returns the texture unit where the @a tex can be binded
	uint choseUnit(Texture* tex);

	/// Alias for glActiveTexture
	void activateUnit(uint unit);

	/// This is called by the texture destructor in order to invalidate the 
	/// texture
	void textureDeleted(Texture* tex);

private:
	/// Texture unit representation
	struct Unit
	{
		Texture* tex;
		/// The bigger the life is the longer the @a tex has stayed witout bing 
		/// binded
		uint life; 
	};

	/// Texture units. The last is reserved and only used in Texture::create
	std::vector<Unit> units;
	uint activeUnit;

	/// @name Hints
	/// Hints when creating new textures. The implementation may try to ignore 
	/// them
	/// @{
	uint anisotropyLevel;
	bool mipmapping;
	/// If true the new textures will be compressed if not already
	bool compression;
	/// @}
};


/// The global texture manager
typedef Singleton<TextureManager> TextureManagerSingleton;


/// @brief Texture class
/// Generally its not thread safe and a few methods can be called by other 
/// threads. See the docs for each method
class Texture
{
	friend class TextureManager;

public:
	/// @brief Texture filtering type
	enum TextureFilteringType
	{
		TFT_NEAREST,
		TFT_LINEAR,
		TFT_TRILINEAR
	};

	/// @brief Texture initializer struct
	struct Initializer
	{
		uint width;
		uint height;
		GLint internalFormat;
		GLenum format;
		GLenum type;
		const void* data;
		bool mipmapping;
		TextureFilteringType filteringType;
		bool repeat;
		int anisotropyLevel;
		size_t dataSize; ///< For compressed textures
	};

	/// @brief Default constructor
	/// Thread safe
	Texture();

	/// @brief Desrcuctor
	/// Thread unsafe
	~Texture();

	/// @name Accessors
	/// Thread safe
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isLoaded());
		return glId;
	}

	GLenum getInternalFormat() const
	{
		ANKI_ASSERT(isLoaded());
		return internalFormat;
	}

	GLenum getFormat() const
	{
		ANKI_ASSERT(isLoaded());
		return format;
	}

	GLenum getType() const
	{
		ANKI_ASSERT(isLoaded());
		return type;
	}

	int getUnit() const
	{
		return unit;
	}

	GLuint getWidth() const
	{
		return width;
	}

	GLuint getHeight() const
	{
		return height;
	}
	/// @}

	/// @brief Create a texture
	/// Thread safe
	void create(const Initializer& init);

	/// @brief Bind the texture to a unit that the texture manager will decide
	/// Thread unsafe
	void bind() const;

	/// @brief Change the filtering type
	/// Thread unsafe
	void setFiltering(TextureFilteringType filterType)
	{
		bind();
		setFilteringNoBind(filterType);
	}

	/// @brief Generate new mipmaps
	/// Thread unsafe
	void genMipmap();

private:
	GLuint glId; ///< Identification for OGL
	GLuint target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLuint internalFormat; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLuint format; ///< GL_RED, GL_RG, GL_RGB etc
	GLuint type; ///< GL_UNSIGNED_BYTE, GL_BYTE etc
	GLuint width, height;

	/// The texure unit that its binded to. It's -1 if it's not binded to any
	/// texture unit
	mutable int unit; 

	bool isLoaded() const
	{
		return glId != std::numeric_limits<uint>::max();
	}

	void setFilteringNoBind(TextureFilteringType filterType) const;
};


} // end namespace


#endif
