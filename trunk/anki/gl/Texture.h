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

	void lock()
	{
		mtx.lock();
	}
	void unlock()
	{
		mtx.unlock();
	}

private:
	/// Texture units. The last is reserved and only used in Texture::create
	std::vector<Texture*> units;
	std::mutex mtx; ///< A mutex that locks the texture manager
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


/// Texture class
///
/// @note ITS NOT THREAD SAFE
class Texture
{
	friend class TextureManager;

public:
	enum TextureFilteringType
	{
		TFT_NEAREST,
		TFT_LINEAR,
		TFT_TRILINEAR
	};

	/// Texture initializer struct
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

	/// Default constructor
	Texture();

	/// Desrcuctor
	~Texture();

	/// @name Accessors
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

	/// Create a texture
	void create(const Initializer& init);

	void bind() const;
	void setFiltering(TextureFilteringType filterType)
	{
		bind();
		setFilteringNoBind(filterType);
	}
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
